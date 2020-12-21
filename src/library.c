#include "library.h"

#include <stdio.h>

int my_printf(const char *format, ...) {
    return printf("%s", format);
}
