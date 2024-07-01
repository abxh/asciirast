#pragma once

#include "transform.h"

#include <stdbool.h>

// 2D Point Clipping
// ----------------------------------------------------------------------------------------------------------

static inline bool clip_point_2d(const vec2_type f0, const float xmin, const float ymin, const float xmax, const float ymax) {
    return inside_range_vec2(f0, (vec2_type){xmin, ymin}, (vec2_type){xmax, ymax});
}

// 2D Line Clipping
// ----------------------------------------------------------------------------------------------------------

static inline bool clip_line_2d(const vec2_type f0, const vec2_type f1, const float xmin, const float ymin, const float xmax,
                                const float ymax, float* out_t0, float* out_t1) {
    // Based on (Liang-Barsky algorithm implementation):
    // https://www.geeksforgeeks.org/liang-barsky-algorithm/

    const float x1 = f0.x;
    const float y1 = f0.y;
    const float x2 = f1.x;
    const float y2 = f1.y;

    const float dx = x2 - x1;
    const float dy = y2 - y1;

    // 0, 1, 2, 3: left, right, bottom, top
    const float p[4] = {-dx, dx, -dy, dy};
    const float q[4] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};

    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    for (size_t i = 0; i < 4; i++) {
        if (is_equal_float(p[i], 0.f)) { // Check if line is parallel to the clipping boundary
            if (q[i] < 0.f) {
                return false; // Line is outside and parallel, so completely discarded
            }
        } else {
            const float t = q[i] / p[i];

            if (p[i] < 0.f) {
                if (t > *out_t0) {
                    *out_t0 = t;
                }
            } else {
                if (t < *out_t1) {
                    *out_t1 = t;
                }
            }
        }
    }

    if (*out_t0 > *out_t1) {
        return false; // Line is completely outside
    }

    return true;
}

// 3D Line Clipping
// ----------------------------------------------------------------------------------------------------------

static inline bool clip_line_3d_w_plane(const vec3_type f0, const vec3_type f1, const vec3_type p_on_plane,
                                        const vec3_type plane_normal, float* out_t0, float* out_t1) {
    // linesegment - plane intesection:
    // https://math.stackexchange.com/a/4432225
    // determine which side of plane:
    // https://stackoverflow.com/a/15691064

    const vec3_type n0 = normalize_vec3(plane_normal);

    const float d = -dot_vec3(n0, p_on_plane);
    const float n_dot_f0 = dot_vec3(n0, f0);
    const float n_dot_f1 = dot_vec3(n0, f1);

    const float top = -d - n_dot_f0;
    const float bottom = n_dot_f1 - n_dot_f0;

    *out_t0 = 0.f;
    *out_t1 = 1.f;
    const bool is_parallel = is_equal_float(bottom, 0.f);
    if (is_parallel) {
        const bool forward_side = dot_vec3(n0, f0) + d >= 0.f;
        return forward_side;
    }
    const float t = top / bottom;
    const bool intersection_with_line_segment = !inside_range_float(t, 0.f, 1.f);
    if (intersection_with_line_segment) {
        return false;
    }

    const bool f0_is_in_front = dot_vec3(n0, f0) + d >= 0.f;
    if (f0_is_in_front) {
        *out_t1 = t;
    } else {
        *out_t0 = t;
    }
    return true;
}
