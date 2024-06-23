#pragma once

#include <stdint.h>
#include <stdio.h>

#include "transform.h"

typedef struct {
    float r;
    float g;
    float b;
} color;

typedef struct {
    char r[4];
    char g[4];
    char b[4];
} colorint_str;

static color color_white = {.r = 1.f, .g = 1.f, .b = 1.f};
static color color_grey = {.r = 0.5f, .g = 0.5f, .b = 0.5f};
static color color_black = {.r = 0.f, .g = 0.f, .b = 0.f};

static color color_red = {.r = 1.f, .g = 0.f, .b = 0.f};
static color color_green = {.r = 0.f, .g = 1.f, .b = 0.f};
static color color_blue = {.r = 0.f, .g = 0.f, .b = 1.f};

static color color_yellow = {.r = 1.f, .g = 1.f, .b = 0.f};
static color color_magenta = {.r = 1.f, .g = 0.f, .b = 1.f};
static color color_cyan = {.r = 0.f, .g = 1.f, .b = 1.f};

static inline color sum_color(color a, color b) {
    return (color){.r = a.r + b.r, .g = a.g + b.g, .b = a.b + b.b};
}

static inline color scaled_color(color c, float scalar) {
    return (color){.r = scalar * c.r, .g = scalar * c.g, .b = scalar * c.b};
}

static inline color src_to_dest_color(color src, color dest) {
    return (color){.r = dest.r - src.r, .g = dest.g - src.g, .b = dest.b - src.b};
}

static inline color lerp_color(color c0, color c1, float t) {
    return (color){.r = lerp_float(c0.r, c1.r, t), .g = lerp_float(c0.g, c1.g, t), .b = lerp_float(c0.b, c1.b, t)};
}

static inline colorint_str to_colorint_str(color c) {
    int r_int = (int)(255.999f * c.r);
    int g_int = (int)(255.999f * c.g);
    int b_int = (int)(255.999f * c.b);

    colorint_str c_int;
    snprintf(c_int.r, sizeof c_int.r, "%03d", r_int);
    snprintf(c_int.g, sizeof c_int.g, "%03d", g_int);
    snprintf(c_int.b, sizeof c_int.b, "%03d", b_int);

    return c_int;
}
