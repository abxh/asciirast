
// Some references:
// - Liang-Barsky algorithm:
//      https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
//      https://www.geeksforgeeks.org/liang-barsky-algorithm/
//      https://gist.github.com/Larry57/a8a66824b4b299a26fbe64ef5de6f53e
//      https://www.gamedev.net/forums/topic/369105-liang-barsky-clipping-in-homogeneous-coordinates/
//      https://gamedev.stackexchange.com/questions/112528/liang-barsky-line-clipping-algorithm
// - Triangle clipping algorithm. Liang-Barsky is used instead of half-plane intersections.
//      https://www.youtube.com/watch?v=HXSuNxpCzdM

#pragma once

#include "math/int.h"
#include "math/vec.h"
#include "rasterizer/draw/draw_propi.h"
#include "sc_list.h"

#include <stdint.h>

typedef enum { LEFT = 0, RIGHT, BOTTOM, TOP } border_id_type;

typedef struct {
    vec4_type pos[3];
    propi_rep_type prop[3];

    bool clipped_at_border_id[4];

    struct sc_list next;
} triangle_data_type;

// internal
// ------------------------------------------------------------------------------------------------------------

static inline bool internal_clip_line_helper(const float p, const float q, float* out_t0, float* out_t1) {
    if (float_is_equal(p, 0.f)) { // Check if line is parallel to the clipping boundary
        if (q < 0.f) {
            return false; // Line is outside and parallel, so completely discarded
        }
        return true;
    }

    const float u = q / p; // t for the intersection point of the line and the window edge (possibly projected)

    if (p < 0.f) {
        // the line proceeds outside to inside the clip window
        if (*out_t1 < u) {
            return false;
        }
        *out_t0 = float_max(u, *out_t0);
    } else {
        // the line proceeds inside to outside the clip window
        if (u < *out_t0) {
            return false;
        }
        *out_t1 = float_min(u, *out_t1);
    }
    return true;
}

static inline bool internal_clip_line_w_border(const int border_id, const vec2_type v0, const vec2_type v1, const vec2_type min,
                                               const vec2_type max, const float dw, float* out_t0, float* out_t1) {
    assert(int_in_range(border_id, LEFT, TOP));

    // liang barsky algorithm:

    const float dx = v1.x - v0.x;
    const float dy = v1.y - v0.y;

    // clang-format off
    const float q[4] = {
        +(v0.x - min.x), -(v0.x - max.x),
        +(v0.y - min.y), -(v0.y - max.y),
    };
    const float p[4] = {
        +dw - dx, -dw + dx,
        +dw - dy, -dw + dy,
    };
    // clang-format on

    if (!internal_clip_line_helper(p[border_id], q[border_id], out_t0, out_t1)) {
        return false;
    }
    return true;
}

static inline bool internal_homogeneous_line_4d_w_border(const int border_id, const vec4_type v0, const vec4_type v1, float* out_t0,
                                                         float* out_t1) {
    const float dw = v1.w - v0.w;

    const vec2_type pos0 = (vec2_type){.x = v0.x, .y = v0.y};
    const vec2_type pos1 = (vec2_type){.x = v1.x, .y = v1.y};

    const vec2_type min = (vec2_type){.x = -v0.w, .y = -v0.w};
    const vec2_type max = (vec2_type){.x = +v0.w, .y = +v0.w};

    return internal_clip_line_w_border(border_id, pos0, pos1, min, max, dw, out_t0, out_t1);
}

// public
// ------------------------------------------------------------------------------------------------------------

static inline bool back_face_cull_2d(const vec2_type v[3]) {
    const vec2_type p0_to_p1 = vec2_sub(v[1], v[0]);
    const vec2_type p0_to_p2 = vec2_sub(v[2], v[0]);

    return vec2_cross(p0_to_p1, p0_to_p2) >= 0;
}

static inline bool back_face_cull_3d(const vec4_type v[3]) {
    const vec2_type p0_to_p1 = vec2_sub(vec2_from_array(v[1].array), vec2_from_array(v[0].array));
    const vec2_type p0_to_p2 = vec2_sub(vec2_from_array(v[2].array), vec2_from_array(v[0].array));

    return vec2_cross(p0_to_p1, p0_to_p2) >= 0; // assuming camera is at (0,0,-1) after mvp transformation
}

static inline bool clip_line_2d(const vec2_type v0, const vec2_type v1, float* out_t0, float* out_t1) {
    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    const vec2_type min = {.array = {-1.f, -1.f}};
    const vec2_type max = {.array = {1.f, 1.f}};

    for (int id = 0; id < 4; id++) {
        if (!internal_clip_line_w_border(id, v0, v1, min, max, 0, out_t0, out_t1)) {
            return false;
        }
    }

    return true;
}

static inline bool clip_line_3d(const vec4_type v0, const vec4_type v1, float* out_t0, float* out_t1) {
    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    for (int id = 0; id < 4; id++) {
        if (!internal_homogeneous_line_4d_w_border(id, v0, v1, out_t0, out_t1)) {
            return false;
        }
    }

    return true;
}

/*
static inline int internal_triangle_count_points_inside(const int border_id, const triangle_data_type vert3, bool inside[3]) {

    vec4_type min = {-INFINITY, -INFINITY, -INFINITY, 0};
    vec4_type max = {+INFINITY, +INFINITY, +INFINITY, INFINITY};

    switch (border_id) {
    case LEFT:
        min.x = -1.f;
        break;
    case RIGHT:
        max.x = +1.f;
        break;
    case BOTTOM:
        min.y = -1.f;
        break;
    case TOP:
        max.y = +1.f;
        break;
    }

    inside[0] = vec4_in_range(vert3.pos[0], min, max);
    inside[1] = vec4_in_range(vert3.pos[1], min, max);
    inside[2] = vec4_in_range(vert3.pos[2], min, max);
    const int count = inside[0] + inside[1] + inside[2];

    return count;
}

static inline void internal_get_ordered_verticies_from_inside_points(const bool inside[3], int idx[3]) {
    // order:
    // 0 -> 1 -> 2
    if (inside[0] && inside[1]) {
        // inside:
        idx[0] = 0;
        idx[1] = 1;
        // outside:
        idx[2] = 2;
    } else if (inside[0] && inside[2]) {
        // inside:
        idx[0] = 2;
        idx[1] = 0;
        // outside:
        idx[2] = 1;
    } else if (inside[1] && inside[2]) {
        // inside:
        idx[0] = 1;
        idx[1] = 2;
        // outside:
        idx[2] = 0;
    } else if (inside[0]) {
        // inside:
        idx[0] = 0;
        // outside:
        idx[1] = 1;
        idx[2] = 2;
    } else if (inside[1]) {
        // inside:
        idx[0] = 1;
        // outside:
        idx[1] = 2;
        idx[2] = 0;
    } else if (inside[2]) {
        // inside:
        idx[0] = 2;
        // outside:
        idx[1] = 0;
        idx[2] = 1;
    }
}

static inline void internal_clip_triangle(const ascii_table_type* table_p, const vec4_type v[3], const propi_rep_type prop[3]) {
    struct sc_list list;

    triangle_data_type vert3 = {.pos = {v[0], v[1], v[2]}, .prop = {prop[0], prop[1], prop[2]}, .clipped_at_border_id = {false}};
    sc_list_add_head(&list, &vert3.next);

    for (int id = 0; id < 4; id++) {
        struct sc_list* tmp;
        struct sc_list* it;

        sc_list_foreach_safe(&list, tmp, it) {
            triangle_data_type test = *sc_list_entry(it, triangle_data_type, next);

            bool inside[3];
            const int inside_point_count = internal_triangle_count_points_inside(id, test, inside);

            switch (inside_point_count) {
            case 0:
                sc_list_del(&list, it);
                break;
            case 1: {
                bool ret1, ret2;
                int idx[3];

                float t0 = 0.f;
                float t1 = 1.f;
                float t2 = 1.f;

                internal_get_ordered_verticies_from_inside_points(inside, idx);

                .x = test.

                     ret1 = internal_clip_line_w_border(id, test.pos[idx[0]], test.pos[idx[1]], min1, max1, dw1, &t0, &t1);
                assert(ret1);
                assert(float_is_equal(t0, 0.f));
                assert(t1 < 1.f);

                ret2 = internal_clip_line_w_border(id, test.pos[idx[0]], test.pos[idx[2]], min, max, &t0, &t2);
                assert(ret2);
                assert(float_is_equal(t0, 0.f));
                assert(t2 < 1.f);

                vec2_lerp(test.pos[idx[1]].pos, test.pos[idx[0]].pos, test.pos[idx[1]].pos, t1);
                vec2_lerp(test.pos[idx[2]].pos, test.pos[idx[0]].pos, test.pos[idx[2]].pos, t2);

                test.pos[idx[1]].prop = vertix_prop_lerped(conv, test.pos[idx[0]].prop, test.pos[idx[1]].prop, t1);
                test.value[idx[2]].prop = vertix_prop_lerped(conv, test.value[idx[0]].prop, test.value[idx[2]].prop, t2);

                test.clipped_at_border_id[id] = true;
                lst_vert3_2d_push_back(lst_p, test);
            } break;
            }
        }
    }
}
*/
