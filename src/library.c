#include "output_stream.h"

#include "library.h"

#include "varargs_loading.h"
#include "printing.h"

int my_printf(const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vprintf(format, va_list);
    va_end(va_list);
    return result;
}

int my_fprintf(struct output_stream output_stream, const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vfprintf(output_stream, format, va_list);
    va_end(va_list);
    return result;
}

int my_dprintf(int fd, const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vdprintf(fd, format, va_list);
    va_end(va_list);
    return result;
}

int my_sprintf(char *string, const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vsprintf(string, format, va_list);
    va_end(va_list);
    return result;
}

int my_snprintf(char *string, size_t limit, const char *format, ...) {
    va_list va_list;
    va_start(va_list, format);
    int result = my_vsnprintf(string, limit, format, va_list);
    va_end(va_list);
    return result;
}

int my_vprintf(const char *format, va_list va_list) {
    return my_vdprintf(stdout_fd(), format, va_list);
}

int my_vfprintf(struct output_stream output_stream, const char *format, va_list va_list) {
    struct parsed_format parsed_format = load_varargs(format, va_list);
    size_t printed = print_parsed_format(output_stream, parsed_format);
    parsed_format_destroy(parsed_format);
    return printed;
}

int my_vdprintf(int fd, const char *format, va_list va_list) {
    struct output_stream output_stream = file_output_stream_create(fd);
    size_t printed = my_vfprintf(output_stream, format, va_list);
    output_stream.destroy(output_stream.data);
    return printed;
}

int my_vsprintf(char *string, const char *format, va_list va_list) {
    struct output_stream output_stream = string_output_stream_create(string, false, 0);
    size_t printed = my_vfprintf(output_stream, format, va_list);
    output_stream.destroy(output_stream.data);
    return printed;
}

int my_vsnprintf(char *string, size_t limit, const char *format, va_list va_list) {
    struct output_stream output_stream = string_output_stream_create(string, true, limit);
    size_t printed = my_vfprintf(output_stream, format, va_list);
    output_stream.destroy(output_stream.data);
    return printed;
}
