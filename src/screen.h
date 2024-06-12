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

void plot_point(int x0, int y0, char c);

void plot_point_w_depth(int x0, int y0, char c, float depth);

void plot_point_w_depth_unchecked_bounds(int x0, int y0, char c, float depth);

void plot_point_vec2int(vec2int v0, char c);

void plot_point_vec2int_w_depth(vec2int v0, char c, float depth);

void plot_point_vec2int_w_depth_unchecked_bounds(vec2int v0, char c, float depth);

int to_framebuf_x(float screen_x);

int to_framebuf_y(float screen_y);

vec2int to_framebuf_coords(vec2 v);

bool point_inside_framebuf(vec2int v);
