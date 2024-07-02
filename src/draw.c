#include "draw.h"
#include "math/mat4x4.h"
#include "math/vec.h"
#include "screen.h"

#define STACK_PREFIX vert2dstk
#define VALUE_TYPE vertix_2d_type
#include "data_structures/stack.h"

typedef struct {
    mat4x4_type mvp;

    struct screen_type* screen_context_p;
    vert2dstk_type* vert2dstk_p;

    size_t ascii_palette_size;
    char index_to_ascii[128];
    int ascii_to_index[128];

    bool initialized;
} renderer_state;

static renderer_state s = {.initialized = false};

// vertix copy
// ------------------------------------------------------------------------------------------------------------

static inline void vertix_2d_copy(const size_t n, vertix_2d_type res[n], const vertix_2d_type v[n]) {
    for (size_t i = 0; i < n; i++) {
        res[i].ascii_char = v[i].ascii_char;
        vec2_copy(res[i].pos, v[i].pos);
        vec3_copy(res[i].color.as_vec3, v[i].color.as_vec3);
    }
}

// valid vertix check
// ------------------------------------------------------------------------------------------------------------

static inline bool valid_vertix_2d_check(const size_t n, const vertix_2d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        const char c = v[i].ascii_char;
        is_valid &= s.ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}

static inline bool valid_vertix_3d_check(const size_t n, const vertix_3d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        const char c = v[i].ascii_char;
        is_valid &= s.ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}

// vertix 2d interpolation
// ------------------------------------------------------------------------------------------------------------

static inline void vectix_2d_lerp(vertix_2d_type res[1], const vertix_2d_type v[2], const float t0) {
    vec2_lerp(res[0].pos, v[0].pos, v[1].pos, t0);
    vec3_lerp(res[0].color.as_vec3, v[0].color.as_vec3, v[1].color.as_vec3, t0);

    const int c0 = s.ascii_to_index[(int)v[0].ascii_char];
    const int c1 = s.ascii_to_index[(int)v[1].ascii_char];
    const int c = int_lerped(c0, c1, t0);

    res[0].ascii_char = s.index_to_ascii[c];
}

// renderer init/deinit
// ------------------------------------------------------------------------------------------------------------

void renderer_init(struct screen_type* screen_context_p, const mat4x4_type model_view_matrix, const size_t n,
                   const char ascii_palette[n]) {
    if (s.initialized) {
        return;
    }
    s.initialized = true;

    s.screen_context_p = screen_context_p;
    mat4x4_copy(s.mvp, model_view_matrix);

    for (size_t i = 0; i < 128; i++) {
        s.ascii_to_index[i] = -1;
        s.index_to_ascii[i] = ' ';
    }

    for (size_t i = 0; i < n; i++) {
        assert(int_is_inside_range(ascii_palette[i], 32, 126) && "ascii char is not printable");

        const char c = ascii_palette[i];
        s.ascii_to_index[(int)c] = (int)i;
        s.index_to_ascii[i] = c;
    }

    s.vert2dstk_p = vert2dstk_create_with_initial_capacity(32);
}

void renderer_deinit(void) {
    if (!s.initialized) {
        return;
    }
    s.initialized = false;

    vert2dstk_count(s.vert2dstk_p);
}

// world space -> screen space
// ------------------------------------------------------------------------------------------------------------

static inline void vertix_2d_transform_pos_to_screen_space(const size_t n, vertix_2d_type res[n], const vertix_2d_type v[n]) {
    for (size_t i = 0; i < n; i++) {
        vec2_add(res[i].pos, v[i].pos, (vec2_type){1.f, 1.f});
        vec2_scale(res[i].pos, res[i].pos, 1.f / 2.f);
        vec2_elementwise_prod(res[i].pos, res[i].pos, (vec2_type){(float)SCREEN_WIDTH - 1.f, (float)SCREEN_HEIGHT - 1.f});
        vec2_truncated(res[i].pos, res[i].pos);
    }
}

// internal draw routines
// ------------------------------------------------------------------------------------------------------------

static inline void internal_plot_point(const vertix_2d_type v[1], const float depth0) {
    vec2int_type r_v;
    vec2_truncated_to_vec2int(r_v, v[0].pos);
    screen_set_pixel_data(s.screen_context_p, r_v,
                          (pixel_data_type){.color = v[0].color, .depth = depth0, .ascii_char = v[0].ascii_char});
}

static inline void internal_plot_line(const vertix_2d_type v[2], const float depth[2]) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    const int dx = float_truncated_to_int(float_abs(v[1].pos[0] - v[0].pos[0]));
    const int dy = float_truncated_to_int(float_abs(v[1].pos[1] - v[0].pos[1]));
    const int diagonal_dist = dx > dy ? dx : dy;

    if (diagonal_dist == 0) {
        return;
    }

    for (int step = 0.f; step <= diagonal_dist; step += 1) {
        const float t = (float)step / (float)diagonal_dist;
        const float d = float_lerped(depth[0], depth[1], t);

        vertix_2d_type p;
        vectix_2d_lerp(&p, &v[0], t);
        vec2int_type r_v;
        vec2_truncated_to_vec2int(r_v, v[0].pos);

        screen_set_pixel_data(s.screen_context_p, r_v,
                              (pixel_data_type){.color = v[0].color, .depth = d, .ascii_char = v[0].ascii_char});
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

static inline void internal_plot_triangle(const vertix_2d_type v[3], const float depth[3]) {
    // baycentric algorithm:
    // https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const int maxX = int_max(v[0].x, int_max(v[1].x, v[2].x));
    const int minX = int_min(v[0].x, int_min(v[1].x, v[2].x));
    const int maxY = int_max(v[0].y, int_max(v[1].y, v[2].y));
    const int minY = int_min(v[0].y, int_min(v[1].y, v[2].y));

    const vec2_type p0 = {(float)minX + 0.5f, (float)minY + 0.5f};

    // bias to include top left edge and exclude bottom right edge:
    const float bias0 = internal_is_top_left_edge_of_triangle(v[1].pos, v[2].pos) ? 0 : -1;
    const float bias1 = internal_is_top_left_edge_of_triangle(v[2].pos, v[0].pos) ? 0 : -1;
    const float bias2 = internal_is_top_left_edge_of_triangle(v[0].pos, v[1].pos) ? 0 : -1;

    // relevant vectors:
    vec2_type v1_to_v2, v2_to_v0, v0_to_v1, v0_to_v2, v1_to_p0, v2_to_p0, v0_to_p0;

    vec2_sub(v1_to_v2, v[2].pos, v[1].pos);
    vec2_sub(v2_to_v0, v[0].pos, v[2].pos);
    vec2_sub(v0_to_v1, v[1].pos, v[0].pos);
    vec2_sub(v0_to_v2, v[2].pos, v[0].pos);

    vec2_sub(v0_to_p0, p0, v[0].pos);
    vec2_sub(v1_to_p0, p0, v[1].pos);
    vec2_sub(v2_to_p0, p0, v[2].pos);

    const float triangle_area_2 = vec2_cross(v0_to_v1, v0_to_v2);

    // for efficient cross product calculation for each point in the bounding box. see video for derivation.
    const float delta_w0_col = v[1].y - v[2].y;
    const float delta_w0_row = v[2].x - v[1].x;
    float w0_row = vec2_cross(v1_to_v2, v1_to_p0) + bias0;

    const float delta_w1_col = v[2].y - v[0].y;
    const float delta_w1_row = v[0].x - v[2].x;
    float w1_row = vec2_cross(v2_to_v0, v2_to_p0) + bias1;

    const float delta_w2_col = v[0].y - v[1].y;
    const float delta_w2_row = v[1].x - v[0].x;
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
                vec3_scale(c_comp0.as_vec3, v[0].color.as_vec3, alpha);
                vec3_scale(c_comp1.as_vec3, v[1].color.as_vec3, beta);
                vec3_scale(c_comp2.as_vec3, v[2].color.as_vec3, gamma);
                vec3_add(c.as_vec3, c_comp0.as_vec3, c_comp1.as_vec3);
                vec3_add(c.as_vec3, c.as_vec3, c_comp2.as_vec3);

                char ch;
                float ch_comp0_idx, ch_comp1_idx, ch_comp2_idx;
                ch_comp0_idx = int_to_float(s.ascii_to_index[(int)v[0].ascii_char]);
                ch_comp1_idx = int_to_float(s.ascii_to_index[(int)v[1].ascii_char]);
                ch_comp2_idx = int_to_float(s.ascii_to_index[(int)v[1].ascii_char]);
                ch = s.index_to_ascii[float_truncated_to_int(alpha * ch_comp0_idx + beta * ch_comp1_idx + gamma * ch_comp2_idx)];

                screen_set_pixel_data(s.screen_context_p, (int[2]){x, y}, (pixel_data_type){.color = c, .depth = d, .ascii_char = ch});
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

static inline bool clip_line_2d(const vec2_type f0, const vec2_type f1, const float xmin, const float ymin, const float xmax,
                                const float ymax, float* out_t0, float* out_t1) {
    // Based on (Liang-Barsky algorithm implementation):
    // https://www.geeksforgeeks.org/liang-barsky-algorithm/

    const float x1 = f0[0];
    const float y1 = f0[1];
    const float x2 = f1[0];
    const float y2 = f1[1];

    const float dx = x2 - x1;
    const float dy = y2 - y1;

    // 0, 1, 2, 3: left, right, bottom, top
    const float p[4] = {-dx, dx, -dy, dy};
    const float q[4] = {x1 - xmin, xmax - x1, y1 - ymin, ymax - y1};

    *out_t0 = 0.0f;
    *out_t1 = 1.0f;

    for (size_t i = 0; i < 4; i++) {
        if (float_is_equal(p[i], 0.f)) { // Check if line is parallel to the clipping boundary
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
// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(const vertix_2d_type v[1], const uint8_t z_order) {
    assert(valid_vertix_2d_check(1, v));

    if (!vec2_is_inside_range(v[0].pos, (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f})) {
        return;
    }

    const float depth = (float)z_order / UINT8_MAX;

    vertix_2d_type res[1];
    vertix_2d_copy(1, res, v);
    vertix_2d_transform_pos_to_screen_space(1, res, v);

    internal_plot_point(res, depth);
}

void draw_line_2d(const vertix_2d_type v[2], const uint8_t z_order) {
    assert(valid_vertix_2d_check(2, v));

    float t0, t1;
    if (!clip_line_2d(v[0].pos, v[1].pos, -1.f, -1.f, 1.f, 1.f, &t0, &t1)) {
        return;
    }

    const float depth = (float)z_order / UINT8_MAX;

    vertix_2d_type res[2];
    vertix_2d_copy(2, res, v);

    vectix_2d_lerp(&res[0], v, t0);
    vectix_2d_lerp(&res[1], v, t1);

    vertix_2d_transform_pos_to_screen_space(2, res, res);

    internal_plot_line(res, (float[2]){depth, depth});
}

void draw_filled_triangle_2d(const vertix_2d_type v[3], const uint8_t z_order) {
    assert(valid_vertix_2d_check(3, v));

    // TODO: culling

    const float depth = (float)z_order / UINT8_MAX;

    vertix_2d_type res[3];
    vertix_2d_copy(3, res, v);
    vertix_2d_transform_pos_to_screen_space(3, res, res);
    internal_plot_triangle(res, (float[3]){depth, depth, depth});
}
