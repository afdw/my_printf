#include "fp.h"

#include <string.h>

static struct fp fp_create_empty() {
    return (struct fp) {
        .precision = 0,
        .significand = ap_from_uintmax_t(0),
        .exponent = ap_from_uintmax_t(0),
    };
}

struct fp fp_copy(struct fp fp) {
    return (struct fp) {
        .precision = fp.precision,
        .significand = ap_copy(fp.significand),
        .exponent = ap_copy(fp.exponent),
    };
}

struct fp fp_extend(struct fp fp, size_t new_precision) {
    if (fp.precision < new_precision) {
        fp.significand = ap_left_shift(fp.significand, new_precision - fp.precision);
        fp.precision = new_precision;
    }
    return fp;
}

struct fp fp_shrink(struct fp fp, size_t new_precision) {
    if (fp.precision > new_precision) {
        fp.significand = ap_right_shift(fp.significand, fp.precision - new_precision);
        fp.precision = new_precision;
    }
    return fp;
}

struct fp fp_normalize(struct fp fp) {
    size_t bit_scan_reverse_result = ap_bit_scan_reverse(ap_copy(fp.significand));
    if (bit_scan_reverse_result != (size_t) -1) {
        if (bit_scan_reverse_result < fp.precision) {
            fp.significand = ap_left_shift(fp.significand, fp.precision - bit_scan_reverse_result);
            fp.exponent = ap_subtract(fp.exponent, ap_from_uintmax_t(fp.precision - bit_scan_reverse_result));
        }
        if (bit_scan_reverse_result > fp.precision) {
            fp.significand = ap_right_shift(fp.significand, bit_scan_reverse_result - fp.precision);
            fp.exponent = ap_add(fp.exponent, ap_from_uintmax_t(bit_scan_reverse_result - fp.precision));
        }
    } else {
        ap_destroy(fp.exponent);
        fp.exponent = ap_from_intmax_t(0);
    }
    return fp;
}

struct fp_from_long_double_result fp_from_long_double(long double long_double) {
    uint64_t long_double_significand;
    uint16_t long_double_sign_exponent;
    memcpy(&long_double_significand, (char *) &long_double, 8);
    memcpy(&long_double_sign_exponent, ((char *) &long_double) + 8, 2);
    bool negative = (long_double_sign_exponent & ((uint16_t) 1 << 15)) != 0;
    uint16_t exponent = long_double_sign_exponent & (((uint16_t) 1 << 15) - 1);
    struct fp fp = fp_create_empty();
    ap_destroy(fp.significand);
    ap_destroy(fp.exponent);
    fp.precision = 63;
    fp.significand = ap_from_uintmax_t(long_double_significand);
    fp.exponent = ap_subtract(ap_from_uintmax_t(exponent), ap_from_intmax_t(16383));
    if (negative) {
        fp.significand = ap_negate(fp.significand);
    }
    if (exponent == 0) {
        fp.exponent = ap_add(fp.exponent, ap_from_intmax_t(1));
    }
    bool infinity = false;
    bool nan = false;
    if (exponent == ((uint16_t) 1 << 15) - 1) {
        if (long_double_significand << 1 == 0) {
            infinity = true;
        } else {
            nan = true;
        }
    }
    return (struct fp_from_long_double_result) {
        .fp = fp_normalize(fp),
        .negative = negative,
        .infinity = infinity,
        .nan = nan,
    };
}

long double fp_to_long_double(struct fp fp) {
    fp = fp_shrink(fp_extend(fp, 63), 63);
    bool negative = fp_sign(fp_copy(fp)) < 0;
    uint16_t exponent = (uint16_t) ap_to_intmax_t(fp.exponent);
    uint64_t long_double_significand = (uint64_t) ap_to_uintmax_t(ap_abs(fp.significand));
    uint16_t long_double_sign_exponent = (negative ? ((uint16_t) 1 << 15) : 0) | (exponent + 16383);
    long double long_double;
    memcpy((char *) &long_double, &long_double_significand, 8);
    memcpy(((char *) &long_double) + 8, &long_double_sign_exponent, 2);
    return long_double;
}

struct fp fp_from_ap(struct ap ap, size_t precision) {
    struct fp fp = fp_create_empty();
    ap_destroy(fp.significand);
    ap_destroy(fp.exponent);
    fp.precision = precision;
    fp.significand = ap;
    fp.exponent = ap_from_uintmax_t(precision);
    return fp_normalize(fp);
}

struct ap fp_to_ap(struct fp fp) {
    struct ap real_exponent = ap_subtract(fp.exponent, ap_from_uintmax_t(fp.precision));
    size_t real_exponent_size_t = ap_to_uintmax_t(ap_abs(ap_copy(real_exponent)));
    if (ap_sign(real_exponent) >= 0) {
        return ap_left_shift(fp.significand, real_exponent_size_t);
    } else {
        return ap_right_shift(fp.significand, real_exponent_size_t);
    }
}

struct fp fp_negate(struct fp fp) {
    fp.significand = ap_negate(fp.significand);
    return fp_normalize(fp);
}

struct fp fp_add(struct fp x, struct fp y) {
    struct fp result = fp_create_empty();
    result = fp_extend(result, x.precision);
    result = fp_extend(result, y.precision);
    x = fp_extend(x, result.precision);
    y = fp_extend(y, result.precision);
    if (ap_compare(ap_copy(x.exponent), ap_copy(y.exponent)) < 0) {
        struct fp tmp = x;
        x = y;
        y = tmp;
    }
    struct ap exponent_difference = ap_subtract(ap_copy(x.exponent), ap_copy(y.exponent));
    if (ap_compare(ap_copy(exponent_difference), ap_from_uintmax_t(result.precision)) >= 0) {
        ap_destroy(exponent_difference);
        fp_destroy(y);
        return x;
    }
    ap_destroy(x.exponent);
    ap_destroy(result.significand);
    ap_destroy(result.exponent);
    result.significand = ap_add(ap_left_shift(x.significand, ap_to_uintmax_t(exponent_difference)), y.significand);
    result.exponent = y.exponent;
    return fp_normalize(result);
}

struct fp fp_subtract(struct fp x, struct fp y) {
    return fp_add(x, fp_negate(y));
}

struct fp fp_multiply(struct fp x, struct fp y) {
    struct fp result = fp_create_empty();
    result = fp_extend(result, x.precision);
    result = fp_extend(result, y.precision);
    x = fp_extend(x, result.precision);
    y = fp_extend(y, result.precision);
    ap_destroy(result.significand);
    ap_destroy(result.exponent);
    result.significand = ap_multiply(x.significand, y.significand);
    result.exponent = ap_subtract(ap_add(x.exponent, y.exponent), ap_from_uintmax_t(result.precision));
    return fp_normalize(result);
}

struct fp fp_divide(struct fp numerator, struct fp denominator) {
    struct fp result = fp_create_empty();
    result = fp_extend(result, numerator.precision);
    result = fp_extend(result, denominator.precision);
    numerator = fp_extend(numerator, result.precision);
    denominator = fp_extend(denominator, result.precision);
    ap_destroy(result.significand);
    ap_destroy(result.exponent);
    struct ap_division_result ap_division_result = ap_divide(ap_left_shift(numerator.significand, result.precision), denominator.significand);
    ap_destroy(ap_division_result.remainder);
    result.significand = ap_division_result.quotient;
    result.exponent = ap_subtract(numerator.exponent, denominator.exponent);
    return fp_normalize(result);
}

struct fp fp_integer_power(struct fp base, struct ap exponent) {
    int8_t sign = ap_sign(ap_copy(exponent));
    if (sign < 0) {
        return fp_divide(fp_from_long_double(1.0).fp, fp_integer_power(base, ap_negate(exponent)));
    }
    if (sign == 0) {
        fp_destroy(base);
        ap_destroy(exponent);
        return fp_from_long_double(1.0).fp;
    }
    struct ap_division_result ap_division_result = ap_divide(exponent, ap_from_intmax_t(2));
    struct fp half_power = fp_integer_power(fp_copy(base), ap_division_result.quotient);
    struct fp power = fp_multiply(fp_copy(half_power), half_power);
    if (ap_sign(ap_division_result.remainder) == 0) {
        fp_destroy(base);
        return power;
    } else {
        return fp_multiply(power, base);
    }
}

int8_t fp_sign(struct fp fp) {
    ap_destroy(fp.exponent);
    return ap_sign(fp.significand);
}

int8_t fp_compare(struct fp x, struct fp y) {
    return fp_sign(fp_subtract(x, y));
}

struct fp fp_abs(struct fp fp) {
    if (fp_sign(fp_copy(fp)) < 0) {
        return fp_negate(fp);
    } else {
        return fp;
    }
}

void fp_destroy(struct fp fp) {
    ap_destroy(fp.significand);
    ap_destroy(fp.exponent);
}
