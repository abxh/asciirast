#pragma once

#include <stddef.h>

#define ASCII_MIN_PRINTABLE (32)
#define ASCII_MAX_PRINTABLE (126)

typedef struct {
    size_t size;
    char index_to_ascii[ASCII_MAX_PRINTABLE + 1];
    int ascii_to_index[ASCII_MAX_PRINTABLE + 1];
} ascii_table_type;
