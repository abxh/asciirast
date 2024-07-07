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

static const color_type g_color_min = {.r = 0.f, .g = 0.f, .b = 0.f};
static const color_type g_color_max = {.r = 1.f, .g = 1.f, .b = 1.f};

static const color_type g_color_white = {.r = 1.f, .g = 1.f, .b = 1.f};
static const color_type g_color_gray = {.r = 1.f / 2.f, .g = 1.f / 2.f, .b = 1.f / 2.f};
static const color_type g_color_black = {.r = 0.f, .g = 0.f, .b = 0.f};

static const color_type g_color_red = {.r = 1.f, .g = 0.f, .b = 0.f};
static const color_type g_color_green = {.r = 0.f, .g = 1.f, .b = 0.f};
static const color_type g_color_blue = {.r = 0.f, .g = 0.f, .b = 1.f};

static const color_type g_color_yellow = {.r = 1.f, .g = 1.f, .b = 0.f};
static const color_type g_color_magenta = {.r = 1.f, .g = 0.f, .b = 1.f};
static const color_type g_color_cyan = {.r = 0.f, .g = 1.f, .b = 1.f};
