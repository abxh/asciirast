#pragma once

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

#define DIM 2
#include "internal_vec.h"
#include "internal_vecint.h"
#include "internal_vecint_vec.h"
#undef DIM

#define DIM 3
#include "internal_vec.h"
#include "internal_vecint.h"
#include "internal_vecint_vec.h"
#undef DIM

#define DIM 4
#include "internal_vec.h"
#include "internal_vecint.h"
#include "internal_vecint_vec.h"
#undef DIM

// special vector properties
// ----------------------------------------------------------------------------------------------------------

static inline void vec2_rotate_origo(vec2_type res, const vec2_type v0, const float angle_rad) {
    const float x = v0[0];
    const float y = v0[1];

    res[0] = x * cosf(angle_rad) - y * sinf(angle_rad);
    res[1] = x * sinf(angle_rad) + y * cosf(angle_rad);
}

static inline void vec3_cross(vec3_type res, const vec3_type v0, const vec3_type v1) {
    //     | x v0.x v1.x |            | v0.y v1.y |            | v0.x v1.x |            | v0.x v1.x |
    // det(| y v0.y v1.y |) = x * det(| v0.z v1.z |) - y * det(| v0.z v1.z |) + z * det(| v0.y v1.y |)
    //     | z v0.z v1.z |
    //
    //   = x * (v0.y * v1.z - v0.z * v1.y) + y * (-v0.x * v1.z + v0.z * v1.x) + z * (v0.x * v1.y - v0.y * v1.x)

    const float v0x = v0[0];
    const float v1x = v1[0];

    const float v0y = v0[1];
    const float v1y = v1[1];

    const float v0z = v0[2];
    const float v1z = v1[2];

    res[0] = v0y * v1z - v0z * v1y;
    res[1] = -v0x * v1z + v0z * v1x;
    res[2] = v0x * v1y - v0y * v1x;
}

static inline float vec2_cross(const vec2_type v0, const vec2_type v1) {
    const float res_2 = v0[0] * v1[1] - v0[1] * v1[0];
    return res_2;
}

static inline int vec2int_cross(const vec2int_type v0, const vec2int_type v1) {
    const int res_2 = v0[0] * v1[1] - v0[1] * v1[0];
    return res_2;
}

static inline void vec3_reflect(vec3_type res, const vec3_type v0, const vec3_type normal) {
    assert(float_is_equal(vec3_length(normal), 1.f));

    const float p = 2.f * vec3_dot(v0, normal);

    for (size_t i = 0; i < 3; ++i) {
        res[i] = v0[i] - p * normal[i];
    }
}

static inline void vec4_cross(vec4_type res, const vec4_type v0, const vec4_type v1) {
    const float v0x = v0[0];
    const float v1x = v1[0];

    const float v0y = v0[1];
    const float v1y = v1[1];

    const float v0z = v0[2];
    const float v1z = v1[2];

    res[0] = v0y * v1z - v0z * v1y;
    res[1] = -v0x * v1z + v0z * v1x;
    res[2] = v0x * v1y - v0y * v1x;
    res[3] = 1.f;
}

static inline void vec4_reflect(vec4_type res, const vec4_type v0, const vec4_type normal) {
    assert(float_is_equal(vec4_length(normal), 1.f));

    const float p = 2.f * vec4_dot(v0, normal);

    for (size_t i = 0; i < 4; ++i) {
        res[i] = v0[i] - p * normal[i];
    }
}
