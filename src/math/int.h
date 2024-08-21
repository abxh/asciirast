#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static inline bool int_is_equal(const int i0, const int i1) {
    return i0 == i1;
}

static inline int int_min(const int i0, const int i1) {
    return i0 <= i1 ? i0 : i1;
}

static inline int int_max(const int i0, const int i1) {
    return i0 >= i1 ? i0 : i1;
}

static inline int int_clamp(const int i0, const int min, const int max) {
    return int_min(int_max(i0, min), max);
}

static inline int int_abs(const int i0) {
    return abs(i0);
}

static inline int int_signof(const int i0) {
    return (i0 > 0) - (i0 < 0);
}

static inline bool int_in_range(const int i0, const int min, const int max) {
    return min <= i0 && i0 <= max;
}

static inline float int_to_float(const int i0) {
    const float new_value = (float)i0;

    return new_value;
}
