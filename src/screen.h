#pragma once

#include <stdbool.h>

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

void plot_point(int framebuf_x, int framebuf_y, char c);

void plot_point_w_depth(int framebuf_x, int framebuf_y, char c, float depth);

void plot_point_w_depth_fast_unchecked(int framebuf_x, int framebuf_y, char c, float depth);

// framebuffer and depthbuffer helper functions:
// ----------------------------------------------------------------------------

int to_framebuf_x(float screen_x);

int to_framebuf_y(float screen_y);

vec2int to_framebuf_coords(vec2 v);

bool inside_framebuf(int framebuf_x, int framebuf_y);
