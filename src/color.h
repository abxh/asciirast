#pragma once

#include "transform.h"

typedef struct {
    float r;
    float g;
    float b;
} color_type;

#ifndef COLOR_INTENSITY
#define COLOR_INTENSITY 0.9f
#endif

static const float CLRI = COLOR_INTENSITY;

static const color_type color_white = {.r = CLRI, .g = CLRI, .b = CLRI};
static const color_type color_gray = {.r = CLRI / 2.f, .g = CLRI / 2.f, .b = CLRI / 2.f};
static const color_type color_black = {.r = 0.f, .g = 0.f, .b = 0.f};

static const color_type color_red = {.r = CLRI, .g = 0.f, .b = 0.f};
static const color_type color_green = {.r = 0.f, .g = CLRI, .b = 0.f};
static const color_type color_blue = {.r = 0.f, .g = 0.f, .b = CLRI};

static const color_type color_yellow = {.r = CLRI, .g = CLRI, .b = 0.f};
static const color_type color_magenta = {.r = CLRI, .g = 0.f, .b = CLRI};
static const color_type color_cyan = {.r = 0.f, .g = CLRI, .b = CLRI};

static inline vec3_type from_color_to_vec3(const color_type c0) {
    return (vec3_type){.x = c0.r, .y = c0.g, .z = c0.b};
}

static inline color_type from_vec3_to_color(const vec3_type v0) {
    return (color_type){.r = v0.x, .g = v0.y, .b = v0.z};
}

static inline bool is_equal_color(const color_type c0, const color_type c1) {
    return is_equal_vec3(from_color_to_vec3(c0), from_color_to_vec3(c1));
}

static inline bool inside_range_color(const color_type c0, const color_type min, const color_type max) {
    return inside_range_vec3(from_color_to_vec3(c0), from_color_to_vec3(min), from_color_to_vec3(max));
}

static inline color_type sum_color(const color_type c0, const color_type c1) {
    return from_vec3_to_color(sum_vec3(from_color_to_vec3(c0), from_color_to_vec3(c1)));
}

static inline color_type scaled_color(const color_type c0, const float t) {
    return from_vec3_to_color(scaled_vec3(from_color_to_vec3(c0), t));
}

static inline color_type lerp_color(const color_type c0, const color_type c1, const float t) {
    return from_vec3_to_color(lerp_vec3(from_color_to_vec3(c0), from_color_to_vec3(c1), t));
}

static inline color_type clamp_color(const color_type c0, const color_type min, const color_type max) {
    const float r = clamp_float(c0.r, min.r, max.r);
    const float g = clamp_float(c0.g, min.g, max.g);
    const float b = clamp_float(c0.b, min.b, max.b);

    return (color_type){.r = r, .g = g, .b = b};
}
