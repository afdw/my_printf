#ifndef MY_PRINTF_FP_H
#define MY_PRINTF_FP_H

#include <stdbool.h>
#include "ap.h"

struct fp {
    size_t precision;
    struct ap significand;
    struct ap exponent;
};

struct fp_from_long_double_result {
    struct fp fp;
    bool negative;
    bool infinity;
    bool nan;
};

struct fp fp_copy(struct fp fp);

struct fp fp_extend(struct fp fp, size_t new_precision);

struct fp fp_shrink(struct fp fp, size_t new_precision);

struct fp fp_normalize(struct fp fp);

struct fp_from_long_double_result fp_from_long_double(long double long_double);

long double fp_to_long_double(struct fp fp);

struct fp fp_negate(struct fp fp);

struct fp fp_add(struct fp x, struct fp y);

struct fp fp_subtract(struct fp x, struct fp y);

struct fp fp_multiply(struct fp x, struct fp y);

struct fp fp_divide(struct fp numerator, struct fp denominator);

struct fp fp_integer_power(struct fp base, struct ap exponent);

int8_t fp_sign(struct fp fp);

int8_t fp_compare(struct fp x, struct fp y);

struct fp fp_abs(struct fp fp);

void fp_destroy(struct fp fp);

#endif
