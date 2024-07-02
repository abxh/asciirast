#pragma once

#include <stdbool.h>
#include <stdlib.h>

static inline bool int_is_equal(const int v0, const int v1) {
    return v0 == v1;
}

static inline int int_abs(const int v0) {
    return abs(v0);
}

static inline int int_min(const int v0, const int v1) {
    return v0 < v1 ? v0 : v1;
}

static inline int int_max(const int v0, const int v1) {
    return v0 > v1 ? v0 : v1;
}

static inline int int_clamp(const int v0, const int min, const int max) {
    return int_min(int_max(v0, min), max);
}

static inline bool int_is_inside_range(const int v0, const int min, const int max) {
    return min <= v0 && v0 <= max;
}

static inline int int_lerped(const int v0, const int v1, const float t0) {
    return v0 + (int)(t0 * (float)(v1 - v0));
}

static inline float int_to_float(const int v0) {
    return (float)v0;
}

