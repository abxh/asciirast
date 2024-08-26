#include <assert.h>

#include "color.h"

uint32_t color_encode_rgb(const uint8_t r, const uint8_t g, const uint8_t b)
{
    return (r & 255) | ((g & 255) << 8) | ((b & 255) << 16);
}

struct rgb_type color_decode_rgb(const uint32_t rgb)
{
    return (struct rgb_type){.r = (uint8_t)(rgb & 255), .g = (uint8_t)((rgb >> 8) & 255), .b = (uint8_t)((rgb >> 16) & 255)};
}

struct rgb_type color_decode_rgb_unchecked(const uint32_t rgb)
{
    assert((rgb & (sizeof(uint8_t) - 1)) <= 255);
    assert(((rgb >> 8) & (sizeof(uint8_t) - 1)) <= 255);
    assert(((rgb >> 16) & (sizeof(uint8_t) - 1)) <= 255);

    return (struct rgb_type){.r = (uint8_t)rgb, .g = (uint8_t)(rgb >> 8), .b = (uint8_t)(rgb >> 16)};
}
