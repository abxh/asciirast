#pragma once

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#ifndef FLOAT_TOLERANCE
#define FLOAT_TOLERANCE 0.00001f
#endif

static inline bool float_is_zero(const float f0) {
    return f0 <= FLOAT_TOLERANCE;
}

static inline bool float_is_equal(const float f0, const float f1) {
    return fabsf(f0 - f1) <= FLOAT_TOLERANCE;
}

static inline float float_min(const float f0, const float f1) {
    return fminf(f0, f1);
}

static inline float float_max(const float f0, const float f1) {
    return fmaxf(f0, f1);
}

static inline float float_clamp(const float f0, const float min, const float max) {
    return float_min(float_max(f0, min), max);
}

static inline bool float_in_range(const float f0, const float min, const float max) {
    return min <= f0 && f0 <= max;
}

static inline float float_lerp(const float f0, const float f1, const float t) {
    const float f0_weight = (1.f - t) * f0;
    const float f1_weight = t * f1;

    return f0_weight + f1_weight;
}

static inline float float_signof(const float f0) {
    return (float)((f0 > 0) - (f0 < 0));
}

static inline float float_abs(const float f0) {
    return fabsf(f0);
}

static inline float float_ceil(const float f0) {
    return ceilf(f0);
}

static inline float float_floor(const float f0) {
    return floorf(f0);
}

static inline float float_round(const float f0) {
    return roundf(f0);
}

static inline int float_to_int_floor(const float f0) {
    const int new_value = (int)f0;

    return new_value;
}

static inline int float_to_int_ceil(const float f0) {
    const float rounded_value_f = ceilf(f0);
    const int new_value = (int)rounded_value_f;

    return new_value;
}

static inline int float_to_int_round(const float f0) {
    const float rounded_value_f = roundf(f0);
    const int new_value = (int)rounded_value_f;

    return new_value;
}
