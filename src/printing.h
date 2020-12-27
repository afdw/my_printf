#ifndef MY_PRINTF_PRINTING_H
#define MY_PRINTF_PRINTING_H

#include "parsing.h"
#include "output_stream.h"

size_t print_parsed_format(struct output_stream output_stream, struct parsed_format parsed_format);

#endif
