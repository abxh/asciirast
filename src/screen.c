#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "misc.h"
#include "screen.h"

static char framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

static float depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

static inline void clear_lines(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        CLEAR_LINE();
        putchar('\n');
    }
}

static inline void framebuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            framebuf[y][x] = ' ';
        }
    }
}

static inline void depthbuf_clear(void) {
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

bool point_inside_framebuf(vec2int v0) {
    bool inside_framebuf_x = 0 <= v0.x && v0.y < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= v0.x && v0.y < SCREEN_HEIGHT;

    return inside_framebuf_x && inside_framebuf_y;
}

static inline bool point_inside_framebuf_split(int x0, int y0) {
    bool inside_framebuf_x = 0 <= x0 && x0 < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= y0 && y0 < SCREEN_HEIGHT;

    return inside_framebuf_x && inside_framebuf_y;
}

int to_framebuf_x(float screen_x) {
    return (int)((screen_x + 1.f) / 2.f * (SCREEN_WIDTH - 1));
}

int to_framebuf_y(float screen_y) {
    return (int)((-screen_y + 1.f) / 2.f * (SCREEN_HEIGHT - 1));
}

vec2int to_framebuf_coords(vec2 v) {
    return (vec2int){.x = to_framebuf_x(v.x), .y = to_framebuf_y(v.y)};
}

void plot_point_w_depth_unchecked_bounds(int x0, int y0, char c, float depth) {
    assert(inside_range_float(depth, 0.f, 1.f));
    assert(point_inside_framebuf_split(x0, y0));

    if (depth > depthbuf[y0][x0]) {
        return;
    }
    framebuf[y0][x0] = c;
    depthbuf[y0][x0] = depth;
}

void plot_point_w_depth(int x0, int y0, char c, float depth) {
    if (!point_inside_framebuf_split(x0, y0)) {
        return;
    }
    plot_point_w_depth_unchecked_bounds(x0, y0, c, depth);
}

void plot_point(int x0, int y0, char c) {
    plot_point_w_depth(x0, y0, c, 0);
}
void plot_point_vec2int_w_depth_unchecked_bounds(vec2int v0, char c, float depth) {
    plot_point_w_depth_unchecked_bounds(v0.x, v0.y, c, depth);
}

void plot_point_vec2int_w_depth(vec2int v0, char c, float depth) {
    plot_point_w_depth(v0.x, v0.y, c, depth);
}

void plot_point_vec2int(vec2int v0, char c) {
    plot_point(v0.x, v0.y, c);
}
