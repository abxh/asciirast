#pragma once

#include "math/vec.h"

typedef struct {
    union {
        struct {
            float r;
            float g;
            float b;
        };
        vec3_type vec3;
    };
} color_type;

static const color_type g_color_min = {.r = 0.f - FLOAT_TOLERANCE, .g = 0.f - FLOAT_TOLERANCE, .b = 0.f - FLOAT_TOLERANCE};
static const color_type g_color_max = {.r = 1.f + FLOAT_TOLERANCE, .g = 1.f + FLOAT_TOLERANCE, .b = 1.f + FLOAT_TOLERANCE};

static const color_type g_color_white = {.r = 1.f, .g = 1.f, .b = 1.f};
static const color_type g_color_gray = {.r = 1.f / 2.f, .g = 1.f / 2.f, .b = 1.f / 2.f};
static const color_type g_color_black = {.r = 0.f, .g = 0.f, .b = 0.f};

static const color_type g_color_red = {.r = 1.f, .g = 0.f, .b = 0.f};
static const color_type g_color_green = {.r = 0.f, .g = 1.f, .b = 0.f};
static const color_type g_color_blue = {.r = 0.f, .g = 0.f, .b = 1.f};

static const color_type g_color_yellow = {.r = 1.f, .g = 1.f, .b = 0.f};
static const color_type g_color_magenta = {.r = 1.f, .g = 0.f, .b = 1.f};
static const color_type g_color_cyan = {.r = 0.f, .g = 1.f, .b = 1.f};

static inline color_type color_add(const color_type c0, const color_type c1) {
    return (color_type){.vec3 = vec3_add(c0.vec3, c1.vec3)};
}

static inline color_type color_sub(const color_type c0, const color_type c1) {
    return (color_type){.vec3 = vec3_sub(c0.vec3, c1.vec3)};
}

static inline color_type color_scale(const color_type c0, const float t) {
    return (color_type){.vec3 = vec3_scale(c0.vec3, t)};
}

static inline color_type color_lerp(const color_type c0, const color_type c1, const float t) {
    return (color_type){.vec3 = vec3_lerp(c0.vec3, c1.vec3, t)};
}

static inline bool color_in_range(const color_type c0, const color_type min, const color_type max) {
    return vec3_in_range(c0.vec3, min.vec3, max.vec3);
}
