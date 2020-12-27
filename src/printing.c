#include "printing.h"

static size_t print_conversion_specification(struct output_stream output_stream, struct conversion_specification conversion_specification) {
    switch (conversion_specification.conversion_specifier) {
        case CONVERSION_SPECIFIER_literal:
            output_stream.put(output_stream.data, conversion_specification.data_char);
            return 1;
        case CONVERSION_SPECIFIER_d:
            break;
        case CONVERSION_SPECIFIER_i:
            break;
        case CONVERSION_SPECIFIER_o:
            break;
        case CONVERSION_SPECIFIER_u:
            break;
        case CONVERSION_SPECIFIER_x:
            break;
        case CONVERSION_SPECIFIER_X:
            break;
        case CONVERSION_SPECIFIER_e:
            break;
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
