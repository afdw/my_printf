#include "output_stream.h"

#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>

void void_output_stream_put(void *data, char c) {
}

void void_output_stream_destroy(void *data) {
}

struct output_stream void_output_stream_create() {
    return (struct output_stream) {
        .put = &void_output_stream_put,
        .destroy = &void_output_stream_destroy,
        .data = NULL,
    };
}

struct string_output_stream_data {
    char *string;
    bool limited;
    size_t limit;
    size_t used;
};

void string_output_stream_put(void *data, char c) {
    struct string_output_stream_data *string_output_stream_data = (struct string_output_stream_data *) data;
    if (!string_output_stream_data->limited || string_output_stream_data->used < string_output_stream_data->limit) {
        string_output_stream_data->string[string_output_stream_data->used] = c;
        string_output_stream_data->used++;
    }
}

void string_output_stream_destroy(void *data) {
    string_output_stream_put(data, '\0');
    free(data);
}

struct output_stream string_output_stream_create(char *string, bool limited, size_t limit) {
    struct string_output_stream_data *string_output_stream_data = malloc(sizeof(struct string_output_stream_data));
    *string_output_stream_data = (struct string_output_stream_data) {
        .string = string,
        .limited = limited,
        .limit = limit,
        .used = 0,
    };
    return (struct output_stream) {
        .put = &string_output_stream_put,
        .destroy = &string_output_stream_destroy,
        .data = (void *) string_output_stream_data,
    };
}

struct file_output_stream_data {
    int fd;
};

void file_output_stream_put(void *data, char c) {
    struct file_output_stream_data *file_output_stream_data = (struct file_output_stream_data *) data;
    write(file_output_stream_data->fd, (const void *) &c, 1);
}

void file_output_stream_destroy(void *data) {
    free(data);
}

struct output_stream file_output_stream_create(int fd) {
    struct file_output_stream_data *file_output_stream_data = malloc(sizeof(struct file_output_stream_data));
    *file_output_stream_data = (struct file_output_stream_data) {
        .fd = fd,
    };
    return (struct output_stream) {
        .put = &file_output_stream_put,
        .destroy = &file_output_stream_destroy,
        .data = (void *) file_output_stream_data,
    };
}

int stdout_fd() {
    return STDOUT_FILENO;
}
