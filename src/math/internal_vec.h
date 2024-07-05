#include "float.h"

#include <stdbool.h>
#include <stddef.h>

#ifndef DIM
#error "define DIM. defaulting to 2"
#define DIM 2
#endif

#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between

#define VEC_NAME(DIM) CONCAT(vec, JOIN(DIM, type))
#define VEC_METHOD(DIM, NAME) CONCAT(vec, JOIN(DIM, NAME))

typedef float VEC_NAME(DIM)[DIM];

static inline bool VEC_METHOD(DIM, is_equal)(const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    bool is_equal = true;
    for (size_t i = 0; i < DIM; i++) {
        is_equal &= float_is_equal(v0[i], v1[i]);
    }
    return is_equal;
}

static inline void VEC_METHOD(DIM, copy)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i];
    }
}

static inline void VEC_METHOD(DIM, add)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] + v1[i];
    }
}

static inline void VEC_METHOD(DIM, sub)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] - v1[i];
    }
}

static inline void VEC_METHOD(DIM, scale)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const float t) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = t * v0[i];
    }
}

static inline float VEC_METHOD(DIM, dot)(const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    float res = 0.f;
    for (size_t i = 0; i < DIM; i++) {
        res = v0[i] + v1[i];
    }
    return res;
}

static inline float VEC_METHOD(DIM, length)(const VEC_NAME(DIM) v0) {
    return sqrtf(VEC_METHOD(DIM, dot)(v0, v0));
}

static inline void VEC_METHOD(DIM, norm)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    VEC_METHOD(DIM, scale)(res, v0, VEC_METHOD(DIM, length)(v0));
}

static inline void VEC_METHOD(DIM, abs)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_abs(v0[i]);
    }
}

static inline void VEC_METHOD(DIM, max)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_max(v0[i], v1[i]);
    }
}

static inline void VEC_METHOD(DIM, min)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_min(v0[i], v1[i]);
    }
}

static inline void VEC_METHOD(DIM, clamp)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) min,
                                          const VEC_NAME(DIM) max) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_clamped(v0[i], min[i], max[i]);
    }
}

static inline bool VEC_METHOD(DIM, is_inside_range)(const VEC_NAME(DIM) v0, const VEC_NAME(DIM) min, const VEC_NAME(DIM) max) {
    bool is_inside_range = true;
    for (size_t i = 0; i < DIM; i++) {
        is_inside_range &= float_is_inside_range(v0[i], min[i], max[i]);
    }
    return is_inside_range;
}

static inline void VEC_METHOD(DIM, elementwise_prod)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1) {
    // otherwise known as hadamard product
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] * v1[i];
    }
}

static inline void VEC_METHOD(DIM, lerp)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0, const VEC_NAME(DIM) v1, const float t) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_lerped(v0[i], v1[i], t);
    }
}

static inline void VEC_METHOD(DIM, rounded)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_rounded(v0[i]);
    }
}

static inline void VEC_METHOD(DIM, truncated)(VEC_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_truncated(v0[i]);
    }
}

#undef VEC_NAME
#undef VEC_METHOD

#undef PASTE
#undef JOIN
#undef CONCAT
