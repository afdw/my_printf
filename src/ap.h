#ifndef MY_PRINTF_AP_H
#define MY_PRINTF_AP_H

#include <stddef.h>
#include <stdint.h>

struct ap {
    size_t length;
    uint8_t *bytes;
};

struct ap_division_result {
    struct ap quotient;
    struct ap remainder;
};

struct ap ap_copy(struct ap ap);

struct ap ap_reserve(struct ap ap, size_t new_length);

struct ap ap_shrink(struct ap ap, size_t new_length);

struct ap ap_shrink_to_fit(struct ap ap);

struct ap ap_from_intmax_t(intmax_t intmax_t);

struct ap ap_from_uintmax_t(uintmax_t uintmax_t);

intmax_t ap_to_intmax_t(struct ap ap);

uintmax_t ap_to_uintmax_t(struct ap ap);

struct ap ap_not(struct ap ap);

struct ap ap_and(struct ap x, struct ap y);

struct ap ap_or(struct ap x, struct ap y);

struct ap ap_xor(struct ap x, struct ap y);

struct ap ap_left_shift(struct ap ap, size_t amount);

struct ap ap_right_shift(struct ap ap, size_t amount);

struct ap ap_negate(struct ap ap);

struct ap ap_add(struct ap x, struct ap y);

struct ap ap_subtract(struct ap x, struct ap y);

struct ap ap_multiply(struct ap x, struct ap y);

struct ap_division_result ap_divide(struct ap numerator, struct ap denominator);

int8_t ap_sign(struct ap ap);

int8_t ap_compare(struct ap x, struct ap y);

struct ap ap_abs(struct ap ap);

void ap_destroy(struct ap ap);

#endif
