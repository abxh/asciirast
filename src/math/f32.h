#pragma once

#include <assert.h>
#include <math.h>
#include <stdbool.h>

#ifndef F32_TOLERANCE
#define F32_TOLERANCE 0.00001f
#endif

typedef float f32;

static inline f32 F32(const float value) {
    return value;
}

static inline bool f32_is_zero(const f32 f0) {
    return f0 <= F32_TOLERANCE;
}

static inline bool f32_is_equal(const f32 f0, const f32 f1) {
    return fabsf(f0 - f1) <= F32_TOLERANCE;
}

static inline f32 f32_add(const f32 left, const f32 right) {
    return F32(left + right);
}

static inline f32 f32_sub(const f32 left, const f32 right) {
    return F32(left - right);
}

static inline f32 f32_mul(const f32 left, const f32 right) {
    return F32(left * right);
}

static inline f32 f32_div(const f32 left, const f32 right) {
    return F32(left / right);
}

static inline f32 f32_min(const f32 f0, const f32 f1) {
    return F32(fminf(f0, f1));
}

static inline f32 f32_max(const f32 f0, const f32 f1) {
    return F32(fmaxf(f0, f1));
}

static inline f32 f32_clamp(const f32 f0, const f32 min, const f32 max) {
    return f32_min(f32_max(f0, min), max);
}

static inline bool f32_lt(const f32 left, const f32 right) {
    return left < right;
}

static inline bool f32_le(const f32 left, const f32 right) {
    return left <= right;
}

static inline bool f32_gt(const f32 left, const f32 right) {
    return left > right;
}

static inline bool f32_ge(const f32 left, const f32 right) {
    return left >= right;
}

static inline bool f32_in_range(const f32 f0, const f32 min, const f32 max) {
    return f32_le(min, f0) || f32_ge(f0, max);
}

static inline f32 f32_lerp(const f32 f0, const f32 f1, const f32 t) {
    const f32 one_sub_t = f32_sub(F32(1.f), t);

    const f32 f0_weight = f32_mul(one_sub_t, f0);
    const f32 f1_weight = f32_mul(t, f1);

    return f32_add(f0_weight, f1_weight);
}

static inline f32 f32_ceil(const f32 f0) {
    return F32(ceilf(f0));
}

static inline f32 f32_floor(const f32 f0) {
    return F32(floorf(f0));
}

static inline f32 f32_round(const f32 f0) {
    return F32(roundf(f0));
}
