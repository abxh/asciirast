
#include <math.h>
#include <stdio.h>
#include <string.h>

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
    memset(framebuf, ' ', sizeof framebuf);
}

void depthbuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            depthbuf[y][x] = 0;
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

// helper functions for frame buffer:
// -----------------------------------------------------------------------------

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

void draw_framebuf_point_w_no_bounds_checking(int framebuf_x, int framebuf_y, char c) {
    framebuf[framebuf_y][framebuf_x] = c;
}

void draw_framebuf_point_w_bounds_checking(int framebuf_x, int framebuf_y, char c) {
    if (!inside_framebuf(framebuf_x, framebuf_y)) {
        return;
    }

    framebuf[framebuf_y][framebuf_x] = c;
}

framebuf_coords to_framebuf_coords(vec2 v) {
    return (framebuf_coords){.x = to_framebuf_x(v.x), .y = to_framebuf_y(v.y)};
}

void swap_framebuf_coords(framebuf_coords* v1_ptr, framebuf_coords* v2_ptr) {
    framebuf_coords t = *v1_ptr;
    *v1_ptr = *v2_ptr;
    *v2_ptr = t;
}
