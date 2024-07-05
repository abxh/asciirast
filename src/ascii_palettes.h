#pragma once

#include <stddef.h>

// Paul Bourke's palettes. Taken from:
// https://www.astoundingscripts.com/art/create-your-own-ascii-art-palettes-densitysort/

typedef struct {
    size_t ascii_palette_size;
    char index_to_ascii[128];
    int ascii_to_index[128];
} ascii_index_conversion_table;

static const char ascii_short_palette[] = "@%#*+=-:. ";

static const char ascii_long_palette[] = "$@B%8&WM#*oahkbdpqwmZO0QLCJUYXzcvunxrjft/\\|()1{}[]?-_+~<>i!lI;:,\"^`'. ";
