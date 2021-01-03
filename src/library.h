#ifndef MY_PRINTF_LIBRARY_H
#define MY_PRINTF_LIBRARY_H

#include <stdarg.h>

int my_printf(const char *format, ...);

#ifdef MY_PRINTF_OUTPUT_STREAM_DEFINED
int my_fprintf(struct output_stream output_stream, const char *format, ...);
#endif

int my_dprintf(int fd, const char *format, ...);

int my_sprintf(char *string, const char *format, ...);

int my_snprintf(char *string, size_t limit, const char *format, ...);

int my_vprintf(const char *format, va_list va_list);

#ifdef MY_PRINTF_OUTPUT_STREAM_DEFINED
int my_vfprintf(struct output_stream output_stream, const char *format, va_list va_list);
#endif

int my_vdprintf(int fd, const char *format, va_list va_list);

int my_vsprintf(char *string, const char *format, va_list va_list);

int my_vsnprintf(char *string, size_t limit, const char *format, va_list va_list);

#endif
