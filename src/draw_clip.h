
// Some references:
// - Liang-Barsky algorithm:
//      https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
//      https://www.geeksforgeeks.org/liang-barsky-algorithm/
//      https://gamedev.stackexchange.com/questions/112528/liang-barsky-line-clipping-algorithm
//      https://gist.github.com/Larry57/a8a66824b4b299a26fbe64ef5de6f53e
// - Triangle clipping [not exact impl -- sutherland clipping is used instead]:
//      https://www.youtube.com/watch?v=HXSuNxpCzdM

#pragma once

#include "draw.h"
#include "draw_vertix_prop.h"
#include "math/vec.h"

#include <stdint.h>

typedef enum { LEFT = 0, RIGHT, BOTTOM, TOP } border_id_type;

typedef struct {
    vertix_2d_type value[3];
    bool clipped_at_border_id[4];
} vert3_2d;

#define P
#define T vert3_2d
#include <lst.h>

// internal cull/clip routines
// ------------------------------------------------------------------------------------------------------------

static inline bool internal_vert3_2d_back_face_cull(const vert3_2d vert) {
    vec2_type p0_to_p1;
    vec2_type p0_to_p2;

    vec2_sub(p0_to_p1, vert.value[1].pos, vert.value[0].pos);
    vec2_sub(p0_to_p2, vert.value[2].pos, vert.value[0].pos);

    return vec2_cross(p0_to_p2, p0_to_p1) >= 0;
}

static inline bool internal_clip_line_2d_helper(const float p, const float q, float* out_t0, float* out_t1) {
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

static inline bool internal_clip_line_2d_w_border(const int border_id, const vec2_type pos0, const vec2_type pos1, const vec2_type min,
                                                  const vec2_type max, float* out_t0, float* out_t1) {
    assert(int_is_inside_range(border_id, LEFT, TOP));

    const float dx = pos1[0] - pos0[0];
    const float dy = pos1[1] - pos0[1];

    // clang-format off
    const float p[4] = {
        -dx, dx,
        -dy, dy,
    };
    const float q[4] = {
        +(pos0[0] - min[0]), -(pos0[0] - max[0]),
        +(pos0[1] - min[1]), -(pos0[1] - max[1]),
    };
    // clang-format on

    if (!internal_clip_line_2d_helper(p[border_id], q[border_id], out_t0, out_t1)) {
        return false;
    }
    return true;
}

static inline bool internal_clip_line_2d(const vec2_type pos0, const vec2_type pos1, const vec2_type min, const vec2_type max,
                                         float* out_t0, float* out_t1) {
    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    for (int id = 0; id < 4; id++) {
        if (!internal_clip_line_2d_w_border(id, pos0, pos1, min, max, out_t0, out_t1)) {
            return false;
        }
    }

    return true;
}

static inline int internal_vert3_2d_count_points_inside(const int border_id, const vert3_2d vert3, int i[3]) {

    vec2_type min = {-INFINITY, -INFINITY};
    vec2_type max = {+INFINITY, +INFINITY};

    switch (border_id) {
    case LEFT:
        min[0] = -1.f;
        break;
    case RIGHT:
        max[0] = +1.f;
        break;
    case BOTTOM:
        min[1] = -1.f;
        break;
    case TOP:
        max[1] = +1.f;
        break;
    }

    const bool p0_inside = vec2_is_inside_range(vert3.value[0].pos, min, max);
    const bool p1_inside = vec2_is_inside_range(vert3.value[1].pos, min, max);
    const bool p2_inside = vec2_is_inside_range(vert3.value[2].pos, min, max);
    const int count = p0_inside + p1_inside + p2_inside;

    if (count == 1) {
        if (p0_inside) {
            // inside:
            i[0] = 0;
            // outside:
            i[1] = 1;
            i[2] = 2;
        } else if (p1_inside) {
            // inside:
            i[0] = 1;
            // outside:
            i[1] = 0;
            i[2] = 2;
        } else {
            // inside:
            i[0] = 2;
            // outside:
            i[1] = 0;
            i[2] = 1;
        }
    } else if (count == 2) {
        if (p0_inside && p1_inside) {
            // inside:
            i[0] = 0;
            i[1] = 1;
            // outside:
            i[2] = 2;
        } else if (p0_inside && p2_inside) {
            // inside:
            i[0] = 0;
            i[1] = 2;
            // outside:
            i[2] = 1;
        } else if (p1_inside && p2_inside) {
            // inside:
            i[0] = 1;
            i[1] = 2;
            // outside:
            i[2] = 0;
        }
    }

    return count;
}

static inline void internal_clip_triangle_2d(const ascii_index_conversion_table* conv, lst_vert3_2d* lst_p, vert3_2d vert3,
                                             const vec2_type min, const vec2_type max) {
    // adding new triangles:
    // https://github.com/OneLoneCoder/Javidx9/blob/master/ConsoleGameEngine/BiggerProjects/Engine3D/OneLoneCoder_olcEngine3D_Part3.cpp

    assert(lst_p->size == 0);

    float t0, t1, t2;
    int idx[3];

    vert3.clipped_at_border_id[0] = false;
    vert3.clipped_at_border_id[1] = false;
    vert3.clipped_at_border_id[2] = false;
    vert3.clipped_at_border_id[3] = false;

    lst_vert3_2d_push_front(lst_p, vert3);
    size_t n_new_triangles = 1;

    for (int id = 0; id < 4; id++) {
        while (n_new_triangles > 0) {
            vert3_2d test = *lst_vert3_2d_front(lst_p);
            lst_vert3_2d_pop_front(lst_p);
            n_new_triangles--;

            const int inside_point_count = internal_vert3_2d_count_points_inside(id, test, idx);
            switch (inside_point_count) {
            case 0:
                break;
            case 1:
                t0 = 0.f;
                t1 = 1.f;
                t2 = 1.f;

                internal_clip_line_2d_w_border(id, test.value[idx[0]].pos, test.value[idx[1]].pos, min, max, &t0, &t1);
                internal_clip_line_2d_w_border(id, test.value[idx[0]].pos, test.value[idx[2]].pos, min, max, &t0, &t2);

                assert(float_is_equal(t0, 0.f));
                assert(!float_is_equal(t1, 1.f));
                assert(!float_is_equal(t2, 1.f));

                vec2_lerp(test.value[idx[1]].pos, test.value[idx[0]].pos, test.value[idx[1]].pos, t1);
                vec2_lerp(test.value[idx[2]].pos, test.value[idx[0]].pos, test.value[idx[2]].pos, t1);

                test.value[idx[1]].prop = vertix_prop_lerped(conv, test.value[idx[0]].prop, test.value[idx[1]].prop, t1);
                test.value[idx[2]].prop = vertix_prop_lerped(conv, test.value[idx[0]].prop, test.value[idx[2]].prop, t2);

                test.clipped_at_border_id[id] = true;
                lst_vert3_2d_push_back(lst_p, test);
                break;
            case 2:
                t0 = 0.f;
                t1 = 1.f;
                t2 = 1.f;

                internal_clip_line_2d_w_border(id, test.value[idx[0]].pos, test.value[idx[2]].pos, min, max, &t0, &t1);
                internal_clip_line_2d_w_border(id, test.value[idx[1]].pos, test.value[idx[2]].pos, min, max, &t0, &t2);

                assert(float_is_equal(t0, 0.f));
                assert(!float_is_equal(t1, 1.f));
                assert(!float_is_equal(t2, 1.f));

                const vert3_2d vert_tmp = {.value = {test.value[idx[0]], test.value[idx[2]], test.value[idx[1]]}};
                const bool winding_order_012 = internal_vert3_2d_back_face_cull(vert_tmp);

                vertix_2d_type v2_0, v2_1;

                vec2_lerp(v2_0.pos, test.value[idx[0]].pos, test.value[idx[2]].pos, t1);
                vec2_lerp(v2_1.pos, test.value[idx[1]].pos, test.value[idx[2]].pos, t2);

                v2_0.prop = vertix_prop_lerped(conv, test.value[idx[0]].prop, test.value[idx[2]].prop, t1);
                v2_1.prop = vertix_prop_lerped(conv, test.value[idx[1]].prop, test.value[idx[2]].prop, t2);

                if (winding_order_012) {
                    vert3_2d vert3_0 = {.value = {test.value[idx[0]], test.value[idx[1]], v2_0}};
                    vert3_2d vert3_1 = {.value = {test.value[idx[1]], v2_1, v2_0}};

                    vert3_1.clipped_at_border_id[id] = true;

                    lst_vert3_2d_push_back(lst_p, vert3_0);
                    lst_vert3_2d_push_back(lst_p, vert3_1);
                } else {
                    vert3_2d vert3_0 = {.value = {test.value[idx[0]], v2_0, test.value[idx[1]]}};
                    vert3_2d vert3_1 = {.value = {test.value[idx[1]], v2_0, v2_1}};

                    vert3_1.clipped_at_border_id[id] = true;

                    lst_vert3_2d_push_back(lst_p, vert3_0);
                    lst_vert3_2d_push_back(lst_p, vert3_1);
                }
                break;
            case 3:
                lst_vert3_2d_push_back(lst_p, test);
                break;
            }
        }
        n_new_triangles = lst_p->size;
    }
}
