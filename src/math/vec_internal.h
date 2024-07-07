#include "math/f32.h"
#include "math/macros.h"

#include <stdbool.h>
#include <stddef.h>

#ifndef DIM
#error "define DIM. defaulting to 2"
#define DIM 2
#endif

#ifndef STRUCT_TO_UNION
#error "define STRUCT_TO_UNION. defaulting to struct { f32_type x; f32_type y; }"
#define STRUCT_TO_UNION \
    struct {            \
        f32 x;          \
        f32 y;          \
    }
#endif

#define VEC_TYPE CONCAT(vec, JOIN(DIM, type))
#define VEC_METHOD(NAME) CONCAT(vec, JOIN(DIM, NAME))

typedef struct {
    union {
        f32 array[DIM];
        STRUCT_TO_UNION;
    };
} VEC_TYPE;

static inline bool VEC_METHOD(is_equal)(const VEC_TYPE v0, const VEC_TYPE v1) {
    bool is_equal = true;
    for (size_t i = 0; i < DIM; i++) {
        is_equal &= f32_is_equal(v0.array[i], v1.array[i]);
    }
    return is_equal;
}

static inline VEC_TYPE VEC_METHOD(copy)(const VEC_TYPE v0) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = v0.array[i];
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(add)(const VEC_TYPE v0, const VEC_TYPE v1) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = v0.array[i] + v1.array[i];
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(sub)(const VEC_TYPE v0, const VEC_TYPE v1) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = v0.array[i] - v1.array[i];
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(scale)(const VEC_TYPE v0, const float t) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = t * v0.array[i];
    }
    return res;
}

static inline float VEC_METHOD(dot)(const VEC_TYPE v0, const VEC_TYPE v1) {
    float res = 0.f;
    for (size_t i = 0; i < DIM; i++) {
        res = v0.array[i] + v1.array[i];
    }
    return res;
}

static inline float VEC_METHOD(length)(const VEC_TYPE v0) {
    return sqrtf(VEC_METHOD(dot)(v0, v0));
}

static inline VEC_TYPE VEC_METHOD(norm)(const VEC_TYPE v0) {
    return VEC_METHOD(scale)(v0, VEC_METHOD(length)(v0));
}

static inline VEC_TYPE VEC_METHOD(max)(const VEC_TYPE v0, const VEC_TYPE v1) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_max(v0.array[i], v1.array[i]);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(min)(const VEC_TYPE v0, const VEC_TYPE v1) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_min(v0.array[i], v1.array[i]);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(clamp)(const VEC_TYPE v0, const VEC_TYPE min, const VEC_TYPE max) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_clamp(v0.array[i], min.array[i], max.array[i]);
    }
    return res;
}

static inline bool VEC_METHOD(in_range)(const VEC_TYPE v0, const VEC_TYPE min, const VEC_TYPE max) {
    bool is_inside_range = true;
    for (size_t i = 0; i < DIM; i++) {
        is_inside_range &= f32_in_range(v0.array[i], min.array[i], max.array[i]);
    }
    return is_inside_range;
}

static inline VEC_TYPE VEC_METHOD(elementwise_prod)(const VEC_TYPE v0, const VEC_TYPE v1) {
    VEC_TYPE res;
    // otherwise known as hadamard product
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = v0.array[i] * v1.array[i];
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(lerp)(const VEC_TYPE v0, const VEC_TYPE v1, const float t) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_lerp(v0.array[i], v1.array[i], t);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(ceil)(const VEC_TYPE v0) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_ceil(v0.array[i]);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(floor)(const VEC_TYPE v0) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_floor(v0.array[i]);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(round)(const VEC_TYPE v0) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = f32_round(v0.array[i]);
    }
    return res;
}

static inline VEC_TYPE VEC_METHOD(from_array)(const float arr[DIM]) {
    VEC_TYPE res;
    for (size_t i = 0; i < DIM; i++) {
        res.array[i] = arr[i];
    }
    return res;
}

static inline void VEC_METHOD(to_array)(float res[DIM], const VEC_TYPE v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0.array[i];
    }
}

#undef DIM
#undef STRUCT_TO_UNION
#undef VEC
#undef VEC_TYPE
#undef VEC_METHOD
