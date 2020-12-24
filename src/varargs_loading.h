#ifndef MY_PRINTF_VARARGS_LOADING_H
#define MY_PRINTF_VARARGS_LOADING_H

#include <stdarg.h>
#include "parsing.h"

struct parsed_format load_varargs(const char *format, va_list va_list);

#endif
