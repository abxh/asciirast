#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "draw.h"
#include "screen.h"
#include "transform.h"

void draw_point_2d(vec2 vec, char c) {
    int framebuf_x = to_framebuf_x(vec.x);
    int framebuf_y = to_framebuf_y(vec.y);

    draw_framebuf_point_w_bounds_checking(framebuf_x, framebuf_y, c);
}

void draw_line_vertical_framebuf_coords_split(int framebuf_x, int top_framebuf_y, size_t steps, char c) {
    for (size_t i = 0; i < steps; i += 1) {
        draw_framebuf_point_w_no_bounds_checking(framebuf_x, top_framebuf_y + i, c);
    }
}

void draw_line_horizontal_framebuf_coords_split(int left_framebuf_x, int framebuf_y, size_t steps, char c) {
    for (size_t i = 0; i < steps; i += 1) {
        draw_framebuf_point_w_no_bounds_checking(left_framebuf_x + i, framebuf_y, c);
    }
}

void draw_line_steep_framebuf_coords_split(int x0, int y0, int x1, int y1, char c) {
    // Bresenham line algorithm - first case:
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases

    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int D = (2 * dy) - dx;
    int y = y0;

    for (int x = x0; x <= x1; x++) {
        draw_framebuf_point_w_bounds_checking(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

void draw_line_nonsteep_framebuf_coords_split(int x0, int y0, int x1, int y1, char c) {
    // Bresenham line algorithm - second case:
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases

    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int D = (2 * dx) - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++) {
        draw_framebuf_point_w_bounds_checking(x, y, c);
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2 * dx;
        }
    }
}

void draw_line_framebuf_coords_split(int x1, int y1, int x2, int y2, char c) {
    // Bresenham line algorithm - all cases:
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases

    if (!inside_framebuf(x1, y1) && !inside_framebuf(x2, y2)) {
        return;
    }

    x1 = clamp_int(x1, 0, SCREEN_WIDTH - 1);
    x2 = clamp_int(x2, 0, SCREEN_WIDTH - 1);
    y1 = clamp_int(y1, 0, SCREEN_HEIGHT - 1);
    y2 = clamp_int(y2, 0, SCREEN_HEIGHT - 1);

    int x_abs_diff = abs_int(x1 - x2);
    int y_abs_diff = abs_int(y1 - y2);

    if (x1 == x2) {
        if (y1 < y2) {
            draw_line_vertical_framebuf_coords_split(x1, y1, y_abs_diff, c);
        } else {
            draw_line_vertical_framebuf_coords_split(x2, y2, y_abs_diff, c);
        }
    } else if (y1 == y2) {
        if (x1 < x2) {
            draw_line_horizontal_framebuf_coords_split(x1, y1, x_abs_diff, c);
        } else {
            draw_line_horizontal_framebuf_coords_split(x2, y2, x_abs_diff, c);
        }
    }

    bool steep_slope = y_abs_diff < x_abs_diff;
    if (steep_slope) {
        if (x1 > x2) {
            draw_line_steep_framebuf_coords_split(x2, y2, x1, y1, c);
        } else {
            draw_line_steep_framebuf_coords_split(x1, y1, x2, y2, c);
        }
    } else {
        if (y1 > y2) {
            draw_line_nonsteep_framebuf_coords_split(x2, y2, x1, y1, c);
        } else {
            draw_line_nonsteep_framebuf_coords_split(x1, y1, x2, y2, c);
        }
    }
}

void draw_line_framebuf_coords(framebuf_coords v1, framebuf_coords v2, char c) {
    draw_line_framebuf_coords_split(v1.x, v1.y, v2.x, v2.y, c);
}

void draw_line_2d(vec2 p1, vec2 p2, char c) {
    draw_line_framebuf_coords_split(to_framebuf_x(p1.x), to_framebuf_y(p1.y), to_framebuf_x(p2.x), to_framebuf_y(p2.y), c);
}

void draw_line_horizontal_framebuf_coords_split_w_bounds_checking(int left_framebuf_x, int framebuf_y, size_t steps, char c) {
    if (!inside_framebuf(left_framebuf_x + steps, framebuf_y) && !inside_framebuf(left_framebuf_x + steps, framebuf_y)) {
        return;
    }
    int lim = min_int(left_framebuf_x + steps, SCREEN_WIDTH - 1);
    for (int current = max_int(left_framebuf_x, 0); current < lim; current += 1) {
        if (!(0 <= framebuf_y && framebuf_y < SCREEN_HEIGHT)) {
            continue;
        }
        draw_framebuf_point_w_no_bounds_checking(current, framebuf_y, c);
    }
}

void draw_filled_bottom_flat_triangle_framebuf_coords(framebuf_coords v1, framebuf_coords v2, framebuf_coords v3, char c) {
    // scanline algorithm:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    if (!(v2.x < v3.x)) {
        swap_framebuf_coords(&v2, &v3);
    }

    float change_left = (v2.x - v1.x) / (float)(v2.y - v1.y);
    float change_right = (v3.x - v1.x) / (float)(v3.y - v1.y);

    float current_left = v1.x;
    float current_right = v1.x;

    for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++) {
        draw_line_horizontal_framebuf_coords_split_w_bounds_checking(current_left, scanlineY, current_right - current_left, c);
        current_left += change_left;
        current_right += change_right;
    }
}

void draw_filled_top_flat_triangle_framebuf_coords(framebuf_coords v1, framebuf_coords v2, framebuf_coords v3, char c) {
    // scanline algorithm:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    if (!(v1.x < v2.x)) {
        swap_framebuf_coords(&v1, &v2);
    }

    float change_left = (v3.x - v1.x) / (float)(v3.y - v1.y);
    float change_right = (v3.x - v2.x) / (float)(v3.y - v2.y);

    float current_left = v3.x;
    float current_right = v3.x;

    for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--) {
        draw_line_horizontal_framebuf_coords_split_w_bounds_checking(current_left, scanlineY, current_right - current_left, c);
        current_left -= change_left;
        current_right -= change_right;
    }
}

void draw_filled_triangle_framebuf_coords(framebuf_coords v1, framebuf_coords v2, framebuf_coords v3, char c) {
    // scanline algorithm:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    /* sort points and ensure v1.y <= v2.y <= v3.y */
    if (v2.y < v1.y) {
        swap_framebuf_coords(&v2, &v1);
    }
    if (v3.y < v1.y) {
        swap_framebuf_coords(&v3, &v1);
    }
    if (v3.y < v2.y) {
        swap_framebuf_coords(&v3, &v2);
    }

    if (v1.y == v2.y && v2.y == v3.y) {
        draw_framebuf_point_w_bounds_checking(v1.x, v1.y, c);
    } else if (v2.y == v3.y) {
        draw_filled_bottom_flat_triangle_framebuf_coords(v1, v2, v3, c);
    } else if (v1.y == v2.y) {
        draw_filled_top_flat_triangle_framebuf_coords(v1, v2, v3, c);
    } else {
        framebuf_coords v = {.x = v1.x + ((v2.y - v1.y) / (float)(v3.y - v1.y)) * (v3.x - v1.x), .y = v2.y};
        draw_filled_bottom_flat_triangle_framebuf_coords(v1, v2, v, c);
        draw_filled_top_flat_triangle_framebuf_coords(v2, v, v3, c);
    }
}

void draw_filled_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c) {
    draw_filled_triangle_framebuf_coords(to_framebuf_coords(p1), to_framebuf_coords(p2), to_framebuf_coords(p3), c);
}

void draw_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c) {
    framebuf_coords v1 = to_framebuf_coords(p1);
    framebuf_coords v2 = to_framebuf_coords(p2);
    framebuf_coords v3 = to_framebuf_coords(p3);

    draw_line_framebuf_coords(v1, v2, c);
    draw_line_framebuf_coords(v2, v3, c);
    draw_line_framebuf_coords(v3, v1, c);
}

static float fov_angle_rad = M_PI / 3; // 60 degrees

void draw_filled_triangle_3d(vec3 p1, vec3 p2, vec3 p3, char c) {
    vec3 p1_to_p3 = sum_vec3(p3, scaled_vec3(p1, -1));
    vec3 p1_to_p2 = sum_vec3(p2, scaled_vec3(p1, -1));

    vec3 triangle_normal = cross_vec3(p1_to_p3, p1_to_p2);

    // backface culling:
    if (dot_vec3(p1, triangle_normal) >= 0) {
        return;
    }

    // transform to screen space:
    vec2 p1_2d = vec3_projected_as_vec2(p1, fov_angle_rad, aspect_ratio);
    vec2 p2_2d = vec3_projected_as_vec2(p2, fov_angle_rad, aspect_ratio);
    vec2 p3_2d = vec3_projected_as_vec2(p3, fov_angle_rad, aspect_ratio);

    draw_filled_triangle_2d(p1_2d, p2_2d, p3_2d, c);
}
