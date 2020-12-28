#include "parsing.h"

#include <stdlib.h>
#include <string.h>

static int32_t parse_int(const char *input, size_t *i) {
    int32_t result = 0;
    bool negative = false;
    if (input[*i] == '-') {
        negative = true;
        (*i)++;
    }
    for (; input[*i] >= '0' && input[*i] <= '9'; (*i)++) {
        result = result * 10 + (int32_t) (input[*i] - '0');
    }
    if (negative) {
        result *= -1;
    }
    return result;
}

static size_t parse_argument_index(const char *format, size_t *i, size_t *current_sequential_argument_index) {
    size_t j = *i;
    int32_t parsed = parse_int(format, &j);
    if (format[j] == '$') {
        *i = j + 1;
        return (size_t) parsed - 1;
    } else {
        if (current_sequential_argument_index == NULL) {
            return 0;
        } else {
            (*current_sequential_argument_index)++;
            return *current_sequential_argument_index - 1;
        }
    }
}

static void parse_literal_or_field_supplied(const char *format, size_t *i, size_t *current_sequential_argument_index, size_t *value_argument_index, int32_t *value) {
    if (format[*i] == '*') {
        (*i)++;
        *value_argument_index = parse_argument_index(format, i, current_sequential_argument_index);
    } else {
        *value_argument_index = (size_t) -1;
        *value = parse_int(format, i);
    }
}

static size_t parse_internal(const char *format, struct parsed_format parsed_format) {
    size_t format_length = strlen(format);
    size_t used_conversion_specifications = 0;
    size_t current_sequential_argument_index = 0;
    for (size_t i = 0; i < format_length; i++) {
        struct conversion_specification conversion_specification = {
            .conversion_specification_flags = {},
            .field_width_argument_index = (size_t) -1,
            .field_width = -1,
            .precision_argument_index = (size_t) -1,
            .precision = -1,
            .length_modifier = LENGTH_MODIFIER_none,
            .conversion_specifier = CONVERSION_SPECIFIER_literal,
            .data_argument_index = (size_t) -1,
            .data_void_pointer = NULL,
        };
        if (format[i] == '%') {
            i++;
            size_t data_argument_index_start = i;
            parse_argument_index(format, &i, NULL);
            for (; format[i] == '#' || format[i] == '0' || format[i] == '-' || format[i] == ' ' || format[i] == '+'; i++) {
                if (format[i] == '#') {
                    conversion_specification.conversion_specification_flags.hash = true;
                }
                if (format[i] == '0') {
                    conversion_specification.conversion_specification_flags.zero = true;
                }
                if (format[i] == '-') {
                    conversion_specification.conversion_specification_flags.minus = true;
                }
                if (format[i] == ' ') {
                    conversion_specification.conversion_specification_flags.space = true;
                }
                if (format[i] == '+') {
                    conversion_specification.conversion_specification_flags.plus = true;
                }
            }
            if (format[i] >= '0' && format[i] <= '9') {
                parse_literal_or_field_supplied(format, &i, &current_sequential_argument_index, &conversion_specification.field_width_argument_index, &conversion_specification.field_width);
            }
            if (format[i] == '.') {
                i++;
                parse_literal_or_field_supplied(format, &i, &current_sequential_argument_index, &conversion_specification.precision_argument_index, &conversion_specification.precision);
                if (conversion_specification.precision < 0) {
                    conversion_specification.precision = -1;
                }
            }
            conversion_specification.data_argument_index = parse_argument_index(format, &data_argument_index_start, &current_sequential_argument_index);
            if (format[i] == 'h' && format[i + 1] == 'h') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_hh;
                i += 2;
            }
            if (format[i] == 'h' && format[i + 1] != 'h') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_h;
                i++;
            }
            if (format[i] == 'l' && format[i + 1] != 'l') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_l;
                i++;
            }
            if (format[i] == 'l' && format[i + 1] == 'l') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_ll;
                i += 2;
            }
            if (format[i] == 'L') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_L;
                i++;
            }
            if (format[i] == 'j') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_j;
                i++;
            }
            if (format[i] == 'z') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_z;
                i++;
            }
            if (format[i] == 't') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_t;
                i++;
            }
            if (format[i] == 'Z') {
                conversion_specification.length_modifier = LENGTH_MODIFIER_Z;
                i++;
            }
#define MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(x) if (format[i] == #x[0]) { \
    conversion_specification.conversion_specifier = CONVERSION_SPECIFIER_##x; \
}
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(d)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(i)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(o)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(u)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(x)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(X)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(e)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(E)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(f)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(F)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(g)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(G)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(a)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(A)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(c)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(s)
            MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER(p)
#undef MY_PRINTF_PARSING_C_PARSE_CONVERSION_SPECIFIER
            if (format[i] == '%') {
                conversion_specification.conversion_specifier = CONVERSION_SPECIFIER_literal;
                conversion_specification.data_char = '%';
            }
        } else {
            conversion_specification.conversion_specifier = CONVERSION_SPECIFIER_literal;
            conversion_specification.data_char = format[i];
        }
        if (used_conversion_specifications < parsed_format.length) {
            parsed_format.conversion_specifications[used_conversion_specifications] = conversion_specification;
        }
        used_conversion_specifications++;
    }
    return used_conversion_specifications;
}

struct parsed_format parse(const char *format) {
    struct parsed_format parsed_format = {.length = 0, .conversion_specifications = NULL};
    parsed_format.length = parse_internal(format, parsed_format);
    parsed_format.conversion_specifications = malloc(sizeof(struct conversion_specification) * parsed_format.length);
    parse_internal(format, parsed_format);
    return parsed_format;
}

void parsed_format_destroy(struct parsed_format parsed_format) {
    free(parsed_format.conversion_specifications);
}
