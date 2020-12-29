#include "ap.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static struct ap ap_create_empty() {
    return (struct ap) {
        .length = 0,
        .bytes = NULL,
    };
}

struct ap ap_copy(struct ap ap) {
    if (ap.length != 0) {
        struct ap copy = {
            .length = ap.length,
            .bytes = malloc(ap.length),
        };
        memcpy(copy.bytes, ap.bytes, ap.length);
        return copy;
    } else {
        return ap_create_empty();
    }
}

struct ap ap_reserve(struct ap ap, size_t new_length) {
    if (ap.length < new_length) {
        bool negative = ap_sign(ap_copy(ap)) < 0;
        ap.bytes = realloc(ap.bytes, new_length);
        for (size_t i = ap.length; i < new_length; i++) {
            ap.bytes[i] = negative ? 0xFF : 0;
        }
        ap.length = new_length;
    }
    return ap;
}

struct ap ap_shrink(struct ap ap, size_t new_length) {
    if (new_length != 0) {
        ap.length = new_length;
        ap.bytes = realloc(ap.bytes, new_length);
        return ap;
    } else {
        ap_destroy(ap);
        return ap_create_empty();
    }
}

struct ap ap_shrink_to_fit(struct ap ap) {
    size_t new_length = 0;
    if (ap.length != 0) {
        if (ap.bytes[ap.length - 1] == 0) {
            for (size_t i = ap.length - 1;; i--) {
                if (ap.bytes[i] != 0) {
                    if ((ap.bytes[i] & 0b10000000) != 0 && i != ap.length - 1) {
                        new_length = i + 2;
                    } else {
                        new_length = i + 1;
                    }
                    break;
                }
                if (i == 0) {
                    break;
                }
            }
        } else if (ap.bytes[ap.length - 1] == 0xFF) {
            for (size_t i = ap.length - 1;; i--) {
                if (ap.bytes[i] != 0xFF) {
                    if ((ap.bytes[i] & 0b10000000) == 0 && i != ap.length - 1) {
                        new_length = i + 2;
                    } else {
                        new_length = i + 1;
                    }
                    break;
                }
                if (i == 0) {
                    new_length = 1;
                    break;
                }
            }
        } else {
            new_length = ap.length;
        }
    }
    return ap_shrink(ap, new_length);
}

struct ap ap_from_intmax_t(intmax_t intmax_t) {
    uintmax_t uintmax_t = intmax_t;
    struct ap ap = ap_reserve(ap_create_empty(), sizeof(uintmax_t));
    for (size_t i = 0; i < sizeof(uintmax_t); i++) {
        ap.bytes[i] = (uint8_t) ((uintmax_t >> (i * 8)) & 0xFF);
    }
    return ap_shrink_to_fit(ap);
}

struct ap ap_from_uintmax_t(uintmax_t uintmax_t) {
    struct ap ap = ap_reserve(ap_create_empty(), sizeof(uintmax_t) + 1);
    for (size_t i = 0; i < sizeof(uintmax_t); i++) {
        ap.bytes[i] = (uint8_t) ((uintmax_t >> (i * 8)) & 0xFF);
    }
    return ap_shrink_to_fit(ap);
}

intmax_t ap_to_intmax_t(struct ap ap) {
    return ap_to_uintmax_t(ap);
}

uintmax_t ap_to_uintmax_t(struct ap ap) {
    ap = ap_reserve(ap, sizeof(uintmax_t));
    uintmax_t result = 0;
    for (size_t i = 0; i < sizeof(result); i++) {
        result |= ((uintmax_t) ap.bytes[i]) << (i * 8);
    }
    ap_destroy(ap);
    return result;
}

struct ap ap_not(struct ap ap) {
    for (size_t i = 0; i < ap.length; i++) {
        ap.bytes[i] = ~ap.bytes[i];
    }
    return ap_shrink_to_fit(ap);
}

struct ap ap_and(struct ap x, struct ap y) {
    struct ap result = ap_create_empty();
    result = ap_reserve(result, x.length);
    result = ap_reserve(result, y.length);
    x = ap_reserve(x, result.length);
    y = ap_reserve(y, result.length);
    for (size_t i = 0; i < result.length; i++) {
        result.bytes[i] = x.bytes[i] & y.bytes[i];
    }
    ap_destroy(x);
    ap_destroy(y);
    return ap_shrink_to_fit(result);
}

struct ap ap_or(struct ap x, struct ap y) {
    struct ap result = ap_create_empty();
    result = ap_reserve(result, x.length);
    result = ap_reserve(result, y.length);
    x = ap_reserve(x, result.length);
    y = ap_reserve(y, result.length);
    for (size_t i = 0; i < result.length; i++) {
        result.bytes[i] = x.bytes[i] | y.bytes[i];
    }
    ap_destroy(x);
    ap_destroy(y);
    return ap_shrink_to_fit(result);
}

struct ap ap_xor(struct ap x, struct ap y) {
    struct ap result = ap_create_empty();
    result = ap_reserve(result, x.length);
    result = ap_reserve(result, y.length);
    x = ap_reserve(x, result.length);
    y = ap_reserve(y, result.length);
    for (size_t i = 0; i < result.length; i++) {
        result.bytes[i] = x.bytes[i] ^ y.bytes[i];
    }
    ap_destroy(x);
    ap_destroy(y);
    return ap_shrink_to_fit(result);
}

struct ap ap_left_shift(struct ap ap, size_t amount) {
    struct ap result = ap_reserve(ap_create_empty(), ap.length + (amount + 8 - 1) / 8);
    ap = ap_reserve(ap, ap.length + 1);
    for (size_t i = 0; i < ap.length; i++) {
        if (i + amount / 8 < result.length) {
            result.bytes[i + amount / 8] |= (ap.bytes[i] << (amount % 8)) & 0xFF;
        }
        if (i + amount / 8 + 1 < result.length) {
            result.bytes[i + amount / 8 + 1] |= (uint8_t) (((uint16_t) ap.bytes[i] >> (8 - amount % 8)) & 0xFF);
        }
    }
    ap_destroy(ap);
    return ap_shrink_to_fit(result);
}

struct ap ap_right_shift(struct ap ap, size_t amount) {
    struct ap result = ap_reserve(ap_create_empty(), ap.length - amount / 8);
    ap = ap_reserve(ap, ap.length + 1);
    for (size_t i = 0; i < ap.length; i++) {
        if (i >= amount / 8 && i < result.length + amount / 8) {
            result.bytes[i - amount / 8] |= (ap.bytes[i] >> (amount % 8)) & 0xFF;
        }
        if (i + 1 >= amount / 8 && i + 1 < result.length + amount / 8) {
            result.bytes[i - amount / 8 + 1] |= (uint8_t) (((uint16_t) ap.bytes[i] << (8 - amount % 8)) & 0xFF);
        }
    }
    ap_destroy(ap);
    return ap_shrink_to_fit(result);
}

struct ap ap_negate(struct ap ap) {
    return ap_shrink_to_fit(ap_add(ap_not(ap), ap_from_intmax_t(1)));
}

struct ap ap_add(struct ap x, struct ap y) {
    struct ap result = ap_create_empty();
    result = ap_reserve(result, x.length + 1);
    result = ap_reserve(result, y.length + 1);
    x = ap_reserve(x, result.length);
    y = ap_reserve(y, result.length);
    uint8_t carry = 0;
    for (size_t i = 0; i < result.length; i++) {
        uint16_t addition_result = (uint16_t) x.bytes[i] + (uint16_t) y.bytes[i] + (uint16_t) carry;
        result.bytes[i] = (uint8_t) (addition_result & 0xFF);
        carry = (uint8_t) ((addition_result >> 8) & 0xFF);
    }
    ap_destroy(x);
    ap_destroy(y);
    return ap_shrink_to_fit(result);
}

struct ap ap_subtract(struct ap x, struct ap y) {
    return ap_add(x, ap_negate(y));
}

static struct ap ap_single_multiply(struct ap ap, size_t byte) {
    ap = ap_reserve(ap, ap.length + 1);
    uint8_t carry = 0;
    for (size_t i = 0; i < ap.length; i++) {
        uint16_t multiplication_result = (uint16_t) ap.bytes[i] * (uint16_t) byte + (uint16_t) carry;
        ap.bytes[i] = (uint8_t) (multiplication_result & 0xFF);
        carry = (uint8_t) ((multiplication_result >> 8) & 0xFF);
    }
    return ap_shrink_to_fit(ap);
}

struct ap ap_multiply(struct ap x, struct ap y) {
    struct ap result = ap_create_empty();
    for (size_t i = 0; i < y.length; i++) {
        result = ap_add(result, ap_left_shift(ap_single_multiply(ap_copy(x), y.bytes[i]), i * 8));
    }
    size_t result_length = x.length + y.length;
    ap_destroy(x);
    ap_destroy(y);
    return ap_shrink_to_fit(ap_shrink(ap_reserve(result, result_length), result_length));
}

static struct ap_division_result ap_unsigned_divide(struct ap numerator, struct ap denominator) {
    struct ap denominator_shifts[8];
    for (size_t i = 0; i < 8; i++) {
        denominator_shifts[i] = ap_left_shift(ap_copy(denominator), i);
    }
    ap_destroy(denominator);
    struct ap quotient = ap_reserve(ap_create_empty(), numerator.length);
    struct ap remainder = ap_create_empty();
    if (numerator.length != 0) {
        for (size_t i = numerator.length - 1;; i--) {
            remainder = ap_add(ap_left_shift(remainder, 8), ap_from_uintmax_t(numerator.bytes[i]));
            for (size_t j = 8 - 1;; j--) {
                if (ap_compare(ap_copy(remainder), ap_copy(denominator_shifts[j])) >= 0) {
                    remainder = ap_subtract(remainder, ap_copy(denominator_shifts[j]));
                    quotient.bytes[i] += 1 << j;
                }
                if (j == 0) {
                    break;
                }
            }
            if (i == 0) {
                break;
            }
        }
    }
    ap_destroy(numerator);
    for (size_t i = 0; i < 8; i++) {
        ap_destroy(denominator_shifts[i]);
    }
    return (struct ap_division_result) {.quotient = ap_shrink_to_fit(quotient), .remainder = remainder};
}

struct ap_division_result ap_divide(struct ap numerator, struct ap denominator) {
    bool numerator_negative = ap_sign(ap_copy(numerator)) < 0;
    bool denominator_negative = ap_sign(ap_copy(denominator)) < 0;
    struct ap_division_result ap_division_result = ap_unsigned_divide(ap_abs(numerator), ap_abs(denominator));
    if (numerator_negative != denominator_negative) {
        ap_division_result.quotient = ap_negate(ap_division_result.quotient);
    }
    if (numerator_negative) {
        ap_division_result.remainder = ap_negate(ap_division_result.remainder);
    }
    return ap_division_result;
}

struct ap ap_power(struct ap base, struct ap exponent) {
    if (ap_sign(ap_copy(exponent)) < 0) {
        ap_destroy(base);
        ap_destroy(exponent);
        return ap_from_intmax_t(0);
    }
    struct ap result = ap_from_intmax_t(1);
    for (; ap_sign(ap_copy(exponent)) > 0; exponent = ap_subtract(exponent, ap_from_intmax_t(1))) {
        result = ap_multiply(result, ap_copy(base));
    }
    ap_destroy(base);
    ap_destroy(exponent);
    return result;
}

int8_t ap_sign(struct ap ap) {
    if (ap.length == 0) {
        ap_destroy(ap);
        return 0;
    }
    if ((ap.bytes[ap.length - 1] & 0b10000000) != 0) {
        ap_destroy(ap);
        return -1;
    }
    for (size_t i = 0; i < ap.length; i++) {
        if (ap.bytes[i] != 0) {
            ap_destroy(ap);
            return 1;
        }
    }
    ap_destroy(ap);
    return 0;
}

int8_t ap_compare(struct ap x, struct ap y) {
    return ap_sign(ap_subtract(x, y));
}

struct ap ap_abs(struct ap ap) {
    if (ap_sign(ap_copy(ap)) < 0) {
        return ap_negate(ap);
    } else {
        return ap;
    }
}

void ap_destroy(struct ap ap) {
    free(ap.bytes);
}
