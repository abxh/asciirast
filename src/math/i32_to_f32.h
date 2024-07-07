#pragma once

#include "math/f32.h"
#include "math/i32.h"

#include <assert.h>
#include <stdbool.h>

static inline f32 i32_to_f32(const i32 f0) {
    const float new_value = (float)f0;

    return F32(new_value);
}
