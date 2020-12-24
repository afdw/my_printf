#include "library.h"

#include <stdio.h>
#include "varargs_loading.h"

int my_vprintf(const char *format, va_list va_list) {
    struct parsed_format parsed_format = load_varargs(format, va_list);
    parsed_format_destroy(parsed_format);
    return printf("%s", format);
}

int my_printf(const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vprintf(format, va_list);
    va_end(va_list);
    return result;
}
