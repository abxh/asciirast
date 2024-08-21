#include "math/int.h"
#include "math/vec.h"

#include "rasterizer/draw_propi.h"
// #include "sc_list.h"

#include <stdbool.h>

typedef enum { LEFT = 0, RIGHT, BOTTOM, TOP, NEAR, FAR } border_id_type;

static inline bool cull_point_2d(const vec2_type v0) {
    return !vec2_in_range(v0, (vec2_type){.x = -1.f, .y = -1.f}, (vec2_type){.x = 1.f, .y = 1.f});
}

// Liang-barsky algorithm based on:
// https://en.wikipedia.org/wiki/Liang%E2%80%93Barsky_algorithm
// https://github.com/Larry57/WinForms3D/blob/master/WinForms3D/Clipping/LiangBarskyClipping2D.cs
// https://gist.github.com/Larry57/a8a66824b4b299a26fbe64ef5de6f53e

static inline bool internal_liang_barsky_2d_helper(const float p, const float q, float* t0_p, float* t1_p) {
    if (float_is_equal(p, 0.f)) { // Check if line is parallel to the clipping boundary
        if (q < 0.f) {
            return false; // Line is outside and parallel, so completely discarded
        }
        return true; // Line is inside and parallel, so kept as is
    }
    const float u = q / p; // t for the intersection point of the line and the window edge (possibly projected)

    if (p < 0.f) { // the line proceeds outside to inside the clip window
        if (*t1_p < u) {
            return false;
        }
        *t0_p = float_max(u, *t0_p);
    } else { // the line proceeds inside to outside the clip window
        if (u < *t0_p) {
            return false;
        }
        *t1_p = float_min(u, *t1_p);
    }
    return true;
}

static inline bool internal_liang_barsky_2d(const int border_id, const vec2_type v0, const vec2_type v1, const vec2_type min,
                                            const vec2_type max, float* t0_p, float* t1_p) {
    assert(int_in_range(border_id, LEFT, TOP));

    // liang barsky algorithm:

    const float dx = v1.x - v0.x;
    const float dy = v1.y - v0.y;

    // clang-format off
    const float q[4] = {
        v0.x - min.x, max.x - v0.x,
        v0.y - min.y, max.y - v0.y,
    };
    const float p[4] = {
        -dx, dx,
        -dy, dy,
    };
    // clang-format on

    if (!internal_liang_barsky_2d_helper(p[border_id], q[border_id], t0_p, t1_p)) {
        return false;
    }
    return true;
}

static inline bool clip_line_2d(const vec2_type v0, const vec2_type v1, float* out_t0, float* out_t1) {
    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    const vec2_type min = {.x = -1.f, .y = -1.f};
    const vec2_type max = {.x = +1.f, .y = +1.f};

    for (int id = LEFT; id <= TOP; id++) {
        if (!internal_liang_barsky_2d(id, v0, v1, min, max, out_t0, out_t1)) {
            return false;
        }
    }

    return true;
}

static inline bool cull_triangle_back_face_2d(const vec2_type v[3]) {
    const vec2_type p0_to_p1 = vec2_sub(v[1], v[0]);
    const vec2_type p0_to_p2 = vec2_sub(v[2], v[0]);

    return vec2_cross(p0_to_p1, p0_to_p2) >= 0;
}

// Triangle clipping algorithm idea:
// https://www.youtube.com/watch?v=HXSuNxpCzdM

static inline int internal_vert3_2d_count_points_inside(const int border_id, const vec2_type vert3[3], bool out_inside[3]) {

    vec2_type min = {-INFINITY, -INFINITY};
    vec2_type max = {+INFINITY, +INFINITY};

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

    out_inside[0] = vec2_in_range(vert3[0], min, max);
    out_inside[1] = vec2_in_range(vert3[1], min, max);
    out_inside[2] = vec2_in_range(vert3[2], min, max);

    const int count = out_inside[0] + out_inside[1] + out_inside[2];

    return count;
}

static inline void internal_get_ordered_verticies_from_inside_points(const bool inside[3], int out_idx[3]) {
    // order:
    // ... -> 0 -> 1 -> 2 -> ...

    // first two points are inside. last one is outside.
    if (inside[0] && inside[1]) {
        out_idx = (int[3]){0, 1, 2};

    } else if (inside[0] && inside[2]) {
        out_idx = (int[3]){2, 0, 1};

    } else if (inside[1] && inside[2]) {
        out_idx = (int[3]){1, 2, 0};

        // first point is inside. others are outside.
    } else if (inside[0]) {
        out_idx = (int[3]){0, 1, 2};

    } else if (inside[1]) {
        out_idx = (int[3]){1, 2, 0};

    } else if (inside[2]) {
        out_idx = (int[3]){2, 0, 1};
    }
}

typedef struct {
    vec2_type vert3[3];
    propi_rep_type propi3[3];
} clipped_triangle_2d_type;

typedef struct {
    size_t begin;
    size_t end;
    clipped_triangle_2d_type buf[128];
} clip_2d_queue_type;

static clip_2d_queue_type g_clip_2d_queue[128];

static inline void clip_triangle_2d(const vec2_type vert3[3], const propi_rep_type propi3[3]) {
    clipped_triangle_2d_type head = {.vert3 = {vert3[0], vert3[1], vert3[2]}, .propi3 = {propi3[0], propi3[1], propi3[2]}};
}
