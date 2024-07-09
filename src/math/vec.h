#pragma once

#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

#include "math/float.h"

#define DIM 2
#define STRUCT_TO_UNION \
    struct {            \
        float x;        \
        float y;        \
    }
#include "math/vec_internal.h"
#undef DIM

#define DIM 3
#define STRUCT_TO_UNION \
    struct {            \
        float x;        \
        float y;        \
        float z;        \
    }
#include "math/vec_internal.h"
#undef DIM

#define DIM 4
#define STRUCT_TO_UNION \
    struct {            \
        float x;        \
        float y;        \
        float z;        \
        float w;        \
    }
#include "math/vec_internal.h"
#undef DIM

// special vector properties
// ----------------------------------------------------------------------------------------------------------

static inline float vec2_angle_fast(const vec2_type v0) {
    assert(v0.y != 0);
    return atan2f(v0.x, v0.y); // tan^-1(v0.x / v1.y);
}

static inline vec2_type vec2_rotate_origo(const vec2_type v0, const float angle_rad) {
    const float res_x = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad);
    const float res_y = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad);

    return (vec2_type){.x = res_x, .y = res_y};
}

static inline vec3_type vec3_cross(const vec3_type v0, const vec3_type v1) {
    //     | x v0.x v1.x |            | v0.y v1.y |            | v0.x v1.x |            | v0.x v1.x |
    // det(| y v0.y v1.y |) = x * det(| v0.z v1.z |) - y * det(| v0.z v1.z |) + z * det(| v0.y v1.y |)
    //     | z v0.z v1.z |
    //
    //   = x * (v0.y * v1.z - v0.z * v1.y) + y * (-v0.x * v1.z + v0.z * v1.x) + z * (v0.x * v1.y - v0.y * v1.x)

    const float res_x = v0.y * v1.z - v0.z * v1.y;
    const float res_y = -v0.x * v1.z + v0.z * v1.x;
    const float res_z = v0.x * v1.y - v0.y * v1.x;

    return (vec3_type){.x = res_x, .y = res_y, .z = res_z};
}

static inline float vec2_cross(const vec2_type v0, const vec2_type v1) {
    const float res_z = v0.x * v1.y - v0.y * v1.x;

    return res_z;
}

static inline vec3_type vec3_reflect(const vec3_type v0, const vec3_type normal) {
    assert(float_is_equal(vec3_length(normal), 1.f));

    const float p = 2.f * vec3_dot(v0, normal);

    vec3_type res;
    for (size_t i = 0; i < 3; ++i) {
        res.array[i] = v0.array[i] - p * normal.array[i];
    }
    return res;
}

static inline vec4_type vec4_cross(const vec4_type v0, const vec4_type v1) {
    const float res_x = v0.y * v1.z - v0.z * v1.y;
    const float res_y = -v0.x * v1.z + v0.z * v1.x;
    const float res_z = v0.x * v1.y - v0.y * v1.x;
    const float res_w = 1.f;

    return (vec4_type){.x = res_x, .y = res_y, .z = res_z, .w = res_w};
}

static inline vec4_type vec4_reflect(const vec4_type v0, const vec4_type normal) {
    assert(float_is_equal(vec4_length(normal), 1.f));

    const float p = 2.f * vec4_dot(v0, normal);

    vec4_type res;
    for (size_t i = 0; i < 4; ++i) {
        res.array[i] = v0.array[i] - p * normal.array[i];
    }
    return res;
}
