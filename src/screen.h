#pragma once

#include "stdbool.h"
#include "transform.h"

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 20
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 40
#endif

static const float aspect_ratio = 2 * SCREEN_HEIGHT / (float)(SCREEN_WIDTH);

void screen_init(void);

void screen_deinit(void);

void screen_clear(void);

void screen_refresh(void);

void draw_framebuf_point_w_no_bounds_checking(int framebuf_x, int framebuf_y, char c);

void draw_framebuf_point_w_bounds_checking(int framebuf_x, int framebuf_y, char c);

// helper functions for frame buffer:
// -----------------------------------------------------------------------------

int to_framebuf_x(float screen_x);

int to_framebuf_y(float screen_y);

bool inside_framebuf(int framebuf_x, int framebuf_y);

typedef struct {
    int x;
    int y;
} framebuf_coords;

framebuf_coords to_framebuf_coords(vec2 v);

void swap_framebuf_coords(framebuf_coords* v1_ptr, framebuf_coords* v2_ptr);
