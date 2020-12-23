#include "library.h"

#include <stdio.h>
#include "parsing.h"

int my_printf(const char *format, ...) {
    struct parsed_format parsed_format = parse(format);
    parsed_format_destroy(parsed_format);
    return printf("%s", format);
}
