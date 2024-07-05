#pragma once

#include <math.h>

#include <stdbool.h>

#define PI 3.1415927f
#define DEG_TO_ANGLE_RAD(expr) ((expr) * 180.f / PI)
#define RAD_TO_ANGLE_DEG(expr) ((expr) * PI / 180.f)

#define FLOAT_TOLERANCE 0.000001f

static inline bool float_is_equal(const float v0, const float v1) {
    return fabsf(v0 - v1) <= FLOAT_TOLERANCE;
}

static inline float float_abs(const float v0) {
    return fabsf(v0);
}

static inline float float_min(const float v0, const float v1) {
    return fminf(v0, v1);
}

static inline float float_max(const float v0, const float v1) {
    return fmaxf(v0, v1);
}

static inline float float_clamped(const float v0, const float min, const float max) {
    return float_min(float_max(v0, min), max);
}

static inline bool float_is_inside_range(const float v0, const float min, const float max) {
    return min <= v0 && v0 <= max;
}

static inline float float_lerped(const float v0, const float v1, const float t) {
    return (1 - t) * v0 + t * v1;
}

static inline int float_truncated_to_int(const float v0) {
    return (int)v0;
}

static inline int float_rounded_to_int(const float v0) {
    const float r = roundf(v0);
    return (int)r;
}

static inline float float_truncated(const float v0) {
    return truncf(v0);
}

static inline float float_rounded(const float v0) {
    return roundf(v0);
}
