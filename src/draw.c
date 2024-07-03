#include "draw.h"
#include "log.h"
#include "math/mat4x4.h"
#include "math/vec.h"
#include "screen.h"

#define STACK_PREFIX vert2dstk
#define VALUE_TYPE vertix_2d_type
#include "data_structures/stack.h"

typedef struct {
    size_t ascii_palette_size;
    char index_to_ascii[128];
    int ascii_to_index[128];
} ascii_index_conversion_table;

typedef struct renderer_type {
    mat4x4_type perspective_mat;
    mat4x4_type model_camera_mat;
    mat4x4_type mvp;

    struct screen_type* screen_context_p;
    vert2dstk_type* vert2dstk_p;

    ascii_index_conversion_table conv;

    bool initialized;
} renderer_type;

// renderer init/deinit
// ------------------------------------------------------------------------------------------------------------

renderer_type* renderer_create(struct screen_type* screen_context_p, const size_t n, const char ascii_palette[n],
                               const perspective_proj_prop_type prop) {
    renderer_type* this = malloc(sizeof(renderer_type));

    HANDLE_NULL(this, "malloc");

    this->initialized = true;

    this->screen_context_p = screen_context_p;
    mat4x4_perspective(this->perspective_mat, prop.fovy_rad, prop.aspect_ratio, prop.z_near, prop.z_far);
    mat4x4_copy(this->mvp, this->perspective_mat);

    for (size_t i = 0; i < 128; i++) {
        this->conv.ascii_to_index[i] = -1;
        this->conv.index_to_ascii[i] = ' ';
    }

    for (size_t i = 0; i < n; i++) {
        assert(int_is_inside_range(ascii_palette[i], 32, 126) && "ascii char is not printable");

        const char c = ascii_palette[i];
        this->conv.ascii_to_index[(int)c] = (int)i;
        this->conv.index_to_ascii[i] = c;
    }

    this->vert2dstk_p = vert2dstk_create_with_initial_capacity(32);

    return this;
}

void renderer_destroy(renderer_type* this) {
    vert2dstk_destroy(this->vert2dstk_p);
    free(this);
}

// vertix properties copy
// ------------------------------------------------------------------------------------------------------------

static inline void vertix_prop_copy(vertix_prop_type* res, const vertix_prop_type* prop) {
    res->ascii_char = prop->ascii_char;
    vec3_copy(res->color.as_vec3, prop->color.as_vec3);
}

// valid vertix check
// ------------------------------------------------------------------------------------------------------------

static inline bool valid_vertix_2d_check(const ascii_index_conversion_table* conv, const size_t n, const vertix_2d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        const char c = v[i].prop.ascii_char;
        is_valid &= conv->ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].prop.color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}

static inline bool valid_vertix_3d_check(const ascii_index_conversion_table* conv, const size_t n, const vertix_3d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        const char c = v[i].prop.ascii_char;
        is_valid &= conv->ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].prop.color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}

// vertix property 2d lerp
// ------------------------------------------------------------------------------------------------------------

static inline void vertix_prop_lerp(const ascii_index_conversion_table* conv, vertix_prop_type* res, const vertix_prop_type v0,
                                    const vertix_prop_type v1, const float t) {
    vec3_lerp(res->color.as_vec3, v0.color.as_vec3, v1.color.as_vec3, t);

    const int c0 = conv->ascii_to_index[(int)v0.ascii_char];
    const int c1 = conv->ascii_to_index[(int)v1.ascii_char];
    const int c = int_lerped_rounded(c0, c1, t);

    res->ascii_char = conv->index_to_ascii[c];
}

// world space -> screen space
// ------------------------------------------------------------------------------------------------------------

static inline void vec2_transform_pos_to_screen_space(vec2_type res, const vec2_type v) {
    vec2_add(res, v, (vec2_type){1.f, 1.f});
    vec2_scale(res, res, 1.f / 2.f);
    vec2_elementwise_prod(res, res, (vec2_type){(float)SCREEN_WIDTH - 1.f, (float)SCREEN_HEIGHT - 1.f});
}

// internal draw routines
// ------------------------------------------------------------------------------------------------------------

static inline void internal_plot_point(const renderer_type* this, const vec2_type* pos, const vertix_prop_type* prop,
                                       const float depth0) {
    vec2int_type r_v;
    vec2_rounded_to_vec2int(r_v, *pos);
    screen_set_pixel_data(this->screen_context_p, r_v,
                          (pixel_data_type){.color = prop->color, .depth = depth0, .ascii_char = prop->ascii_char});
}

static inline void internal_plot_line(const renderer_type* this, const vec2_type pos[2], const vertix_prop_type prop[2],
                                      const float depth[2]) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    const int dx = float_rounded_to_int(float_abs(pos[1][0] - pos[0][0]));
    const int dy = float_rounded_to_int(float_abs(pos[1][1] - pos[0][1]));
    const int diagonal_dist = dx > dy ? dx : dy;

    if (diagonal_dist == 0) {
        return;
    }

    vec2int_type p0, p1;
    vec2_rounded_to_vec2int(p0, pos[0]);
    vec2_rounded_to_vec2int(p1, pos[1]);

    for (int step = 0; step <= diagonal_dist; step++) {
        const float t = (float)step / (float)diagonal_dist;
        const float d = float_lerped(depth[0], depth[1], t);

        vec2int_type p;
        vec2int_lerp_rounded(p, p0, p1, t);

        vertix_prop_type prop_new;
        vertix_prop_lerp(&this->conv, &prop_new, prop[0], prop[1], t);

        screen_set_pixel_data(this->screen_context_p, p,
                              (pixel_data_type){.color = prop->color, .depth = d, .ascii_char = prop->ascii_char});
    }
}

static inline bool internal_is_top_left_edge_of_triangle(const vec2_type src, const vec2_type dest) {
    vec2_type edge;
    vec2_sub(edge, dest, src);

    const bool points_right = edge[0] > 0;
    const bool points_up = edge[1] < 0; // since y-axis points down for framebuffer

    const bool is_top_edge = float_is_equal(edge[1], 0.f) && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

static inline void internal_plot_triangle(const renderer_type* this, const vec2_type pos[3], const vertix_prop_type prop[3],
                                          const float depth[3]) {
    // baycentric algorithm:
    // https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const int maxX = float_rounded_to_int(float_max(pos[0][0], float_max(pos[1][0], pos[2][0])));
    const int minX = float_rounded_to_int(float_min(pos[0][0], float_min(pos[1][0], pos[2][0])));
    const int maxY = float_rounded_to_int(float_max(pos[0][1], float_max(pos[1][1], pos[2][1])));
    const int minY = float_rounded_to_int(float_min(pos[0][1], float_min(pos[1][1], pos[2][1])));

    const vec2_type p0 = {(float)minX + 0.5f, (float)minY + 0.5f};

    // bias to include top left edge and exclude bottom right edge:
    const float bias0 = internal_is_top_left_edge_of_triangle(pos[1], pos[2]) ? 0 : -1;
    const float bias1 = internal_is_top_left_edge_of_triangle(pos[2], pos[0]) ? 0 : -1;
    const float bias2 = internal_is_top_left_edge_of_triangle(pos[0], pos[1]) ? 0 : -1;

    // relevant vectors:
    vec2_type v1_to_v2, v2_to_v0, v0_to_v1, v0_to_v2, v1_to_p0, v2_to_p0, v0_to_p0;

    vec2_sub(v1_to_v2, pos[2], pos[1]);
    vec2_sub(v2_to_v0, pos[0], pos[2]);
    vec2_sub(v0_to_v1, pos[1], pos[0]);
    vec2_sub(v0_to_v2, pos[2], pos[0]);

    vec2_sub(v0_to_p0, p0, pos[0]);
    vec2_sub(v1_to_p0, p0, pos[1]);
    vec2_sub(v2_to_p0, p0, pos[2]);

    const float triangle_area_2 = vec2_cross(v0_to_v1, v0_to_v2);

    // for efficient cross product calculation for each point in the bounding box. see video for derivation.
    const float delta_w0_col = pos[1][1] - pos[2][1];
    const float delta_w0_row = pos[2][0] - pos[1][0];
    float w0_row = vec2_cross(v1_to_v2, v1_to_p0) + bias0;

    const float delta_w1_col = pos[2][1] - pos[0][1];
    const float delta_w1_row = pos[0][0] - pos[2][0];
    float w1_row = vec2_cross(v2_to_v0, v2_to_p0) + bias1;

    const float delta_w2_col = pos[0][1] - pos[1][1];
    const float delta_w2_row = pos[1][0] - pos[0][0];
    float w2_row = vec2_cross(v0_to_v1, v0_to_p0) + bias2;

    for (int y = minY; y <= maxY; y++) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = minX; x <= maxX; x++) {
            bool is_inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
            if (is_inside_triangle) {
                const float alpha = w0 / triangle_area_2;
                const float beta = w1 / triangle_area_2;
                const float gamma = w2 / triangle_area_2;

                const float d = alpha * depth[0] + beta * depth[1] + gamma * depth[2];

                color_type c, c_comp0, c_comp1, c_comp2;
                vec3_scale(c_comp0.as_vec3, prop[0].color.as_vec3, alpha);
                vec3_scale(c_comp1.as_vec3, prop[1].color.as_vec3, beta);
                vec3_scale(c_comp2.as_vec3, prop[2].color.as_vec3, gamma);
                vec3_add(c.as_vec3, c_comp0.as_vec3, c_comp1.as_vec3);
                vec3_add(c.as_vec3, c.as_vec3, c_comp2.as_vec3);

                char ch;
                float ch_comp0_idx, ch_comp1_idx, ch_comp2_idx;
                ch_comp0_idx = int_to_float(this->conv.ascii_to_index[(int)prop[0].ascii_char]);
                ch_comp1_idx = int_to_float(this->conv.ascii_to_index[(int)prop[1].ascii_char]);
                ch_comp2_idx = int_to_float(this->conv.ascii_to_index[(int)prop[2].ascii_char]);
                ch =
                    this->conv.index_to_ascii[float_rounded_to_int(alpha * ch_comp0_idx + beta * ch_comp1_idx + gamma * ch_comp2_idx)];

                screen_set_pixel_data(this->screen_context_p, (int[2]){x, y},
                                      (pixel_data_type){.color = c, .depth = d, .ascii_char = ch});
            }
            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;
    }
}

// internal clip routines
// ------------------------------------------------------------------------------------------------------------

static inline bool cull_and_clip_line_2d(const vec2_type p1, const vec2_type p2, const float xmin, const float ymin, const float xmax,
                                         const float ymax, float* out_t1, float* out_t2) {
    // Based on (Liang-Barsky algorithm implementation):
    // https://www.geeksforgeeks.org/liang-barsky-algorithm/

    const float x1 = p1[0];
    const float y1 = p1[1];
    const float x2 = p2[0];
    const float y2 = p2[1];

    const float dx = x2 - x1;
    const float dy = y2 - y1;

    const float p[4] = {-dx, dx, -dy, dy};
    const float q[4] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};

    *out_t1 = 0.0f;
    *out_t2 = 1.0f;

    for (size_t i = 0; i < 4; i++) {
        if (float_is_equal(p[i], 0.f)) { // Check if line is parallel to the clipping boundary
            if (q[i] < 0.f) {
                return false; // Line is outside and parallel, so completely discarded
            }
        } else {
            // 1st loop: against left border
            // 2nd loop: against right border
            // 3rd loop: against bottom border
            // 4th loop: against top border

            // 1st loop: t <- (x1 - xmin) / (x1 - x2)
            // 2nd loop: t <- (xmax - x1) / (x2 - x1)
            // 3rd loop: t <- (y1 - ymin) / (y1 - y2)
            // 4th loop: t <- (ymax - y1) / (y2 - y1)
            const float t = q[i] / p[i];

            // 1st loop: (x1 < x2) ? try_change_p1() : try_change_p2()
            // 2nd loop: (x2 < x1) ? try_change_p1() : try_change_p2()
            // 3rd loop: (y1 < y2) ? try_change_p1() : try_change_p2()
            // 4th loop: (y2 < y1) ? try_change_p1() : try_change_p2()
            const bool try_change_p1 = p[i] < 0.f;
            if (try_change_p1) {
                if (t > *out_t1) { // implicity also check (t > 0.0f)
                    *out_t1 = t;
                }
            } else {
                if (t < *out_t2) { // implicitly also check (t < 1.0f)
                    *out_t2 = t;
                }
            }
        }
    }

    if (*out_t1 > *out_t2) {
        return false; // Line is completely outside
    }

    return true;
}

static inline bool clip_line_3d_w_plane(const vec3_type v0, const vec3_type v1, const vec3_type point_on_plane,
                                        const vec3_type plane_normal, float* out_t0, float* out_t1) {
    // linesegment - plane intesection:
    // https://math.stackexchange.com/a/4432225

    assert(float_is_equal(vec3_length(plane_normal), 1.f));

    vec3_type n;
    // vec3_norm(n, plane_normal);
    vec3_copy(n, plane_normal);

    // note:
    // n . P == 0, if plane goes through origo -- since n and p(x,y,z) are perpendicular.
    // n . (P - d) == 0, where d is the distance vector from origo, if plane goes through origo.
    // hence
    // n . P == d . n

    const float d_dot_n = vec3_dot(point_on_plane, n);

    const float v0_dot_n = vec3_dot(v0, n);
    const float v1_dot_n = vec3_dot(v1, n);

    // let the line segment be represented by:
    //     V = v0 + t * (v1 - v0), t in [0; 1]

    // We like to solve for t in (V . n == d . n)
    // <=> n . (v0 + t * (v1 - v0)) = d_dot_n
    // <=> v0_dot_n + t * (v1_dot_n - v0_dot_n) = d_dot_n
    // <=> t == (d_dot_n - v0_dot_n) / (v1_dot_n - v0_dot_n)

    const float frac_top = d_dot_n - v0_dot_n;
    const float frac_bottom = v1_dot_n - v0_dot_n;

    *out_t0 = 0.f;
    *out_t1 = 1.f;

    const bool is_parallel = float_is_equal(frac_bottom, 0.f); // aka no solutions
    if (is_parallel) {
        // determine which side of plane:
        // https://stackoverflow.com/a/15691064

        const bool same_side_as_normal = vec3_dot(n, v0) - d_dot_n >= 0.f;

        return same_side_as_normal;
    }

    const float t = frac_top / frac_bottom;

    const bool intersection_with_line_segment = float_is_inside_range(t, 0.f, 1.f);
    if (!intersection_with_line_segment) {
        return false;
    }

    const bool v1_same_side_as_normal = vec3_dot(n, v0) + d_dot_n >= 0.f;
    if (v1_same_side_as_normal) {
        *out_t0 = t; // otherwise defaults to 0.
    } else {
        *out_t1 = t; // otherwise defaults to 1.
    }
    return true;
}

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(const renderer_type* this, const vertix_2d_type v[1], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 1, v));

    if (!vec2_is_inside_range(v[0].pos, (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f})) {
        return;
    }

    const float depth = (float)z_order / UINT8_MAX;

    vec2_type pos;
    vec2_transform_pos_to_screen_space(pos, v[0].pos);

    vertix_prop_type prop;
    vertix_prop_copy(&prop, &v[0].prop);

    internal_plot_point(this, &pos, &prop, depth);
}

void draw_line_2d(const renderer_type* this, const vertix_2d_type v[2], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 2, v));

    float t[2];
    if (!cull_and_clip_line_2d(v[0].pos, v[1].pos, -1.f, -1.f, 1.f, 1.f, &t[0], &t[1])) {
        return;
    }

    const float depth = (float)z_order / UINT8_MAX;

    vec2_type pos[2];
    vertix_prop_type prop[2];

    for (size_t i = 0; i < 2; i++) {
        vec2_lerp(pos[i], v[0].pos, v[1].pos, t[i]);
        vertix_prop_lerp(&this->conv, &prop[i], v[0].prop, v[1].prop, t[i]);
    }

    for (size_t i = 0; i < 2; i++) {
        vec2_transform_pos_to_screen_space(pos[i], pos[i]);
    }

    internal_plot_line(this, pos, prop, (float[2]){depth, depth});
}

void draw_filled_triangle_2d(const renderer_type* this, const vertix_2d_type v[3], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 3, v));

    // TODO: culling / clipping

    const float depth = (float)z_order / UINT8_MAX;

    vec2_type pos[3];
    vertix_prop_type prop[3];

    for (size_t i = 0; i < 2; i++) {
        vertix_prop_copy(&prop[i], &v[i].prop);
        vec2_transform_pos_to_screen_space(pos[i], v[i].pos);
    }

    internal_plot_triangle(this, pos, prop, (float[3]){depth, depth, depth});
}

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(const struct renderer_type* this, const vertix_3d_type v[1]) {
    assert(valid_vertix_3d_check(&this->conv, 1, v));

    // const bool forward_facing = v[0].w > 0;
    // if (!vec2_is_inside_range(v[0].pos, (vec3_type){-v[0].w, -v[0].w}, (vec2_type){v[0].w, v[0].w}) || !forward_facing) {
    //     return;
    // }
    //
    // vertix_3d_type res[1];
    //
    // for (size_t i = 0; i < 1; i++) {
    //     res[i].ascii_char = v[i].ascii_char;
    //     mat4x4_mul_vec4(res[i].pos, this->mvp, v->pos);
    //     vec3_copy(res[i].color.as_vec3, v[i].color.as_vec3);
    // }
    //
    // for (size_t i = 0; i < 1; i++) {
    //     vec2_transform_pos_to_screen_space(res[i].pos, res[i].pos);
    // }
}
