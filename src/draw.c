#include "draw.h"
#include "screen.h"
#include "transform.h"

vec3_type g_camera_position = {0.f, 0.f, 0.f};

camera_orientation_type g_camera_orientation = {.x_axis_angle_rad = 0.f, .y_axis_angle_rad = 0.f, .z_axis_angle_rad = 0.f};

static inline vec2int_type from_screen_pos_to_framebuf_pos(const vec2_type v0) {
    const float framebuf_x = (v0.x + 1.f) / 2.f * (FRAMEBUF_WIDTH - 1);
    const float framebuf_y = (v0.y + 1.f) / 2.f * (FRAMEBUF_HEIGHT - 1);

    return (vec2int_type){.x = from_float_to_int_rounded(framebuf_x), .y = from_float_to_int_rounded(framebuf_y)};
}

static inline void internal_draw_line_routine(int x0, int y0, int x1, int y1, char c, float d0, float d1, color_type c0,
                                              color_type c1) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    const int dx = abs_int(x1 - x0);
    const int dy = abs_int(y1 - y0);
    const int diagonal_dist = dx > dy ? dx : dy;

    if (diagonal_dist == 0) {
        return;
    }

    const vec2_type v0 = {from_int_to_float(x0), from_int_to_float(y0)};
    const vec2_type v1 = {from_int_to_float(x1), from_int_to_float(y1)};

    for (int step = 0; step <= diagonal_dist; step++) {
        const float t = from_int_to_float(step) / from_int_to_float(diagonal_dist);
        const vec2_type p = lerp_vec2(v0, v1, t);

        const vec2int_type p_int = from_vec2_to_vec2int_rounded(p);
        const color_type color = lerp_color(c0, c1, t);
        const float depth = lerp_float(d0, d1, t);

        screen_set_pixel_data(p_int, (pixel_data_type){.ascii_char = c, .color = color, .depth = depth});
    }
}

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(vec2_type v0, char c) {
    const vec2int_type f0 = from_screen_pos_to_framebuf_pos(v0);

    const vec2int_type zero = {0, 0};
    const vec2int_type border_exclusive = {FRAMEBUF_WIDTH - 1, FRAMEBUF_HEIGHT - 1};

    if (!inside_range_vec2int(f0, zero, border_exclusive)) {
        return;
    }

    screen_set_pixel_data(f0, (pixel_data_type){.ascii_char = c, .color = DEFAULT_COLOR, .depth = 1.f});
}

void draw_point_2d_w_color(vec2_type v0, color_type color0, char c) {
    const vec2int_type f0 = from_screen_pos_to_framebuf_pos(v0);

    const vec2int_type zero = {0, 0};
    const vec2int_type border_exclusive = {FRAMEBUF_WIDTH - 1, FRAMEBUF_HEIGHT - 1};

    if (!inside_range_vec2int(f0, zero, border_exclusive)) {
        return;
    }

    screen_set_pixel_data(f0, (pixel_data_type){.ascii_char = c, .color = color0, .depth = 1.f});
}

void draw_line_2d(vec2_type v0, vec2_type v1, char c) {
    const vec2int_type f0 = from_screen_pos_to_framebuf_pos(v0);
    const vec2int_type f1 = from_screen_pos_to_framebuf_pos(v1);

    // TODO: framebuf check

    internal_draw_line_routine(f0.x, f0.y, f1.x, f1.y, c, 1.f, 1.f, DEFAULT_COLOR, DEFAULT_COLOR);
}

void draw_line_2d_w_color(vec2_type v0, vec2_type v1, color_type color0, char c) {
    const vec2int_type f0 = from_screen_pos_to_framebuf_pos(v0);
    const vec2int_type f1 = from_screen_pos_to_framebuf_pos(v1);

    // TODO: framebuf check

    internal_draw_line_routine(f0.x, f0.y, f1.x, f1.y, c, 1.f, 1.f, color0, color0);
}

void draw_line_2d_w_interpolated_color(vec2_type v[2], color_type color[2], char c) {
    const vec2int_type f0 = from_screen_pos_to_framebuf_pos(v[0]);
    const vec2int_type f1 = from_screen_pos_to_framebuf_pos(v[1]);

    // TODO: framebuf check

    internal_draw_line_routine(f0.x, f0.y, f1.x, f1.y, c, 1.f, 1.f, color[0], color[1]);
}

// 3d
// ------------------------------------------------------------------------------------------------------------


