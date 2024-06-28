#pragma once

#include "external.h"

#include "color.h"

#ifndef FRAMEBUF_WIDTH
#define FRAMEBUF_WIDTH 40
#endif

#ifndef FRAMEBUF_HEIGHT
#define FRAMEBUF_HEIGHT 20
#endif

#define ASPECT_RATIO (2.f * (float)FRAMEBUF_WIDTH / (float)FRAMEBUF_HEIGHT)

typedef struct {
    color_type color;
    float depth;
    char ascii_char;
} pixel_data_type;

void screen_init(FILE* output_stream);

void screen_deinit(void);

void screen_refresh(void);

void screen_set_pixel_data(const vec2int_type framebuf_pos, const pixel_data_type data);

pixel_data_type screen_get_pixel_data(const vec2int_type framebuf_pos);
