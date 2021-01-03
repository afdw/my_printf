#ifndef MY_PRINTF_PARSING_H
#define MY_PRINTF_PARSING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include "fp.h"

struct conversion_specification_flags {
    bool hash;
    bool zero;
    bool minus;
    bool space;
    bool plus;
};

enum length_modifier {
    LENGTH_MODIFIER_none,
    LENGTH_MODIFIER_hh,
    LENGTH_MODIFIER_h,
    LENGTH_MODIFIER_l,
    LENGTH_MODIFIER_ll,
    LENGTH_MODIFIER_L,
    LENGTH_MODIFIER_j,
    LENGTH_MODIFIER_z,
    LENGTH_MODIFIER_t,
    LENGTH_MODIFIER_Z,
    LENGTH_MODIFIER_F,
};

enum conversion_specifier {
    CONVERSION_SPECIFIER_literal,
    CONVERSION_SPECIFIER_d,
    CONVERSION_SPECIFIER_i,
    CONVERSION_SPECIFIER_o,
    CONVERSION_SPECIFIER_u,
    CONVERSION_SPECIFIER_x,
    CONVERSION_SPECIFIER_X,
    CONVERSION_SPECIFIER_e,
    CONVERSION_SPECIFIER_E,
    CONVERSION_SPECIFIER_f,
    CONVERSION_SPECIFIER_F,
    CONVERSION_SPECIFIER_g,
    CONVERSION_SPECIFIER_G,
    CONVERSION_SPECIFIER_a,
    CONVERSION_SPECIFIER_A,
    CONVERSION_SPECIFIER_c,
    CONVERSION_SPECIFIER_s,
    CONVERSION_SPECIFIER_p,
    CONVERSION_SPECIFIER_n,
};

struct conversion_specification {
    struct conversion_specification_flags conversion_specification_flags;
    size_t field_width_argument_index;
    int32_t field_width;
    size_t precision_argument_index;
    int32_t precision;
    enum length_modifier length_modifier;
    enum conversion_specifier conversion_specifier;
    size_t data_argument_index;
    union {
        char data_char;
        intmax_t data_intmax_t;
        uintmax_t data_uintmax_t;
        long double data_long_double;
        struct ap data_ap;
        struct fp data_fp;
        void *data_void_pointer;
        char *data_char_pointer;
        signed char *data_signed_char_pointer;
        short *data_short_pointer;
        int *data_int_pointer;
        long *data_long_pointer;
        long long *data_long_long_pointer;
        intmax_t *data_intmax_t_pointer;
        size_t *data_size_t_pointer;
        ptrdiff_t *data_ptrdiff_t_pointer;
        struct ap *data_ap_pointer;
    };
};

struct parsed_format {
    size_t length;
    struct conversion_specification *conversion_specifications;
};

struct parsed_format parse(const char *format);

void parsed_format_destroy(struct parsed_format parsed_format);

#endif
