#pragma once

#include "math/f32.h"
#include "math/i32.h"

#include <assert.h>
#include <stdbool.h>

static inline i32 f32_to_i32_floor(const f32 f0) {
    const int new_value = (int)f0;

    return I32(new_value);
}

static inline i32 f32_to_i32_ceil(const f32 f0) {
    const float rounded_value_f = ceilf(f0);
    const int new_value = (int)rounded_value_f;

    return I32(new_value);
}

static inline i32 f32_to_i32_round(const f32 f0) {
    const float rounded_value_f = roundf(f0);
    const int new_value = (int)rounded_value_f;

    return I32(new_value);
}
