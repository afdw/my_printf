#ifndef MY_PRINTF_PARSING_H
#define MY_PRINTF_PARSING_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

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
    const void *data;
};

struct parsed_format {
    size_t length;
    struct conversion_specification *conversion_specifications;
};

struct parsed_format parse(const char *format);

void parsed_format_destroy(struct parsed_format parsed_format);

#endif
