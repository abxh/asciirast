#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "draw.h"
#include "screen.h"
#include "transform.h"

int to_framebuf_x(float screen_x) {
    return (screen_x + 1.) / 2. * (SCREEN_WIDTH - 1);
}

int to_framebuf_y(float screen_y) {
    return (-screen_y + 1.) / 2. * (SCREEN_HEIGHT - 1);
}

bool inside_framebuf(int framebuf_x, int framebuf_y) {
    bool inside_framebuf_x = 0 <= framebuf_x && framebuf_x < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= framebuf_y && framebuf_y < SCREEN_HEIGHT;
    return inside_framebuf_x && inside_framebuf_y;
}

void draw_point_framebuf_coords_split(int framebuf_x, int framebuf_y, char c) {
    if (!inside_framebuf(framebuf_x, framebuf_y)) {
        return;
    }

    framebuf[framebuf_y][framebuf_x] = c;
}

void draw_point_2d(vec2 vec, char c) {
    int framebuf_x = to_framebuf_x(vec.x);
    int framebuf_y = to_framebuf_y(vec.y);

    draw_point_framebuf_coords_split(framebuf_x, framebuf_y, c);
}

void draw_line_vertical_framebuf_coords_split(int framebuf_x, int top_framebuf_y, unsigned int steps, char c) {
    if (!inside_framebuf(framebuf_x, top_framebuf_y) && !inside_framebuf(framebuf_x, top_framebuf_y + steps)) {
        return;
    }
    int lim = min_int(top_framebuf_y + steps, SCREEN_HEIGHT - 1);
    for (int current = max_int(top_framebuf_y, 0); current <= lim; current += 1) {
        framebuf[current][framebuf_x] = c;
    }
}

void draw_line_horizontal_framebuf_coords_split(int left_framebuf_x, int framebuf_y, unsigned int steps, char c) {
    if (!inside_framebuf(left_framebuf_x + steps, framebuf_y) && !inside_framebuf(left_framebuf_x + steps, framebuf_y)) {
        return;
    }
    int lim = min_int(left_framebuf_x + steps, SCREEN_WIDTH - 1);
    for (int current = max_int(left_framebuf_x, 0); current <= lim; current += 1) {
        framebuf[framebuf_y][current] = c;
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
        draw_point_framebuf_coords_split(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

void draw_nonsteep_slope_framebuf_coords_split(int x0, int y0, int x1, int y1, char c) {
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
        draw_point_framebuf_coords_split(x, y, c);
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

    unsigned int x_abs_diff = abs_int(x1 - x2);
    unsigned int y_abs_diff = abs_int(y1 - y2);

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
            draw_nonsteep_slope_framebuf_coords_split(x2, y2, x1, y1, c);
        } else {
            draw_nonsteep_slope_framebuf_coords_split(x1, y1, x2, y2, c);
        }
    }
}

void draw_line_2d(vec2 p1, vec2 p2, char c) {
    draw_line_framebuf_coords_split(to_framebuf_x(p1.x), to_framebuf_y(p1.y), to_framebuf_x(p2.x), to_framebuf_y(p2.y), c);
}

typedef struct {
    int x;
    int y;
} framebuf_coords;

void swap_framebuf_coords(framebuf_coords* v1_ptr, framebuf_coords* v2_ptr) {
    framebuf_coords t = *v1_ptr;
    *v1_ptr = *v2_ptr;
    *v2_ptr = t;
}

void draw_filled_bottom_flat_triangle_framebuf_coords(framebuf_coords v1, framebuf_coords v2, framebuf_coords v3, char c) {
    // scanline algorithm:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    if (!(v2.x < v3.x)) {
        swap_framebuf_coords(&v2, &v3);
    }

    if (v1.y == v2.y) {
        draw_line_framebuf_coords_split(v1.x, v1.y, v3.x, v3.y, c);
        return;
    } else if (v1.y == v3.y) {
        draw_line_framebuf_coords_split(v1.x, v1.y, v2.x, v2.y, c);
        return;
    }

    float change_left = (float)(v2.x - v1.x) / (float)(v2.y - v1.y);
    float change_right = (float)(v3.x - v1.x) / (float)(v3.y - v1.y);

    float current_left = v1.x;
    float current_right = v1.x;

    for (int scanlineY = v1.y; scanlineY <= v2.y; scanlineY++) {
        draw_line_horizontal_framebuf_coords_split(round_float_to_int(current_left), scanlineY,
                                             round_float_to_int(current_right - current_left), c);
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

    if (v3.y == v1.y || v3.y == v2.y) {
        draw_line_framebuf_coords_split(v1.x, v1.y, v2.x, v2.y, c);
        return;
    }

    float change_left = (float)(v3.x - v1.x) / (float)(v3.y - v1.y);
    float change_right = (float)(v3.x - v2.x) / (float)(v3.y - v2.y);

    float current_left = v3.x;
    float current_right = v3.x;

    for (int scanlineY = v3.y; scanlineY > v1.y; scanlineY--) {
        draw_line_horizontal_framebuf_coords_split(round_float_to_int(current_left), scanlineY,
                                             round_float_to_int(current_right - current_left), c);
        current_left -= change_left;
        current_right -= change_right;
    }
}

void draw_filled_triangle_framebuf_coords(framebuf_coords v1, framebuf_coords v2, framebuf_coords v3, char c) {
    // scanline algorithm:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    /* sort points and ensure v1.y <= v2.y <= v3.y */
    if (v1.y > v2.y) {
        swap_framebuf_coords(&v1, &v2);
    }
    if (v1.y > v3.y) {
        swap_framebuf_coords(&v1, &v2);
    }
    if (v2.y > v3.y) {
        swap_framebuf_coords(&v2, &v3);
    }

    if (v2.y == v3.y) {
        draw_filled_bottom_flat_triangle_framebuf_coords(v1, v2, v3, c);
    } else if (v1.y == v2.y) {
        draw_filled_top_flat_triangle_framebuf_coords(v1, v2, v3, c);
    } else {
        framebuf_coords v = {round_float_to_int((float)v1.x + ((float)(v2.y - v1.y) / (float)(v3.y - v1.y)) * (float)(v3.x - v1.x)),
                             v2.y};

        draw_filled_bottom_flat_triangle_framebuf_coords(v1, v2, v, c);
        draw_filled_top_flat_triangle_framebuf_coords(v2, v, v3, c);
    }
}

framebuf_coords to_framebuf_coords(vec2 v) {
    return (framebuf_coords){.x = to_framebuf_x(v.x), .y = to_framebuf_y(v.y)};
}

void draw_filled_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c) {
    draw_filled_triangle_framebuf_coords(to_framebuf_coords(p1), to_framebuf_coords(p2), to_framebuf_coords(p3), c);
}

void draw_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c) {
    framebuf_coords v1 = to_framebuf_coords(p1);
    framebuf_coords v2 = to_framebuf_coords(p2);
    framebuf_coords v3 = to_framebuf_coords(p3);

    /* sort points and ensure v1.y <= v2.y <= v3.y */
    if (v1.y > v2.y) {
        swap_framebuf_coords(&v1, &v2);
    }
    if (v1.y > v3.y) {
        swap_framebuf_coords(&v1, &v2);
    }
    if (v2.y > v3.y) {
        swap_framebuf_coords(&v2, &v3);
    }

    draw_line_framebuf_coords_split(v1.x, v1.y, v2.x, v2.y, c);
    draw_line_framebuf_coords_split(v2.x, v2.y, v3.x, v3.y, c);
    draw_line_framebuf_coords_split(v3.x, v3.y, v1.x, v1.y, c);
}
