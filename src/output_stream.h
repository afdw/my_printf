#ifndef MY_PRINTF_OUTPUT_STREAM_H
#define MY_PRINTF_OUTPUT_STREAM_H

#define MY_PRINTF_OUTPUT_STREAM_DEFINED

struct output_stream {
    void (*put)(void *data, char c);
    void (*destroy)(void *data);
    void *data;
};

struct output_stream void_output_stream_create();

struct output_stream file_output_stream_create(int fd);

int stdout_fd();

#endif
