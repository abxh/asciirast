#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "draw.h"
#include "screen.h"
#include "transform.h"

typedef struct {
    int x;
    int y;
} vec2int;

void plot_int2(int screen_x, int screen_y, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);

    int inv_screen_y = SCREEN_HEIGHT - 1 - screen_y;
    assert(0 <= inv_screen_y && inv_screen_y < SCREEN_HEIGHT);

    screen_buf[inv_screen_y][screen_x] = c;
}

void plot_vec2int(vec2int sv, char c) {
    plot_int2(sv.x, sv.y, c);
}

int conv_to_screen_index_unclamped_float(float x, int length) {
    return ((x + 1) / 2) * (length - 1);
}

int conv_to_screen_index_clamped_float(float x, int length) {
    return conv_to_screen_index_unclamped_float(clamp_float(x, -1., 1.), length);
}

vec2int conv_to_screen_indicies_unclamped_vec2(vec2 vec) {
    return (vec2int){conv_to_screen_index_unclamped_float(vec.x, SCREEN_WIDTH),
                     conv_to_screen_index_unclamped_float(vec.y, SCREEN_HEIGHT)};
}

vec2int conv_to_screen_indicies_clamped_vec2(vec2 vec) {
    return (vec2int){conv_to_screen_index_clamped_float(vec.x, SCREEN_WIDTH),
                     conv_to_screen_index_clamped_float(vec.y, SCREEN_HEIGHT)};
}

void draw_point_vec2(vec2 vec, char c) {
    vec2int screen_indicies = conv_to_screen_indicies_unclamped_vec2(vec);

    bool inside_x_bounds = 0 <= screen_indicies.x && screen_indicies.x < SCREEN_WIDTH;
    bool inside_y_bounds = 0 <= screen_indicies.y && screen_indicies.y < SCREEN_HEIGHT;

    if (!(inside_x_bounds || inside_y_bounds)) {
        return;
    }
    plot_vec2int(screen_indicies, c);
}

void draw_line_vertical_int2(int screen_x, int screen_y, unsigned int steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_HEIGHT);
    assert(screen_y + steps < SCREEN_HEIGHT);

    for (int var = screen_y; var <= screen_y + (int)steps; var += 1) {
        plot_int2(screen_x, var, c);
    }
}

void draw_line_horizontal_int2(int screen_x, int screen_y, unsigned int steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_HEIGHT);
    assert(screen_x + steps < SCREEN_WIDTH);

    for (int var = screen_x; var <= screen_x + (int)steps; var += 1) {
        plot_int2(var, screen_y, c);
    }
}

void draw_line_pos_slope_int4(int x0, int y0, int x1, int y1, char c) {
    // Bresenham line algorithm:
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
        plot_int2(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

void draw_line_neg_slope_int4(int x0, int y0, int x1, int y1, char c) {
    // Bresenham line algorithm:
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
        plot_int2(x, y, c);
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2 * dx;
        }
    }
}

void draw_line_int4(int x1, int y1, int x2, int y2, char c) {
    // Bresenham line algorithm:
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases

    unsigned int x_abs_diff = abs_int(diff_int(x1, x2));
    unsigned int y_abs_diff = abs_int(diff_int(y1, y2));

    if (x1 == x2) {
        if (y1 < y2) {
            draw_line_vertical_int2(x1, y1, y_abs_diff, c);
        } else {
            draw_line_vertical_int2(x2, y2, y_abs_diff, c);
        }
    } else if (y1 == y2) {
        if (x1 < x2) {
            draw_line_horizontal_int2(x1, y1, x_abs_diff, c);
        } else {
            draw_line_horizontal_int2(x2, y2, x_abs_diff, c);
        }
    }

    if (y_abs_diff < x_abs_diff) {
        if (x1 > x2) {
            draw_line_pos_slope_int4(x2, y2, x1, y1, c);
        } else {
            draw_line_pos_slope_int4(x1, y1, x2, y2, c);
        }
    } else {
        if (y1 > y2) {
            draw_line_neg_slope_int4(x2, y2, x1, y1, c);
        } else {
            draw_line_neg_slope_int4(x1, y1, x2, y2, c);
        }
    }
}

void draw_line_vec2int(vec2int sp1, vec2int sp2, char c) {
    draw_line_int4(sp1.x, sp1.y, sp2.x, sp2.y, c);
}

void draw_line_vec2(vec2 p1, vec2 p2, char c) {
    vec2int sp1 = conv_to_screen_indicies_clamped_vec2(p1);
    vec2int sp2 = conv_to_screen_indicies_clamped_vec2(p2);

    draw_line_vec2int(sp1, sp2, c);
}
