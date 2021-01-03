#include "printing.h"

#include <stdlib.h>

static size_t print_char(struct output_stream output_stream, char c) {
    output_stream.put(output_stream.data, c);
    return 1;
}

static size_t print_repeated_char(struct output_stream output_stream, size_t n, char c) {
    size_t printed = 0;
    for (size_t i = 0; i < n; i++) {
        printed += print_char(output_stream, c);
    }
    return printed;
}

static size_t print_string(struct output_stream output_stream, char *string) {
    size_t printed = 0;
    for (; *string != '\0'; string++) {
        printed += print_char(output_stream, *string);
    }
    return printed;
}

static size_t unsigned_ap_digits_count(uint8_t base, struct ap ap) {
    size_t count = 0;
    while (ap_sign(ap_copy(ap)) != 0) {
        struct ap_division_result ap_division_result = ap_divide(ap, ap_from_uintmax_t(base));
        ap_destroy(ap_division_result.remainder);
        ap = ap_division_result.quotient;
        count++;
    }
    ap_destroy(ap);
    return count;
}

static size_t print_unsigned_ap(struct output_stream output_stream, bool uppercase, uint8_t base, struct ap ap, size_t digits_count) {
    char *string = malloc(digits_count + 1);
    size_t count = 0;
    while (ap_sign(ap_copy(ap)) != 0) {
        struct ap_division_result ap_division_result = ap_divide(ap, ap_from_uintmax_t(base));
        ap = ap_division_result.quotient;
        uint8_t digit = ap_to_uintmax_t(ap_division_result.remainder);
        if (digit < 10) {
            string[digits_count - count - 1] = (char) ('0' + digit);
        } else {
            string[digits_count - count - 1] = (char) ((uppercase ? 'A' : 'a') + (digit - 10));
        }
        count++;
    }
    ap_destroy(ap);
    string[digits_count] = '\0';
    size_t printed = print_string(output_stream, string);
    free(string);
    return printed;
}

static size_t print_integer_conversion_specification(struct output_stream output_stream,
                                                     bool uppercase,
                                                     uint8_t base,
                                                     struct conversion_specification_flags conversion_specification_flags,
                                                     int32_t field_width,
                                                     int32_t precision,
                                                     struct ap ap) {
    int8_t sign = ap_sign(ap_copy(ap));
    size_t digits_count = unsigned_ap_digits_count(base, ap_abs(ap_copy(ap)));
    bool zero_mode = conversion_specification_flags.zero && !conversion_specification_flags.minus && precision == -1;
    size_t actual_precision = precision == -1 ? 1 : precision;
    if (conversion_specification_flags.hash && base == 8 && actual_precision < digits_count + 1) {
        actual_precision = digits_count + 1;
    }
    size_t digits_to_be_printed_count = digits_count;
    if (digits_to_be_printed_count < actual_precision) {
        digits_to_be_printed_count = actual_precision;
    }
    size_t actual_width = digits_to_be_printed_count;
    if (conversion_specification_flags.space || conversion_specification_flags.plus || sign < 0) {
        actual_width++;
    }
    if (conversion_specification_flags.hash && base == 16 && sign != 0) {
        actual_width += 2;
    }
    size_t padding = 0;
    if (field_width != -1 && actual_width < field_width) {
        if (zero_mode) {
            digits_to_be_printed_count += field_width - actual_width;
        } else {
            padding = field_width - actual_width;
        }
    }
    size_t printed = 0;
    if (!conversion_specification_flags.minus) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    if (sign < 0) {
        printed += print_char(output_stream, '-');
    } else if (conversion_specification_flags.plus) {
        printed += print_char(output_stream, '+');
    } else if (conversion_specification_flags.space) {
        printed += print_char(output_stream, ' ');
    }
    if (conversion_specification_flags.hash && base == 16 && sign != 0) {
        printed += print_string(output_stream, uppercase ? "0X" : "0x");
    }
    printed += print_repeated_char(output_stream, digits_to_be_printed_count - digits_count, '0');
    printed += print_unsigned_ap(output_stream, uppercase, base, ap_abs(ap), digits_count);
    if (conversion_specification_flags.minus) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    return printed;
}

struct ap round_even(struct fp fp) {
    struct ap ap = fp_to_ap(fp_add(fp_copy(fp), fp_from_long_double(0.5).fp));
    struct ap_division_result ap_division_result = ap_divide(ap_copy(ap), ap_from_intmax_t(2));
    ap_destroy(ap_division_result.quotient);
    size_t precision = fp.precision;
    struct fp subtraction_result = fp_subtract(fp, fp_from_ap(ap_copy(ap), precision));
    if (ap_sign(ap_division_result.remainder) != 0 && fp_compare(fp_abs(fp_copy(subtraction_result)), fp_from_long_double(0.5).fp) == 0) {
        return ap_add(ap, fp_to_ap(fp_multiply(subtraction_result, fp_from_long_double(2.0).fp)));
    } else {
        fp_destroy(subtraction_result);
        return ap;
    }
}

struct to_digits_result {
    struct ap exponent;
    uint8_t first_digit;
    struct ap rest_digits;
};

struct to_digits_result to_digits(int32_t precision, struct fp fp) {
    fp = fp_extend(fp, precision * 4);
    bool zero = fp_sign(fp_copy(fp)) == 0;
    struct ap ap_precision_power = ap_power(ap_from_uintmax_t(10), ap_from_uintmax_t(precision));
    struct fp fp_precision_power = fp_from_ap(ap_copy(ap_precision_power), fp.precision);
    struct ap starting_exponent;
    if (zero) {
        starting_exponent = ap_from_intmax_t(0);
    } else {
        struct ap_division_result exponent_division_result = ap_divide(ap_multiply(ap_copy(fp.exponent), ap_from_intmax_t(301029995663981195)), ap_from_intmax_t(1000000000000000000));
        ap_destroy(exponent_division_result.remainder);
        starting_exponent = ap_subtract(exponent_division_result.quotient, ap_from_intmax_t(2));
    }
    struct fp normalized = fp_divide(fp_abs(fp_copy(fp)), fp_integer_power(fp_from_ap(ap_from_uintmax_t(10), fp.precision), ap_copy(starting_exponent)));
    fp_destroy(fp);
    for (struct ap exponent = starting_exponent;;) {
        if (zero || fp_compare(fp_copy(normalized), fp_from_long_double(0.1).fp) >= 0 && fp_compare(fp_copy(normalized), fp_from_long_double(10.0).fp) < 0) {
            struct ap multiplied = round_even(fp_multiply(fp_copy(normalized), fp_copy(fp_precision_power)));
            struct ap_division_result multiplied_division_result = ap_divide(multiplied, ap_copy(ap_precision_power));
            struct ap quotient = multiplied_division_result.quotient;
            struct ap remainder = multiplied_division_result.remainder;
            if (zero || ap_compare(ap_copy(quotient), ap_from_uintmax_t(1)) >= 0 && ap_compare(ap_copy(quotient), ap_from_uintmax_t(10)) < 0) {
                fp_destroy(normalized);
                ap_destroy(ap_precision_power);
                fp_destroy(fp_precision_power);
                return (struct to_digits_result) {
                    .exponent = exponent,
                    .first_digit = ap_to_uintmax_t(quotient),
                    .rest_digits = remainder,
                };
            } else {
                ap_destroy(quotient);
                ap_destroy(remainder);
            }
        }
        exponent = ap_add(exponent, ap_from_intmax_t(1));
        normalized = fp_divide(normalized, fp_from_long_double(10.0).fp);
    }
}

struct ap remove_trailing_zeros(struct ap ap) {
    while (ap_sign(ap_copy(ap)) != 0) {
        struct ap_division_result ap_division_result = ap_divide(ap_copy(ap), ap_from_uintmax_t(10));
        if (ap_sign(ap_division_result.remainder) == 0) {
            ap_destroy(ap);
            ap = ap_division_result.quotient;
        } else {
            ap_destroy(ap_division_result.quotient);
            break;
        }
    }
    return ap;
}

static size_t print_exponential_decimal_conversion_specification(struct output_stream output_stream,
                                                                 bool no_trailing_zeros,
                                                                 bool uppercase,
                                                                 struct conversion_specification_flags conversion_specification_flags,
                                                                 int32_t field_width,
                                                                 int32_t precision,
                                                                 struct fp fp,
                                                                 bool negative,
                                                                 bool infinity,
                                                                 bool nan) {
    if (infinity || nan) {
        fp_destroy(fp);
        fp = fp_from_long_double(0.0).fp;
    }
    size_t actual_precision = (infinity || nan) ? 0 : precision == -1 ? 6 : precision;
    bool zero_mode = !infinity && !nan && conversion_specification_flags.zero && !conversion_specification_flags.minus;
    struct to_digits_result to_digits_result = to_digits(actual_precision, fp);
    struct ap exponent = to_digits_result.exponent;
    uint8_t first_digit = to_digits_result.first_digit;
    struct ap rest_digits = to_digits_result.rest_digits;
    size_t initial_rest_digits_digits_count = unsigned_ap_digits_count(10, ap_copy(rest_digits));
    size_t rest_digits_digits_count = initial_rest_digits_digits_count;
    if (!infinity && !nan && no_trailing_zeros && !conversion_specification_flags.hash) {
        rest_digits = remove_trailing_zeros(rest_digits);
        rest_digits_digits_count = unsigned_ap_digits_count(10, ap_copy(rest_digits));
        if (ap_sign(ap_copy(rest_digits)) == 0) {
            initial_rest_digits_digits_count = actual_precision;
        }
    }
    size_t exponent_digits_count = unsigned_ap_digits_count(10, ap_abs(ap_copy(exponent)));
    size_t actual_width = actual_precision - (initial_rest_digits_digits_count - rest_digits_digits_count) + 3;
    if (conversion_specification_flags.space || conversion_specification_flags.plus || negative) {
        actual_width++;
    }
    if (!infinity && !nan) {
        if (initial_rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            actual_width++;
        }
        if (exponent_digits_count < 2) {
            actual_width += 2;
        } else {
            actual_width += exponent_digits_count;
        }
    }
    size_t padding = 0;
    if (field_width != -1 && actual_width < field_width) {
        padding = field_width - actual_width;
    }
    size_t printed = 0;
    if (!conversion_specification_flags.minus && !zero_mode) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    if (negative) {
        printed += print_char(output_stream, '-');
    } else if (conversion_specification_flags.plus) {
        printed += print_char(output_stream, '+');
    } else if (conversion_specification_flags.space) {
        printed += print_char(output_stream, ' ');
    }
    if (!conversion_specification_flags.minus && zero_mode) {
        printed += print_repeated_char(output_stream, padding, '0');
    }
    if (infinity) {
        printed += print_string(output_stream, uppercase ? "INF" : "inf");
    } else if (nan) {
        printed += print_string(output_stream, uppercase ? "NAN" : "nan");
    } else {
        if (first_digit == 0) {
            printed += print_char(output_stream, '0');
        } else {
            printed += print_unsigned_ap(output_stream, false, 10, ap_from_uintmax_t(first_digit), 1);
        }
        if (initial_rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            printed += print_char(output_stream, '.');
        }
        if (initial_rest_digits_digits_count < actual_precision) {
            printed += print_repeated_char(output_stream, actual_precision - initial_rest_digits_digits_count, '0');
        }
        printed += print_unsigned_ap(output_stream, false, 10, rest_digits, rest_digits_digits_count);
        printed += print_char(output_stream, uppercase ? 'E' : 'e');
        printed += print_char(output_stream, ap_sign(ap_copy(exponent)) < 0 ? '-' : '+');
        if (exponent_digits_count < 2) {
            printed += print_repeated_char(output_stream, 2 - exponent_digits_count, '0');
        }
        printed += print_unsigned_ap(output_stream, false, 10, ap_abs(exponent), exponent_digits_count);
    }
    if (conversion_specification_flags.minus) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    return printed;
}

static size_t print_fixed_decimal_conversion_specification(struct output_stream output_stream,
                                                           bool no_trailing_zeros,
                                                           bool uppercase,
                                                           struct conversion_specification_flags conversion_specification_flags,
                                                           int32_t field_width,
                                                           int32_t precision,
                                                           struct fp fp,
                                                           bool negative,
                                                           bool infinity,
                                                           bool nan) {
    if (infinity || nan) {
        fp_destroy(fp);
        fp = fp_from_long_double(0.0).fp;
    }
    size_t actual_precision = (infinity || nan) ? 3 : precision == -1 ? 6 : precision;
    size_t fp_precision = 64 + actual_precision * 4;
    if (ap_sign(ap_copy(fp.exponent)) > 0) {
        fp_precision += ap_to_uintmax_t(ap_copy(fp.exponent));
    }
    bool zero_mode = !infinity && !nan && conversion_specification_flags.zero && !conversion_specification_flags.minus;
    struct ap actual_precision_power = ap_power(ap_from_uintmax_t(10), ap_from_uintmax_t(actual_precision));
    struct ap_division_result multiplied_division_result = ap_divide(
        round_even(fp_multiply(fp_extend(fp_abs(fp), fp_precision), fp_from_ap(ap_copy(actual_precision_power), fp_precision))),
        actual_precision_power
    );
    struct ap first_digits = multiplied_division_result.quotient;
    struct ap rest_digits = multiplied_division_result.remainder;
    size_t first_digits_digits_count = unsigned_ap_digits_count(10, ap_copy(first_digits));
    size_t initial_rest_digits_digits_count = unsigned_ap_digits_count(10, ap_copy(rest_digits));
    size_t rest_digits_digits_count = initial_rest_digits_digits_count;
    if (!infinity && !nan && no_trailing_zeros && !conversion_specification_flags.hash) {
        rest_digits = remove_trailing_zeros(rest_digits);
        rest_digits_digits_count = unsigned_ap_digits_count(10, ap_copy(rest_digits));
        if (ap_sign(ap_copy(rest_digits)) == 0) {
            initial_rest_digits_digits_count = actual_precision;
        }
    }
    size_t actual_width = actual_precision - (initial_rest_digits_digits_count - rest_digits_digits_count);
    if (conversion_specification_flags.space || conversion_specification_flags.plus || negative) {
        actual_width++;
    }
    if (!infinity && !nan) {
        if (initial_rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            actual_width++;
        }
        if (first_digits_digits_count == 0) {
            actual_width += 1;
        } else {
            actual_width += first_digits_digits_count;
        }
    }
    size_t padding = 0;
    if (field_width != -1 && actual_width < field_width) {
        padding = field_width - actual_width;
    }
    size_t printed = 0;
    if (!conversion_specification_flags.minus && !zero_mode) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    if (negative) {
        printed += print_char(output_stream, '-');
    } else if (conversion_specification_flags.plus) {
        printed += print_char(output_stream, '+');
    } else if (conversion_specification_flags.space) {
        printed += print_char(output_stream, ' ');
    }
    if (!conversion_specification_flags.minus && zero_mode) {
        printed += print_repeated_char(output_stream, padding, '0');
    }
    if (infinity) {
        printed += print_string(output_stream, uppercase ? "INF" : "inf");
    } else if (nan) {
        printed += print_string(output_stream, uppercase ? "NAN" : "nan");
    } else {
        if (first_digits_digits_count == 0) {
            printed += print_char(output_stream, '0');
        } else {
            printed += print_unsigned_ap(output_stream, false, 10, first_digits, first_digits_digits_count);
        }
        if (initial_rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            printed += print_char(output_stream, '.');
        }
        if (initial_rest_digits_digits_count < actual_precision) {
            printed += print_repeated_char(output_stream, actual_precision - initial_rest_digits_digits_count, '0');
        }
        printed += print_unsigned_ap(output_stream, false, 10, rest_digits, rest_digits_digits_count);
    }
    if (conversion_specification_flags.minus) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    return printed;
}

static size_t print_shorter_decimal_conversion_specification(struct output_stream output_stream,
                                                             bool uppercase,
                                                             struct conversion_specification_flags conversion_specification_flags,
                                                             int32_t field_width,
                                                             int32_t precision,
                                                             struct fp fp,
                                                             bool negative,
                                                             bool infinity,
                                                             bool nan) {
    if (infinity || nan) {
        fp_destroy(fp);
        fp = fp_from_long_double(0.0).fp;
    }
    size_t actual_precision = precision == -1 ? 6 : precision == 0 ? 1 : precision;
    struct to_digits_result to_digits_result = to_digits(actual_precision - 1, fp_copy(fp));
    if (ap_compare(ap_copy(to_digits_result.exponent), ap_from_intmax_t(-4)) < 0 || ap_compare(ap_copy(to_digits_result.exponent), ap_from_uintmax_t(actual_precision)) >= 0) {
        ap_destroy(to_digits_result.rest_digits);
        ap_destroy(to_digits_result.exponent);
        actual_precision--;
        return print_exponential_decimal_conversion_specification(
            output_stream,
            true,
            uppercase,
            conversion_specification_flags,
            field_width,
            actual_precision,
            fp,
            negative,
            infinity,
            nan
        );
    } else {
        actual_precision -= ap_to_intmax_t(to_digits_result.exponent) + 1;
        ap_destroy(to_digits_result.rest_digits);
        return print_fixed_decimal_conversion_specification(
            output_stream,
            true,
            uppercase,
            conversion_specification_flags,
            field_width,
            actual_precision,
            fp,
            negative,
            infinity,
            nan
        );
    }
}

static size_t print_exponential_hexadecimal_conversion_specification(struct output_stream output_stream,
                                                                     bool uppercase,
                                                                     struct conversion_specification_flags conversion_specification_flags,
                                                                     int32_t field_width,
                                                                     int32_t precision,
                                                                     struct fp fp,
                                                                     bool negative,
                                                                     bool infinity,
                                                                     bool nan) {
    if (infinity || nan) {
        fp_destroy(fp);
        fp = fp_from_long_double(0.0).fp;
    }
    size_t actual_precision = (infinity || nan) ? 0 : precision == -1 ? (fp.precision + 4 - 1) / 4 : precision;
    fp = fp_extend(fp, actual_precision * 4);
    bool zero_mode = !infinity && !nan && conversion_specification_flags.zero && !conversion_specification_flags.minus;
    struct ap exponent = ap_copy(fp.exponent);
    struct fp normalized = fp_abs(fp_copy(fp));
    ap_destroy(normalized.exponent);
    normalized.exponent = ap_from_intmax_t(0);
    struct ap actual_precision_power = ap_left_shift(ap_from_uintmax_t(1), actual_precision * 4);
    struct ap multiplied = round_even(fp_multiply(normalized, fp_from_ap(ap_copy(actual_precision_power), fp.precision)));
    if (!infinity && !nan && precision == -1) {
        if (ap_sign(ap_copy(multiplied)) == 0) {
            ap_destroy(actual_precision_power);
            actual_precision_power = ap_from_uintmax_t(1);
            actual_precision = 0;
        } else {
            while (ap_bit_scan_forward(ap_copy(multiplied)) >= 4) {
                multiplied = ap_right_shift(multiplied, 4);
                actual_precision_power = ap_right_shift(actual_precision_power, 4);
                actual_precision--;
            }
        }
    }
    struct ap_division_result multiplied_division_result = ap_divide(multiplied, actual_precision_power);
    fp_destroy(fp);
    uint8_t first_digit = ap_to_uintmax_t(multiplied_division_result.quotient);
    struct ap rest_digits = multiplied_division_result.remainder;
    size_t rest_digits_digits_count = unsigned_ap_digits_count(16, ap_copy(rest_digits));
    size_t exponent_digits_count = unsigned_ap_digits_count(10, ap_abs(ap_copy(exponent)));
    size_t actual_width = actual_precision + 3;
    if (conversion_specification_flags.space || conversion_specification_flags.plus || negative) {
        actual_width++;
    }
    if (!infinity && !nan) {
        actual_width += 2;
        if (rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            actual_width++;
        }
        if (exponent_digits_count < 1) {
            actual_width++;
        } else {
            actual_width += exponent_digits_count;
        }
    }
    size_t padding = 0;
    if (field_width != -1 && actual_width < field_width) {
        padding = field_width - actual_width;
    }
    size_t printed = 0;
    if (!conversion_specification_flags.minus && !zero_mode) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    if (negative) {
        printed += print_char(output_stream, '-');
    } else if (conversion_specification_flags.plus) {
        printed += print_char(output_stream, '+');
    } else if (conversion_specification_flags.space) {
        printed += print_char(output_stream, ' ');
    }
    if (!infinity && !nan) {
        printed += print_string(output_stream, uppercase ? "0X" : "0x");
    }
    if (!conversion_specification_flags.minus && zero_mode) {
        printed += print_repeated_char(output_stream, padding, '0');
    }
    if (infinity) {
        printed += print_string(output_stream, uppercase ? "INF" : "inf");
    } else if (nan) {
        printed += print_string(output_stream, uppercase ? "NAN" : "nan");
    } else {
        if (first_digit == 0) {
            printed += print_char(output_stream, '0');
        } else {
            printed += print_unsigned_ap(output_stream, uppercase, 16, ap_from_uintmax_t(first_digit), 1);
        }
        if (rest_digits_digits_count < actual_precision || rest_digits_digits_count != 0 || conversion_specification_flags.hash) {
            printed += print_char(output_stream, '.');
        }
        if (rest_digits_digits_count < actual_precision) {
            printed += print_repeated_char(output_stream, actual_precision - rest_digits_digits_count, '0');
        }
        printed += print_unsigned_ap(output_stream, uppercase, 16, rest_digits, rest_digits_digits_count);
        printed += print_char(output_stream, uppercase ? 'P' : 'p');
        printed += print_char(output_stream, ap_sign(ap_copy(exponent)) < 0 ? '-' : '+');
        if (exponent_digits_count == 0) {
            printed += print_char(output_stream, '0');
        }
        printed += print_unsigned_ap(output_stream, false, 10, ap_abs(exponent), exponent_digits_count);
    }
    if (conversion_specification_flags.minus) {
        printed += print_repeated_char(output_stream, padding, ' ');
    }
    return printed;
}

static size_t print_conversion_specification(struct output_stream output_stream, struct conversion_specification conversion_specification, size_t printed_so_far) {
    switch (conversion_specification.conversion_specifier) {
        case CONVERSION_SPECIFIER_literal:
            return print_char(output_stream, conversion_specification.data_char);
        case CONVERSION_SPECIFIER_d:
        case CONVERSION_SPECIFIER_i:
            return print_integer_conversion_specification(
                output_stream,
                false,
                10,
                conversion_specification.conversion_specification_flags,
                conversion_specification.field_width,
                conversion_specification.precision,
                conversion_specification.length_modifier == LENGTH_MODIFIER_Z ? conversion_specification.data_ap : ap_from_intmax_t(conversion_specification.data_intmax_t)
            );
        case CONVERSION_SPECIFIER_o:
            return print_integer_conversion_specification(
                output_stream,
                false,
                8,
                conversion_specification.conversion_specification_flags,
                conversion_specification.field_width,
                conversion_specification.precision,
                conversion_specification.length_modifier == LENGTH_MODIFIER_Z ? conversion_specification.data_ap : ap_from_uintmax_t(conversion_specification.data_uintmax_t)
            );
        case CONVERSION_SPECIFIER_u:
            return print_integer_conversion_specification(
                output_stream,
                false,
                10,
                conversion_specification.conversion_specification_flags,
                conversion_specification.field_width,
                conversion_specification.precision,
                conversion_specification.length_modifier == LENGTH_MODIFIER_Z ? conversion_specification.data_ap : ap_from_uintmax_t(conversion_specification.data_uintmax_t)
            );
        case CONVERSION_SPECIFIER_x:
            return print_integer_conversion_specification(
                output_stream,
                false,
                16,
                conversion_specification.conversion_specification_flags,
                conversion_specification.field_width,
                conversion_specification.precision,
                conversion_specification.length_modifier == LENGTH_MODIFIER_Z ? conversion_specification.data_ap : ap_from_uintmax_t(conversion_specification.data_uintmax_t)
            );
        case CONVERSION_SPECIFIER_X:
            return print_integer_conversion_specification(
                output_stream,
                true,
                16,
                conversion_specification.conversion_specification_flags,
                conversion_specification.field_width,
                conversion_specification.precision,
                conversion_specification.length_modifier == LENGTH_MODIFIER_Z ? conversion_specification.data_ap : ap_from_uintmax_t(conversion_specification.data_uintmax_t)
            );
        case CONVERSION_SPECIFIER_e:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_exponential_decimal_conversion_specification(
                    output_stream,
                    false,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_exponential_decimal_conversion_specification(
                    output_stream,
                    false,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_E:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_exponential_decimal_conversion_specification(
                    output_stream,
                    false,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_exponential_decimal_conversion_specification(
                    output_stream,
                    false,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_f:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_fixed_decimal_conversion_specification(
                    output_stream,
                    false,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_fixed_decimal_conversion_specification(
                    output_stream,
                    false,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_F:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_fixed_decimal_conversion_specification(
                    output_stream,
                    false,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_fixed_decimal_conversion_specification(
                    output_stream,
                    false,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_g:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_shorter_decimal_conversion_specification(
                    output_stream,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_shorter_decimal_conversion_specification(
                    output_stream,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_G:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_shorter_decimal_conversion_specification(
                    output_stream,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_shorter_decimal_conversion_specification(
                    output_stream,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_a:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_exponential_hexadecimal_conversion_specification(
                    output_stream,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_exponential_hexadecimal_conversion_specification(
                    output_stream,
                    false,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_A:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_F) {
                bool negative = fp_sign(fp_copy(conversion_specification.data_fp)) < 0;
                return print_exponential_hexadecimal_conversion_specification(
                    output_stream,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    conversion_specification.data_fp,
                    negative,
                    false,
                    false
                );
            } else {
                struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
                return print_exponential_hexadecimal_conversion_specification(
                    output_stream,
                    true,
                    conversion_specification.conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    fp_from_long_double_result.fp,
                    fp_from_long_double_result.negative,
                    fp_from_long_double_result.infinity,
                    fp_from_long_double_result.nan
                );
            }
        case CONVERSION_SPECIFIER_c: {
            size_t printed = 0;
            if (!conversion_specification.conversion_specification_flags.minus && 1 < conversion_specification.field_width) {
                printed += print_repeated_char(output_stream, conversion_specification.field_width - 1, ' ');
            }
            printed += print_char(output_stream, conversion_specification.data_char);
            if (conversion_specification.conversion_specification_flags.minus && 1 < conversion_specification.field_width) {
                printed += print_repeated_char(output_stream, conversion_specification.field_width - 1, ' ');
            }
            return printed;
        }
        case CONVERSION_SPECIFIER_s: {
            char *string = conversion_specification.data_char_pointer;
            if (string == NULL) {
                if (conversion_specification.precision == -1 || conversion_specification.precision >= 6) {
                    string = "(null)";
                } else {
                    string = "";
                }
            }
            size_t length = 0;
            while ((conversion_specification.precision == -1 || length < conversion_specification.precision) && string[length] != '\0') {
                length++;
            }
            size_t printed = 0;
            if (!conversion_specification.conversion_specification_flags.minus && conversion_specification.field_width != -1 && length < conversion_specification.field_width) {
                printed += print_repeated_char(output_stream, conversion_specification.field_width - length, ' ');
            }
            for (size_t i = 0; i < length; i++) {
                printed += print_char(output_stream, string[i]);
            }
            if (conversion_specification.conversion_specification_flags.minus && conversion_specification.field_width != -1 && length < conversion_specification.field_width) {
                printed += print_repeated_char(output_stream, conversion_specification.field_width - length, ' ');
            }
            return printed;
        }
        case CONVERSION_SPECIFIER_p:
            if (conversion_specification.data_void_pointer == NULL) {
                size_t printed = 0;
                if (!conversion_specification.conversion_specification_flags.minus && 5 < conversion_specification.field_width) {
                    printed += print_repeated_char(output_stream, conversion_specification.field_width - 5, ' ');
                }
                printed += print_string(output_stream, "(nil)");
                if (conversion_specification.conversion_specification_flags.minus && 5 < conversion_specification.field_width) {
                    printed += print_repeated_char(output_stream, conversion_specification.field_width - 5, ' ');
                }
                return printed;
            } else {
                struct conversion_specification_flags conversion_specification_flags = conversion_specification.conversion_specification_flags;
                conversion_specification_flags.hash = true;
                return print_integer_conversion_specification(
                    output_stream,
                    false,
                    16,
                    conversion_specification_flags,
                    conversion_specification.field_width,
                    conversion_specification.precision,
                    ap_from_uintmax_t((uintmax_t) conversion_specification.data_void_pointer)
                );
            }
        case CONVERSION_SPECIFIER_n:
            if (conversion_specification.length_modifier == LENGTH_MODIFIER_hh) {
                *conversion_specification.data_signed_char_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_h) {
                *conversion_specification.data_short_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_l) {
                *conversion_specification.data_long_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_ll) {
                *conversion_specification.data_long_long_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_j) {
                *conversion_specification.data_intmax_t_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_z) {
                *conversion_specification.data_size_t_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_t) {
                *conversion_specification.data_ptrdiff_t_pointer = printed_so_far;
            } else if (conversion_specification.length_modifier == LENGTH_MODIFIER_Z) {
                *conversion_specification.data_ap_pointer = ap_from_uintmax_t(printed_so_far);
            } else {
                *conversion_specification.data_int_pointer = printed_so_far;
            }
            return 0;
    }
}

size_t print_parsed_format(struct output_stream output_stream, struct parsed_format parsed_format) {
    size_t printed = 0;
    for (size_t i = 0; i < parsed_format.length; i++) {
        printed += print_conversion_specification(output_stream, parsed_format.conversion_specifications[i], printed);
    }
    return printed;
}
