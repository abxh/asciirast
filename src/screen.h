#pragma once

#include "color.h"
#include "math/vec.h"

#include <stdio.h>

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 40
#endif

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 20
#endif

#define ASPECT_RATIO (SCREEN_WIDTH / (2.f * SCREEN_HEIGHT))

typedef struct {
    color_type color;
    float depth;
    char ascii_char;
} pixel_data_type;

struct screen_type;

struct screen_type* screen_create(FILE* output_stream);

void screen_destroy(struct screen_type* obj_p);

void screen_clear(struct screen_type* obj_p);

void screen_refresh(const struct screen_type* obj_p);

void screen_set_pixel_data(struct screen_type* obj_p, const vec2int_type pos, const pixel_data_type data);

pixel_data_type screen_get_pixel_data(struct screen_type* obj_p, const vec2int_type pos);
