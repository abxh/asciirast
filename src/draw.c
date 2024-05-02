#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "draw.h"
#include "misc.h"
#include "screen.h"
#include "transform.h"

void plot(size_t screen_x, size_t screen_y, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    screen_buf[screen_y][screen_x] = c;
}

int conv_to_index(float x, size_t length) {
    return (clamp(x, -1., 1.) + 1.) / 2 * (length - 1);
}

void draw_point_2d(vec2 vec, char c) {
    size_t screen_x = conv_to_index(vec.x, SCREEN_WIDTH);
    size_t screen_y = conv_to_index(vec.y, SCREEN_HEIGHT);

    plot(screen_x, screen_y, c);
}

void draw_line_vert_2d_internal(size_t screen_x, size_t screen_y, int64_t steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    assert(IMPLIES(steps >= 0, screen_y + steps < SCREEN_HEIGHT));
    assert(IMPLIES(steps < 0, (int64_t)screen_y + steps >= 0));

    if (steps >= 0) {
        for (size_t var = screen_y; var <= screen_y + steps; var += 1) {
            plot(screen_x, var, c);
        }
    } else {
        for (size_t var = screen_y; var + 1 > screen_y + steps; var -= 1) {
            plot(screen_x, var, c);
        }
    }
}

void draw_line_hort_2d_internal(size_t screen_x, size_t screen_y, int64_t steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    assert(IMPLIES(steps >= 0, screen_x + steps < SCREEN_WIDTH));
    assert(IMPLIES(steps < 0, (int64_t)screen_x + steps >= 0));

    if (steps >= 0) {
        for (size_t var = screen_x; var <= screen_x + steps; var += 1) {
            plot(var, screen_y, c);
        }
    } else {
        for (size_t var = screen_x; var + 1 > screen_x + steps; var -= 1) {
            plot(var, screen_y, c);
        }
    }
}

void plotLineLow(int64_t x0, int64_t y0, int64_t x1, int64_t y1, char c) {
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    int64_t dx = x1 - x0;
    int64_t dy = y1 - y0;
    int64_t yi = 1;
    if (dy < 0) {
        yi = -1;
        dy = -dy;
    }
    int64_t D = (2 * dy) - dx;
    int64_t y = y0;

    for (int64_t x = x0; x <= x1; x++) {
        plot(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

void plotLineHigh(int64_t x0, int64_t y0, int64_t x1, int64_t y1, char c) {
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    int64_t dx = x1 - x0;
    int64_t dy = y1 - y0;
    int64_t xi = 1;
    if (dx < 0) {
        xi = -1;
        dx = -dx;
    }
    int64_t D = (2 * dx) - dy;
    int64_t x = x0;

    for (int64_t y = y0; y <= y1; y++) {
        plot(x, y, c);
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2 * dx;
        }
    }
}

int64_t diff_int64(int64_t x, int64_t y) {
    return x - y;
}

int64_t abs_int64(int64_t x, int64_t y) {
    int64_t res = x - y;
    return (res >= 0) ? res : -res;
}

void draw_line_2d(vec2 p1, vec2 p2, char c) {
    size_t screen_x_1 = conv_to_index(p1.x, SCREEN_WIDTH);
    size_t screen_y_1 = conv_to_index(p1.y, SCREEN_HEIGHT);

    size_t screen_x_2 = conv_to_index(p2.x, SCREEN_WIDTH);
    size_t screen_y_2 = conv_to_index(p2.y, SCREEN_HEIGHT);

    // These two cases are not strictly neccesary. I implemented them for learning reasons.
    if (screen_x_1 == screen_x_2) {
        draw_line_vert_2d_internal(screen_x_1, screen_y_1, diff_int64(screen_y_2, screen_y_1), c);
    } else if (screen_y_1 == screen_y_2) {
        draw_line_hort_2d_internal(screen_x_1, screen_y_1, diff_int64(screen_x_2, screen_x_1), c);
    }

    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    if (abs_int64(screen_y_2, screen_y_1) < abs_int64(screen_x_2, screen_x_1)) {
        if (screen_x_1 > screen_x_2) {
            plotLineLow(screen_x_2, screen_y_2, screen_x_1, screen_y_1, c);
        } else {
            plotLineLow(screen_x_1, screen_y_1, screen_x_2, screen_y_2, c);
        }
    } else {
        if (screen_y_1 > screen_y_2) {
            plotLineHigh(screen_x_2, screen_y_2, screen_x_1, screen_y_1, c);
        } else {
            plotLineHigh(screen_x_1, screen_y_1, screen_x_2, screen_y_2, c);
        }
    }
}
