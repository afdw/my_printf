#ifndef MY_PRINTF_LIBRARY_H
#define MY_PRINTF_LIBRARY_H

#include <stdarg.h>

int my_vprintf(const char *format, va_list va_list);

int my_printf(const char *format, ...);

#endif
