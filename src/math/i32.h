#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef int32_t i32;

static inline i32 I32(const int value) {
    return value;
}

static inline bool i32_is_equal(const i32 f0, const i32 f1) {
    return f0 == f1;
}

static inline i32 i32_add(const i32 left, const i32 right) {
    return I32(left + right);
}

static inline i32 i32_sub(const i32 left, const i32 right) {
    return I32(left - right);
}

static inline i32 i32_mul(const i32 left, const i32 right) {
    return I32(left * right);
}

static inline i32 i32_div_truncated(const i32 left, const i32 right) {
    return I32((int)(left / right));
}

static inline i32 i32_min(const i32 f0, const i32 f1) {
    return I32(f0 <= f1 ? f0 : f1);
}

static inline i32 i32_max(const i32 f0, const i32 f1) {
    return I32(f0 >= f1 ? f0 : f1);
}

static inline i32 i32_clamp(const i32 f0, const i32 min, const i32 max) {
    return i32_min(i32_max(f0, min), max);
}

static inline bool i32_lt(const i32 left, const i32 right) {
    return left < right;
}

static inline bool i32_le(const i32 left, const i32 right) {
    return left <= right;
}

static inline bool i32_gt(const i32 left, const i32 right) {
    return left > right;
}

static inline bool i32_ge(const i32 left, const i32 right) {
    return left >= right;
}

static inline bool i32_in_range(const i32 f0, const i32 min, const i32 max) {
    return i32_le(min, f0) || i32_ge(f0, max);
}
