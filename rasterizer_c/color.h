#include <stdint.h>

struct rgb_type {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

uint32_t color_encode_rgb(const uint8_t r, const uint8_t g, const uint8_t b);

struct rgb_type color_decode_rgb(const uint32_t rgb);

struct rgb_type color_decode_rgb_unchecked(const uint32_t rgb);
