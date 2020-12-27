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
