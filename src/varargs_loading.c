#include "varargs_loading.h"

#include <stdlib.h>

struct parsed_format load_varargs(const char *format, va_list va_list) {
    enum argument_type {
        ARGUMENT_TYPE_INT,
        ARGUMENT_TYPE_UNSIGNED_INT,
        ARGUMENT_TYPE_LONG,
        ARGUMENT_TYPE_UNSIGNED_LONG,
        ARGUMENT_TYPE_LONG_LONG,
        ARGUMENT_TYPE_UNSIGNED_LONG_LONG,
        ARGUMENT_TYPE_DOUBLE,
        ARGUMENT_TYPE_LONG_DOUBLE,
        ARGUMENT_TYPE_INTMAX_T,
        ARGUMENT_TYPE_UINTMAX_T,
        ARGUMENT_TYPE_SIZE_T,
        ARGUMENT_TYPE_PTRDIFF_T,
        ARGUMENT_TYPE_AP,
        ARGUMENT_TYPE_FP,
        ARGUMENT_TYPE_VOID_POINTER,
        ARGUMENT_TYPE_CHAR_POINTER,
    };
    union argument {
        int argument_int;
        unsigned int argument_unsigned_int;
        long argument_long;
        unsigned long argument_unsigned_long;
        long long argument_long_long;
        unsigned long long argument_unsigned_long_long;
        double argument_double;
        long double argument_long_double;
        intmax_t argument_intmax_t;
        uintmax_t argument_uintmax_t;
        size_t argument_size_t;
        ptrdiff_t argument_ptrdiff_t;
        struct ap argument_ap;
        struct fp argument_fp;
        void *argument_void_pointer;
        char *argument_char_pointer;
    };
    struct parsed_format parsed_format = parse(format);
    size_t arguments_count = 0;
    for (size_t i = 0; i < parsed_format.length; i++) {
        if (parsed_format.conversion_specifications[i].field_width_argument_index != (size_t) -1 && parsed_format.conversion_specifications[i].field_width_argument_index + 1 > arguments_count) {
            arguments_count = parsed_format.conversion_specifications[i].field_width_argument_index + 1;
        }
        if (parsed_format.conversion_specifications[i].precision_argument_index != (size_t) -1 && parsed_format.conversion_specifications[i].precision_argument_index + 1 > arguments_count) {
            arguments_count = parsed_format.conversion_specifications[i].precision_argument_index + 1;
        }
        if (parsed_format.conversion_specifications[i].data_argument_index != (size_t) -1 && parsed_format.conversion_specifications[i].data_argument_index + 1 > arguments_count) {
            arguments_count = parsed_format.conversion_specifications[i].data_argument_index + 1;
        }
    }
    enum argument_type *argument_types = malloc(sizeof(enum argument_type) * arguments_count);
    for (size_t i = 0; i < parsed_format.length; i++) {
        if (parsed_format.conversion_specifications[i].field_width_argument_index != -1) {
            argument_types[parsed_format.conversion_specifications[i].field_width_argument_index] = ARGUMENT_TYPE_INT;
        }
        if (parsed_format.conversion_specifications[i].precision_argument_index != -1) {
            argument_types[parsed_format.conversion_specifications[i].precision_argument_index] = ARGUMENT_TYPE_INT;
        }
        enum length_modifier length_modifier = parsed_format.conversion_specifications[i].length_modifier;
        enum argument_type data_argument_type;
        switch (parsed_format.conversion_specifications[i].conversion_specifier) {
            case CONVERSION_SPECIFIER_literal:
                continue;
            case CONVERSION_SPECIFIER_d:
            case CONVERSION_SPECIFIER_i:
                if (length_modifier == LENGTH_MODIFIER_l) {
                    data_argument_type = ARGUMENT_TYPE_LONG;
                } else if (length_modifier == LENGTH_MODIFIER_ll) {
                    data_argument_type = ARGUMENT_TYPE_LONG_LONG;
                } else if (length_modifier == LENGTH_MODIFIER_j) {
                    data_argument_type = ARGUMENT_TYPE_INTMAX_T;
                } else if (length_modifier == LENGTH_MODIFIER_z) {
                    data_argument_type = ARGUMENT_TYPE_SIZE_T;
                } else if (length_modifier == LENGTH_MODIFIER_t) {
                    data_argument_type = ARGUMENT_TYPE_PTRDIFF_T;
                } else if (length_modifier == LENGTH_MODIFIER_Z) {
                    data_argument_type = ARGUMENT_TYPE_AP;
                } else {
                    data_argument_type = ARGUMENT_TYPE_INT;
                }
                break;
            case CONVERSION_SPECIFIER_o:
            case CONVERSION_SPECIFIER_u:
            case CONVERSION_SPECIFIER_x:
            case CONVERSION_SPECIFIER_X:
                if (length_modifier == LENGTH_MODIFIER_l) {
                    data_argument_type = ARGUMENT_TYPE_UNSIGNED_LONG;
                } else if (length_modifier == LENGTH_MODIFIER_ll) {
                    data_argument_type = ARGUMENT_TYPE_UNSIGNED_LONG_LONG;
                } else if (length_modifier == LENGTH_MODIFIER_j) {
                    data_argument_type = ARGUMENT_TYPE_UINTMAX_T;
                } else if (length_modifier == LENGTH_MODIFIER_z) {
                    data_argument_type = ARGUMENT_TYPE_SIZE_T;
                } else if (length_modifier == LENGTH_MODIFIER_t) {
                    data_argument_type = ARGUMENT_TYPE_PTRDIFF_T;
                } else if (length_modifier == LENGTH_MODIFIER_Z) {
                    data_argument_type = ARGUMENT_TYPE_AP;
                } else {
                    data_argument_type = ARGUMENT_TYPE_UNSIGNED_INT;
                }
                break;
            case CONVERSION_SPECIFIER_e:
            case CONVERSION_SPECIFIER_E:
            case CONVERSION_SPECIFIER_f:
            case CONVERSION_SPECIFIER_F:
            case CONVERSION_SPECIFIER_g:
            case CONVERSION_SPECIFIER_G:
            case CONVERSION_SPECIFIER_a:
            case CONVERSION_SPECIFIER_A:
                if (length_modifier == LENGTH_MODIFIER_L) {
                    data_argument_type = ARGUMENT_TYPE_LONG_DOUBLE;
                } else if (length_modifier == LENGTH_MODIFIER_F) {
                    data_argument_type = ARGUMENT_TYPE_FP;
                } else {
                    data_argument_type = ARGUMENT_TYPE_DOUBLE;
                }
                break;
            case CONVERSION_SPECIFIER_c:
                data_argument_type = ARGUMENT_TYPE_INT;
                break;
            case CONVERSION_SPECIFIER_s:
                data_argument_type = ARGUMENT_TYPE_CHAR_POINTER;
                break;
            case CONVERSION_SPECIFIER_p:
                data_argument_type = ARGUMENT_TYPE_VOID_POINTER;
                break;
        }
        argument_types[parsed_format.conversion_specifications[i].data_argument_index] = data_argument_type;
    }
    union argument *arguments = malloc(sizeof(union argument) * arguments_count);
    for (size_t i = 0; i < arguments_count; i++) {
        switch (argument_types[i]) {
            case ARGUMENT_TYPE_INT:
                arguments[i].argument_int = va_arg(va_list, int);
                break;
            case ARGUMENT_TYPE_UNSIGNED_INT:
                arguments[i].argument_unsigned_int = va_arg(va_list, unsigned int);
                break;
            case ARGUMENT_TYPE_LONG:
                arguments[i].argument_long = va_arg(va_list, long);
                break;
            case ARGUMENT_TYPE_UNSIGNED_LONG:
                arguments[i].argument_unsigned_long = va_arg(va_list, unsigned long);
                break;
            case ARGUMENT_TYPE_LONG_LONG:
                arguments[i].argument_long_long = va_arg(va_list, long long);
                break;
            case ARGUMENT_TYPE_UNSIGNED_LONG_LONG:
                arguments[i].argument_unsigned_long_long = va_arg(va_list, unsigned long long);
                break;
            case ARGUMENT_TYPE_DOUBLE:
                arguments[i].argument_double = va_arg(va_list, double);
                break;
            case ARGUMENT_TYPE_LONG_DOUBLE:
                arguments[i].argument_long_double = va_arg(va_list, long double);
                break;
            case ARGUMENT_TYPE_INTMAX_T:
                arguments[i].argument_intmax_t = va_arg(va_list, intmax_t);
                break;
            case ARGUMENT_TYPE_UINTMAX_T:
                arguments[i].argument_uintmax_t = va_arg(va_list, uintmax_t);
                break;
            case ARGUMENT_TYPE_SIZE_T:
                arguments[i].argument_size_t = va_arg(va_list, size_t);
                break;
            case ARGUMENT_TYPE_PTRDIFF_T:
                arguments[i].argument_ptrdiff_t = va_arg(va_list, ptrdiff_t);
                break;
            case ARGUMENT_TYPE_AP:
                arguments[i].argument_ap = va_arg(va_list, struct ap);
                break;
            case ARGUMENT_TYPE_FP:
                arguments[i].argument_fp = va_arg(va_list, struct fp);
                break;
            case ARGUMENT_TYPE_VOID_POINTER:
                arguments[i].argument_void_pointer = va_arg(va_list, void *);
                break;
            case ARGUMENT_TYPE_CHAR_POINTER:
                arguments[i].argument_char_pointer = va_arg(va_list, char *);
                break;
        }
    }
    free(argument_types);
    for (size_t i = 0; i < parsed_format.length; i++) {
        if (parsed_format.conversion_specifications[i].field_width_argument_index != -1) {
            parsed_format.conversion_specifications[i].field_width = (int32_t) arguments[parsed_format.conversion_specifications[i].field_width_argument_index].argument_int;
            if (parsed_format.conversion_specifications[i].field_width < 0) {
                parsed_format.conversion_specifications[i].conversion_specification_flags.minus = true;
                parsed_format.conversion_specifications[i].field_width *= -1;
            }
            parsed_format.conversion_specifications[i].field_width_argument_index = (size_t) -1;
        }
        if (parsed_format.conversion_specifications[i].precision_argument_index != -1) {
            parsed_format.conversion_specifications[i].precision = (int32_t) arguments[parsed_format.conversion_specifications[i].precision_argument_index].argument_int;
            if (parsed_format.conversion_specifications[i].precision < 0) {
                parsed_format.conversion_specifications[i].precision = -1;
            }
            parsed_format.conversion_specifications[i].precision_argument_index = (size_t) -1;
        }
        enum length_modifier length_modifier = parsed_format.conversion_specifications[i].length_modifier;
        if (parsed_format.conversion_specifications[i].conversion_specifier == CONVERSION_SPECIFIER_literal) {
            continue;
        }
        union argument argument = arguments[parsed_format.conversion_specifications[i].data_argument_index];
        switch (parsed_format.conversion_specifications[i].conversion_specifier) {
            case CONVERSION_SPECIFIER_literal:
                break;
            case CONVERSION_SPECIFIER_d:
            case CONVERSION_SPECIFIER_i:
                if (length_modifier == LENGTH_MODIFIER_hh) {
                    parsed_format.conversion_specifications[i].data_intmax_t = (intmax_t) (signed char) argument.argument_int;
                } else if (length_modifier == LENGTH_MODIFIER_h) {
                    parsed_format.conversion_specifications[i].data_intmax_t = (short) argument.argument_int;
                } else if (length_modifier == LENGTH_MODIFIER_l) {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_long;
                } else if (length_modifier == LENGTH_MODIFIER_ll) {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_long_long;
                } else if (length_modifier == LENGTH_MODIFIER_j) {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_intmax_t;
                } else if (length_modifier == LENGTH_MODIFIER_z) {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_size_t;
                } else if (length_modifier == LENGTH_MODIFIER_t) {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_ptrdiff_t;
                } else if (length_modifier == LENGTH_MODIFIER_Z) {
                    parsed_format.conversion_specifications[i].data_ap = argument.argument_ap;
                } else {
                    parsed_format.conversion_specifications[i].data_intmax_t = argument.argument_int;
                }
                break;
            case CONVERSION_SPECIFIER_o:
            case CONVERSION_SPECIFIER_u:
            case CONVERSION_SPECIFIER_x:
            case CONVERSION_SPECIFIER_X:
                if (length_modifier == LENGTH_MODIFIER_hh) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = (uintmax_t) (unsigned char) argument.argument_int;
                } else if (length_modifier == LENGTH_MODIFIER_h) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = (unsigned short) argument.argument_unsigned_int;
                } else if (length_modifier == LENGTH_MODIFIER_l) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_unsigned_long;
                } else if (length_modifier == LENGTH_MODIFIER_ll) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_unsigned_long_long;
                } else if (length_modifier == LENGTH_MODIFIER_j) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_uintmax_t;
                } else if (length_modifier == LENGTH_MODIFIER_z) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_size_t;
                } else if (length_modifier == LENGTH_MODIFIER_t) {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_ptrdiff_t;
                } else if (length_modifier == LENGTH_MODIFIER_Z) {
                    parsed_format.conversion_specifications[i].data_ap = argument.argument_ap;
                } else {
                    parsed_format.conversion_specifications[i].data_uintmax_t = argument.argument_unsigned_int;
                }
                break;
            case CONVERSION_SPECIFIER_e:
            case CONVERSION_SPECIFIER_E:
            case CONVERSION_SPECIFIER_f:
            case CONVERSION_SPECIFIER_F:
            case CONVERSION_SPECIFIER_g:
            case CONVERSION_SPECIFIER_G:
            case CONVERSION_SPECIFIER_a:
            case CONVERSION_SPECIFIER_A:
                if (length_modifier == LENGTH_MODIFIER_L) {
                    parsed_format.conversion_specifications[i].data_long_double = argument.argument_long_double;
                } else if (length_modifier == LENGTH_MODIFIER_F) {
                    parsed_format.conversion_specifications[i].data_fp = argument.argument_fp;
                } else {
                    parsed_format.conversion_specifications[i].data_long_double = argument.argument_double;
                }
                break;
            case CONVERSION_SPECIFIER_c:
                parsed_format.conversion_specifications[i].data_char = (char) (unsigned char) argument.argument_int;
                break;
            case CONVERSION_SPECIFIER_s:
                parsed_format.conversion_specifications[i].data_char_pointer = argument.argument_char_pointer;
                break;
            case CONVERSION_SPECIFIER_p:
                parsed_format.conversion_specifications[i].data_void_pointer = argument.argument_void_pointer;
                break;
        }
        if (length_modifier != LENGTH_MODIFIER_Z && length_modifier != LENGTH_MODIFIER_F) {
            parsed_format.conversion_specifications[i].length_modifier = LENGTH_MODIFIER_none;
        }
        parsed_format.conversion_specifications[i].data_argument_index = (size_t) -1;
    }
    free(arguments);
    return parsed_format;
}
