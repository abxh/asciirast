#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "misc.h"
#include "screen.h"

char framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

float depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

void clear_lines(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        CLEAR_LINE();
        putchar('\n');
    }
}

void framebuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            framebuf[y][x] = ' ';
        }
    }
}

void depthbuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            depthbuf[y][x] = INFINITY;
        }
    }
}

void screen_clear(void) {
    framebuf_clear();
    depthbuf_clear();
}

void screen_init(void) {
    printf(CSI_ESC CSI_HIDECURSOR);
    framebuf_clear();
    clear_lines();
}

void screen_deinit(void) {
    printf(CSI_ESC CSI_SHOWCURSOR);
}

void screen_refresh(void) {
    MOVE_UP_LINES(SCREEN_HEIGHT);
    putchar('\r');
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            putchar(framebuf[y][x]);
        }
        putchar('\n');
    }
}

// framebuffer and depthbuffer helper functions:
// ----------------------------------------------------------------------------

bool inside_framebuf(int framebuf_x, int framebuf_y) {
    bool inside_framebuf_x = 0 <= framebuf_x && framebuf_x < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= framebuf_y && framebuf_y < SCREEN_HEIGHT;

    return inside_framebuf_x && inside_framebuf_y;
}

int to_framebuf_x(float screen_x) {
    return (screen_x + 1.) / 2. * (SCREEN_WIDTH - 1);
}

int to_framebuf_y(float screen_y) {
    return (-screen_y + 1.) / 2. * (SCREEN_HEIGHT - 1);
}

vec2int to_framebuf_coords(vec2 v) {
    return (vec2int){.x = to_framebuf_x(v.x), .y = to_framebuf_y(v.y)};
}

// ----------------------------------------------------------------------------

void plot_point_w_depth_fast_unchecked(int framebuf_x, int framebuf_y, char c, float depth) {
    assert(inside_framebuf(framebuf_x, framebuf_y));
    assert(inside_range_float(depth, 0.f, 1.f));

    if (depth > depthbuf[framebuf_y][framebuf_x]) {
        return;
    }
    framebuf[framebuf_y][framebuf_x] = c;
    depthbuf[framebuf_y][framebuf_x] = depth;
}

void plot_point_w_depth(int framebuf_x, int framebuf_y, char c, float depth) {
    if (!inside_framebuf(framebuf_x, framebuf_y)) {
        return;
    }
    plot_point_w_depth_fast_unchecked(framebuf_x, framebuf_y, c, depth);
}

void plot_point(int framebuf_x, int framebuf_y, char c) {
    plot_point_w_depth(framebuf_x, framebuf_y, c, 0);
}
