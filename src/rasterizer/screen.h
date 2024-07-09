#pragma once

#include "math/vec.h"

#include "rasterizer/color.h"

#include <stdio.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 40
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 20
#endif

// since ascii characters are almost twice as high as they are wide,
// normal screen ratio factors it in like so:
#define ASPECT_RATIO (SCREEN_WIDTH / (2.f * SCREEN_HEIGHT))

typedef struct {
    color_type color;
    float depth;
    char ascii_char;
} pixel_data_type;

typedef struct screen_type {
    char framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];
    float depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];
    color_type colorbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

    FILE* output_stream;
} screen_type;

struct screen_type* screen_create(FILE* output_stream);

void screen_destroy(struct screen_type* obj_p);

void screen_clear(struct screen_type* obj_p);

void screen_refresh(struct screen_type* obj_p);

void screen_set_pixel_data(struct screen_type* obj_p, const vec2_type pos, const pixel_data_type data);

pixel_data_type screen_get_pixel_data(struct screen_type* obj_p, const vec2_type pos);
