#include "draw.h"
#include "screen.h"
#include "transform.h"

#include "draw_clipping.h"

vec3_type g_camera_position = {0.f, 0.f, 0.f};

camera_orientation_type g_camera_orientation = {0.f, 0.f, 0.f};

static inline vec2_type from_screen_pos_to_framebuf_pos(const vec2_type v0) {
    const float framebuf_x = (v0.x + 1.f) / 2.f * (FRAMEBUF_WIDTH - 1);
    const float framebuf_y = (v0.y + 1.f) / 2.f * (FRAMEBUF_HEIGHT - 1);

    return (vec2_type){.x = framebuf_x, .y = framebuf_y};
}

static inline void internal_plot_line_routine(const vec2int_type v0, const vec2int_type v1, const char c, const float d0,
                                              const float d1, const color_type c0, const color_type c1) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    const int dx = abs_int(v1.x - v0.x);
    const int dy = abs_int(v1.y - v0.y);
    const int diagonal_dist = dx > dy ? dx : dy;

    if (diagonal_dist == 0) {
        return;
    }

    const vec2_type v0_f = from_vec2int_to_vec2(v0);
    const vec2_type v1_f = from_vec2int_to_vec2(v1);

    for (int step = 0; step <= diagonal_dist; step++) {
        const float t = from_int_to_float(step) / from_int_to_float(diagonal_dist);
        const vec2_type p = lerp_vec2(v0_f, v1_f, t);

        const vec2int_type p_int = from_vec2_to_vec2int_rounded(p);
        const color_type color = lerp_color(c0, c1, t);
        const float depth = lerp_float(d0, d1, t);

        screen_set_pixel_data(p_int, (pixel_data_type){.ascii_char = c, .color = color, .depth = depth});
    }
}

static inline bool is_top_left_edge_of_triangle(const vec2int_type src, const vec2int_type dest) {
    const vec2int_type edge = src_to_dest_vec2int(src, dest);

    const bool points_right = edge.x > 0;
    const bool points_up = edge.y < 0; // since y-axis points down for framebuffer

    const bool is_top_edge = edge.y == 0 && points_right;
    const bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

static inline void internal_plot_triangle_routine(const vec2int_type v0, const vec2int_type v1, const vec2int_type v2, const char c,
                                                  const float d0, const float d1, const float d2, const color_type c0,
                                                  const color_type c1, const color_type c2) {
    // baycentric algorithm:
    // https://www.youtube.com/watch?v=k5wtuKWmV48

    // get the bounding box of the triangle
    const int maxX = max_int(v0.x, max_int(v1.x, v2.x));
    const int minX = min_int(v0.x, min_int(v1.x, v2.x));
    const int maxY = max_int(v0.y, max_int(v1.y, v2.y));
    const int minY = min_int(v0.y, min_int(v1.y, v2.y));

    const vec2_type p0 = sum_vec2(from_vec2int_to_vec2(v0), (vec2_type){.x = 0.5f, .y = 0.5f});

    // bias to include top left edge and *not* bottom right edge
    const float bias0 = is_top_left_edge_of_triangle(v1, v2) ? 0 : -1;
    const float bias1 = is_top_left_edge_of_triangle(v2, v0) ? 0 : -1;
    const float bias2 = is_top_left_edge_of_triangle(v0, v1) ? 0 : -1;

    // useful vectors:
    const vec2_type v1_to_v2 = from_vec2int_to_vec2(src_to_dest_vec2int(v1, v2));
    const vec2_type v2_to_v0 = from_vec2int_to_vec2(src_to_dest_vec2int(v2, v0));
    const vec2_type v0_to_v1 = from_vec2int_to_vec2(src_to_dest_vec2int(v0, v1));
    const vec2_type v0_to_v2 = from_vec2int_to_vec2(src_to_dest_vec2int(v0, v2));

    const float triangle_area_2 = cross_vec2(v0_to_v1, v0_to_v2);

    // for efficient calculaton of cross product at each point in the bounding box - see video for details
    const float delta_w0_col = from_int_to_float(v1.y - v2.y);
    const float delta_w0_row = from_int_to_float(v2.x - v1.x);
    const float delta_w1_col = from_int_to_float(v2.y - v0.y);
    const float delta_w1_row = from_int_to_float(v0.x - v2.x);
    const float delta_w2_col = from_int_to_float(v0.y - v1.y);
    const float delta_w2_row = from_int_to_float(v1.x - v0.x);

    // cross product value at starting point
    float w0_row = cross_vec2(v1_to_v2, src_to_dest_vec2(from_vec2int_to_vec2(v1), p0)) + bias0;
    float w1_row = cross_vec2(v2_to_v0, src_to_dest_vec2(from_vec2int_to_vec2(v2), p0)) + bias1;
    float w2_row = cross_vec2(v0_to_v1, src_to_dest_vec2(from_vec2int_to_vec2(v0), p0)) + bias2;

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

                const color_type color0 = scaled_color(c0, alpha);
                const color_type color1 = scaled_color(c1, beta);
                const color_type color2 = scaled_color(c2, gamma);
                const color_type color = sum_color(sum_color(color0, color1), color2);

                const float depth = alpha * d0 + beta * d1 + gamma * d2;

                screen_set_pixel_data((vec2int_type){.x = x, .y = y},
                                      (pixel_data_type){.ascii_char = c, .color = color, .depth = depth});
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

// 2d - draw point
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d_w_color_and_z_order(const vec2_type v0, const color_type color0, const uint8_t z_order, const char c) {
    const vec2_type f0 = from_screen_pos_to_framebuf_pos(v0);

    if (!clip_point_2d(f0, 0.f, 0.f, FRAMEBUF_WIDTH - 1.f, FRAMEBUF_HEIGHT - 1.f)) {
        return;
    }

    const vec2int_type f0r = from_vec2_to_vec2int_truncated(f0);

    const float depth0 = from_int_to_float(z_order) / UINT8_MAX;

    screen_set_pixel_data(f0r, (pixel_data_type){.ascii_char = c, .color = color0, .depth = depth0});
}

void draw_point_2d_w_color(const vec2_type v0, const color_type color0, const char c) {
    draw_point_2d_w_color_and_z_order(v0, color0, 0, c);
}

void draw_point_2d(const vec2_type v0, const char c) {
    draw_point_2d_w_color_and_z_order(v0, DEFAULT_COLOR, 0, c);
}

// 2d - draw line
// ------------------------------------------------------------------------------------------------------------

void draw_line_2d_w_interpolated_color_and_z_order(const vec2_type v[2], const color_type color[2], const uint8_t z_order,
                                                   const char c) {
    const vec2_type f0 = from_screen_pos_to_framebuf_pos(v[0]);
    const vec2_type f1 = from_screen_pos_to_framebuf_pos(v[1]);

    float t0 = 0.f, t1 = 1.f;
    if (!clip_line_2d(f0, f1, 0.f, 0.f, FRAMEBUF_WIDTH - 1.f, FRAMEBUF_HEIGHT - 1.f, &t0, &t1)) {
        return;
    }

    const vec2_type f0_new = lerp_vec2(f0, f1, t0);
    const vec2_type f1_new = lerp_vec2(f0, f1, t1);

    const color_type color0_new = lerp_color(color[0], color[1], t0);
    const color_type color1_new = lerp_color(color[0], color[1], t1);

    const vec2int_type f0r = from_vec2_to_vec2int_truncated(f0_new);
    const vec2int_type f1r = from_vec2_to_vec2int_truncated(f1_new);

    const float depth0 = from_int_to_float(z_order) / UINT8_MAX;

    internal_plot_line_routine(f0r, f1r, c, depth0, depth0, color0_new, color1_new);
}

void draw_line_2d_w_interpolated_color(const vec2_type v[2], const color_type color[2], const char c) {
    draw_line_2d_w_interpolated_color_and_z_order((const vec2_type[2]){v[0], v[1]}, (const color_type[2]){color[0], color[1]}, 0, c);
}

void draw_line_2d_w_color_and_z_order(const vec2_type v0, const vec2_type v1, const color_type color0, const uint8_t z_order,
                                      const char c) {
    draw_line_2d_w_interpolated_color_and_z_order((const vec2_type[2]){v0, v1}, (const color_type[2]){color0, color0}, z_order, c);
}

void draw_line_2d_w_color(const vec2_type v0, const vec2_type v1, const color_type color0, const char c) {
    draw_line_2d_w_interpolated_color_and_z_order((const vec2_type[2]){v0, v1}, (const color_type[2]){color0, color0}, 0, c);
}

void draw_line_2d(const vec2_type v0, const vec2_type v1, const char c) {
    draw_line_2d_w_interpolated_color_and_z_order((const vec2_type[2]){v0, v1}, (const color_type[2]){DEFAULT_COLOR, DEFAULT_COLOR}, 0,
                                                  c);
}

// 2d - draw triangle
// ------------------------------------------------------------------------------------------------------------

void draw_filled_triangle_2d_w_interpolated_color_and_z_order(const vec2_type v[3], const color_type color[3], const uint8_t z_order,
                                                              const char c) {
    const vec2_type f0 = from_screen_pos_to_framebuf_pos(v[0]);
    const vec2_type f1 = from_screen_pos_to_framebuf_pos(v[1]);
    const vec2_type f2 = from_screen_pos_to_framebuf_pos(v[2]);

    // TODO: framebuf check

    const vec2int_type f0r = from_vec2_to_vec2int_truncated(f0);
    const vec2int_type f1r = from_vec2_to_vec2int_truncated(f1);
    const vec2int_type f2r = from_vec2_to_vec2int_truncated(f2);

    const float depth0 = from_int_to_float(z_order) / UINT8_MAX;

    internal_plot_triangle_routine(f0r, f1r, f2r, c, depth0, depth0, depth0, color[0], color[1], color[2]);
}

void draw_filled_triangle_2d_w_interpolated_color(const vec2_type v[3], const color_type color[3], const char c) {
    draw_filled_triangle_2d_w_interpolated_color_and_z_order((const vec2_type[3]){v[0], v[1], v[2]},
                                                             (const color_type[3]){color[0], color[1], color[2]}, 0, c);
}

void draw_filled_triangle_2d_w_color_and_z_order(const vec2_type v0, const vec2_type v1, const vec2_type v2, const color_type color0,
                                                 const uint8_t z_order, const char c) {
    draw_filled_triangle_2d_w_interpolated_color_and_z_order((const vec2_type[3]){v0, v1, v2},
                                                             (const color_type[3]){color0, color0, color0}, z_order, c);
}

void draw_filled_triangle_2d_w_color(const vec2_type v0, const vec2_type v1, const vec2_type v2, const color_type color0,
                                     const char c) {
    draw_filled_triangle_2d_w_interpolated_color_and_z_order((const vec2_type[3]){v0, v1, v2},
                                                             (const color_type[3]){color0, color0, color0}, 0, c);
}

void draw_filled_triangle_2d(const vec2_type v0, const vec2_type v1, const vec2_type v2, const char c) {
    draw_filled_triangle_2d_w_interpolated_color_and_z_order((const vec2_type[3]){v0, v1, v2},
                                                             (const color_type[3]){DEFAULT_COLOR, DEFAULT_COLOR, DEFAULT_COLOR}, 0, c);
}

// 3d - draw point
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d_w_color(const vec3_type v0, const color_type color0, const char c) {
    const vec3_type c0 = move_against_camera(v0, g_camera_position, (float*)&g_camera_orientation);

    const vec4_type p0 =
        project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(c0, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);

    if (!inside_range_float(p0.z, Z_NEAR, Z_FAR)) {
        return;
    }

    const vec4_type p0z = scaled_vec4(p0, 1.f / p0.w);

    const vec2_type f0 = from_screen_pos_to_framebuf_pos((vec2_type){.x = p0z.x, .y = p0z.y});

    if (!clip_point_2d(f0, 0.f, 0.f, FRAMEBUF_WIDTH - 1.f, FRAMEBUF_HEIGHT - 1.f)) {
        return;
    }

    const vec2int_type f0r = from_vec2_to_vec2int_truncated(f0);

    screen_set_pixel_data(f0r, (pixel_data_type){.ascii_char = c, .color = color0, .depth = p0z.z});
}

void draw_point_3d(const vec3_type v0, const char c) {
    draw_point_3d_w_color(v0, DEFAULT_COLOR, c);
}

// 3d - draw line
// ------------------------------------------------------------------------------------------------------------

void draw_line_3d_w_interpolated_color(const vec3_type v[2], const color_type color[2], const char c) {
    const vec3_type c0 = move_against_camera(v[0], g_camera_position, (float*)&g_camera_orientation);
    const vec3_type c1 = move_against_camera(v[1], g_camera_position, (float*)&g_camera_orientation);

    const vec4_type p0 =
        project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(c0, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);
    const vec4_type p1 =
        project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(c1, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);

    // TODO:
    // clip against z_near plane


    const vec4_type p0z = scaled_vec4(p0, 1.f / p0.w);
    const vec4_type p1z = scaled_vec4(p1, 1.f / p1.w);

    const vec2_type f0 = from_screen_pos_to_framebuf_pos((vec2_type){.x = p0z.x, .y = p0z.y});
    const vec2_type f1 = from_screen_pos_to_framebuf_pos((vec2_type){.x = p1z.x, .y = p1z.y});

    float t0 = 0.f, t1 = 1.f;
    if (!clip_line_2d(f0, f1, 0.f, 0.f, FRAMEBUF_WIDTH - 1.f, FRAMEBUF_HEIGHT - 1.f, &t0, &t1)) {
        return;
    }

    const vec2_type f0_new = lerp_vec2(f0, f1, t0);
    const vec2_type f1_new = lerp_vec2(f0, f1, t1);

    const color_type color0_new = lerp_color(color[0], color[1], t0);
    const color_type color1_new = lerp_color(color[0], color[1], t1);

    const float depth0_new = lerp_float(p0z.z, p1z.z, t0);
    const float depth1_new = lerp_float(p0z.z, p1z.z, t1);

    const vec2int_type f0r = from_vec2_to_vec2int_truncated(f0_new);
    const vec2int_type f1r = from_vec2_to_vec2int_truncated(f1_new);

    internal_plot_line_routine(f0r, f1r, c, depth0_new, depth1_new, color0_new, color1_new);
}

void draw_line_3d_w_color(const vec3_type v0, const vec3_type v1, const color_type color0, const char c) {
    draw_line_3d_w_interpolated_color((const vec3_type[2]){v0, v1}, (const color_type[3]){color0, color0}, c);
}

void draw_line_3d(const vec3_type v0, const vec3_type v1, const char c) {
    draw_line_3d_w_interpolated_color((const vec3_type[2]){v0, v1}, (const color_type[3]){DEFAULT_COLOR, DEFAULT_COLOR}, c);
}
