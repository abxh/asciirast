#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#include "draw.h"
#include "misc.h"
#include "screen.h"
#include "transform.h"

typedef struct {
    int x;
    int y;
} screen_indicies;

void plot(int screen_x, int screen_y, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    screen_buf[screen_y][screen_x] = c;
}

void plot_at_indicies(screen_indicies sv, char c) {
    plot(sv.x, sv.y, c);
}

int conv_to_screen_index(float x, int length) {
    return (x + 1.) / 2 * (length - 1);
}

int conv_to_screen_index_clamped(float x, int length) {
    return (clamp(x, -1., 1.) + 1.) / 2 * (length - 1);
}

screen_indicies conv_to_screen_indicies(vec2 vec) {
    return (screen_indicies){conv_to_screen_index(vec.x, SCREEN_WIDTH), conv_to_screen_index(-vec.y, SCREEN_HEIGHT)};
}

screen_indicies conv_to_screen_indicies_clamped(vec2 vec) {
    return (screen_indicies){conv_to_screen_index_clamped(vec.x, SCREEN_WIDTH), conv_to_screen_index_clamped(-vec.y, SCREEN_HEIGHT)};
}

void draw_point_2d(vec2 vec, char c) {
    plot_at_indicies(conv_to_screen_indicies_clamped(vec), c);
}

void draw_line_vert_2d_internal(int screen_x, int screen_y, int steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    assert(IMPLIES(steps >= 0, screen_y + steps < SCREEN_HEIGHT));
    assert(IMPLIES(steps < 0, screen_y + steps >= 0));

    if (steps >= 0) {
        for (int var = screen_y; var <= screen_y + steps; var += 1) {
            plot(screen_x, var, c);
        }
    } else {
        for (int var = screen_y; var + 1 > screen_y + steps; var -= 1) {
            plot(screen_x, var, c);
        }
    }
}

void draw_line_hort_2d_internal(int screen_x, int screen_y, int steps, char c) {
    assert(0 <= screen_x && screen_x < SCREEN_WIDTH);
    assert(0 <= screen_y && screen_y < SCREEN_WIDTH);

    assert(IMPLIES(steps >= 0, screen_x + steps < SCREEN_WIDTH));
    assert(IMPLIES(steps < 0, (int)screen_x + steps >= 0));

    if (steps >= 0) {
        for (int var = screen_x; var <= screen_x + steps; var += 1) {
            plot(var, screen_y, c);
        }
    } else {
        for (int var = screen_x; var + 1 > screen_x + steps; var -= 1) {
            plot(var, screen_y, c);
        }
    }
}

void plotLineLow(int x0, int y0, int x1, int y1, char c) {
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
        plot(x, y, c);
        if (D > 0) {
            y = y + yi;
            D = D + (2 * (dy - dx));
        } else {
            D = D + 2 * dy;
        }
    }
}

void plotLineHigh(int x0, int y0, int x1, int y1, char c) {
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
        plot(x, y, c);
        if (D > 0) {
            x = x + xi;
            D = D + (2 * (dx - dy));
        } else {
            D = D + 2 * dx;
        }
    }
}

int diff_int(int x, int y) {
    return x - y;
}

int abs_int(int x, int y) {
    int res = x - y;
    return (res >= 0) ? res : -res;
}

void draw_line_2d_internal(screen_indicies sp1, screen_indicies sp2, char c) {
    // These two cases are not strictly neccesary. I implemented them for learning / optimization reasons.
    if (sp1.x == sp2.x) {
        draw_line_vert_2d_internal(sp1.x, sp1.y, diff_int(sp2.y, sp1.y), c);
    } else if (sp1.y == sp2.y) {
        draw_line_hort_2d_internal(sp1.x, sp1.y, diff_int(sp2.x, sp1.x), c);
    }

    // Bresenham line algorithm:
    // https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm#All_cases
    if (abs_int(sp2.y, sp1.y) < abs_int(sp2.x, sp1.x)) {
        if (sp1.x > sp2.x) {
            plotLineLow(sp2.x, sp2.y, sp1.x, sp1.y, c);
        } else {
            plotLineLow(sp1.x, sp1.y, sp2.x, sp2.y, c);
        }
    } else {
        if (sp1.y > sp2.y) {
            plotLineHigh(sp2.x, sp2.y, sp1.x, sp1.y, c);
        } else {
            plotLineHigh(sp1.x, sp1.y, sp2.x, sp2.y, c);
        }
    }
}

void draw_line_2d(vec2 p1, vec2 p2, char c) {
    screen_indicies sp1 = conv_to_screen_indicies_clamped(p1);
    screen_indicies sp2 = conv_to_screen_indicies_clamped(p2);

    draw_line_2d_internal(sp1, sp2, c);
}

float get_slope_int(float x1, float y1, float x2, float y2) {
    return y2 - y1 / x2 - x1;
}

void swap_vec(vec2* vec1ptr, vec2* vec2ptr) {
    vec2 temp = *vec1ptr;
    *vec1ptr = *vec2ptr;
    *vec2ptr = temp;
}

void draw_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c) {
    // Triangle filling:
    // http://www.sunshine2k.de/coding/java/TriangleRasterization/TriangleRasterization.html

    // Swap so p1.y <= p2.y <= p3.y.
    // TODO: proper float comparision
    if (p2.y < p1.y)
        swap_vec(&p2, &p1);
    if (p3.y < p1.y)
        swap_vec(&p3, &p1);
    if (p3.y < p2.y)
        swap_vec(&p3, &p2);
}
