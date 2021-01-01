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

static size_t print_unsigned_ap(struct output_stream output_stream, bool uppercase, uint8_t base, struct ap ap) {
    size_t digits_count = unsigned_ap_digits_count(base, ap_copy(ap));
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
    printed += print_unsigned_ap(output_stream, uppercase, base, ap_abs(ap));
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

static size_t print_exponential_decimal_conversion_specification(struct output_stream output_stream,
                                                                 bool uppercase,
                                                                 struct conversion_specification_flags conversion_specification_flags,
                                                                 int32_t field_width,
                                                                 int32_t precision,
                                                                 struct fp fp,
                                                                 bool negative,
                                                                 bool infinity,
                                                                 bool nan) {
    size_t actual_precision = precision == -1 ? 6 : precision;
    bool zero_mode = conversion_specification_flags.zero && !conversion_specification_flags.minus;
    size_t printed = 0;
    struct ap starting_exponent;
    if (fp_sign(fp_copy(fp)) == 0) {
        starting_exponent = ap_from_intmax_t(0);
    } else {
        struct ap_division_result exponent_division_result = ap_divide(ap_multiply(ap_copy(fp.exponent), ap_from_intmax_t(301029995663981195)), ap_from_intmax_t(1000000000000000000));
        ap_destroy(exponent_division_result.remainder);
        starting_exponent = ap_subtract(exponent_division_result.quotient, ap_from_intmax_t(2));
    }
    for (struct ap exponent = starting_exponent;; exponent = ap_add(exponent, ap_from_intmax_t(1))) {
        struct fp normalized = fp_divide(fp_abs(fp_copy(fp)), fp_integer_power(fp_from_long_double(10.0).fp, ap_copy(exponent)));
        if (fp_sign(fp_copy(fp)) == 0 || fp_compare(fp_copy(normalized), fp_from_long_double(0.1).fp) >= 0 && fp_compare(fp_copy(normalized), fp_from_long_double(10.0).fp) < 0) {
            struct ap multiplied = round_even(fp_multiply(normalized, fp_integer_power(fp_from_long_double(10.0).fp, ap_from_uintmax_t(actual_precision))));
            struct ap_division_result multiplied_division_result = ap_divide(multiplied, ap_power(ap_from_intmax_t(10), ap_from_uintmax_t(actual_precision)));
            struct ap quotient = multiplied_division_result.quotient;
            struct ap remainder = multiplied_division_result.remainder;
            if (fp_sign(fp_copy(fp)) == 0 || ap_compare(ap_copy(quotient), ap_from_intmax_t(1)) >= 0 && ap_compare(ap_copy(quotient), ap_from_intmax_t(10)) < 0) {
                size_t quotient_digits_count = unsigned_ap_digits_count(10, ap_copy(quotient));
                size_t remainder_digits_count = unsigned_ap_digits_count(10, ap_copy(remainder));
                size_t exponent_digits_count = unsigned_ap_digits_count(10, ap_abs(ap_copy(exponent)));
                size_t actual_width = actual_precision + 3;
                if (conversion_specification_flags.space || conversion_specification_flags.plus || negative) {
                    actual_width++;
                }
                if (actual_precision != 0 || conversion_specification_flags.hash) {
                    actual_width++;
                }
                if (exponent_digits_count < 2) {
                    actual_width += 2;
                } else {
                    actual_width += exponent_digits_count;
                }
                size_t padding = 0;
                if (field_width != -1 && actual_width < field_width) {
                    padding = field_width - actual_width;
                }
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
                if (quotient_digits_count < 1) {
                    printed += print_repeated_char(output_stream, 1 - quotient_digits_count, '0');
                }
                printed += print_unsigned_ap(output_stream, false, 10, ap_copy(quotient));
                if (actual_precision != 0 || conversion_specification_flags.hash) {
                    printed += print_char(output_stream, '.');
                }
                if (remainder_digits_count < actual_precision) {
                    printed += print_repeated_char(output_stream, actual_precision - remainder_digits_count, '0');
                }
                printed += print_unsigned_ap(output_stream, false, 10, ap_copy(remainder));
                printed += print_char(output_stream, uppercase ? 'E' : 'e');
                printed += print_char(output_stream, ap_sign(ap_copy(exponent)) < 0 ? '-' : '+');
                if (exponent_digits_count < 2) {
                    printed += print_repeated_char(output_stream, 2 - exponent_digits_count, '0');
                }
                printed += print_unsigned_ap(output_stream, false, 10, ap_abs(ap_copy(exponent)));
                if (conversion_specification_flags.minus) {
                    printed += print_repeated_char(output_stream, padding, ' ');
                }
                ap_destroy(quotient);
                ap_destroy(remainder);
                ap_destroy(exponent);
                break;
            } else {
                ap_destroy(quotient);
                ap_destroy(remainder);
            }
        } else {
            fp_destroy(normalized);
        }
    }
    fp_destroy(fp);
    return printed;
}

static size_t print_conversion_specification(struct output_stream output_stream, struct conversion_specification conversion_specification) {
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
        case CONVERSION_SPECIFIER_e: {
            struct fp_from_long_double_result fp_from_long_double_result = fp_from_long_double(conversion_specification.data_long_double);
            return print_exponential_decimal_conversion_specification(
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
        case CONVERSION_SPECIFIER_E:
            break;
        case CONVERSION_SPECIFIER_f:
            break;
        case CONVERSION_SPECIFIER_F:
            break;
        case CONVERSION_SPECIFIER_g:
            break;
        case CONVERSION_SPECIFIER_G:
            break;
        case CONVERSION_SPECIFIER_a:
            break;
        case CONVERSION_SPECIFIER_A:
            break;
        case CONVERSION_SPECIFIER_c:
            break;
        case CONVERSION_SPECIFIER_s:
            break;
        case CONVERSION_SPECIFIER_p:
            break;
    }
}

size_t print_parsed_format(struct output_stream output_stream, struct parsed_format parsed_format) {
    size_t printed = 0;
    for (size_t i = 0; i < parsed_format.length; i++) {
        printed += print_conversion_specification(output_stream, parsed_format.conversion_specifications[i]);
    }
    return printed;
}
