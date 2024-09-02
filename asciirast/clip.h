#pragma once

#include <stdbool.h>

struct AABB_type {
    float xmin;
    float ymin;
    float xmax;
    float ymax;
};

bool clip_line_cohen_sutherland(const struct AABB_type aabb, float* x0_ptr, float* y0_ptr, float* x1_ptr, float* y1_ptr);
