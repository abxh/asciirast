#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "misc.h"
#include "screen.h"

char g_framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

float g_depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

colorint_str g_colorbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

size_t g_extra_lines = 0;

static inline void framebuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            g_framebuf[y][x] = ' ';
        }
    }
}

static inline void depthbuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            g_depthbuf[y][x] = INFINITY;
        }
    }
}

static inline void colorbuf_clear(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            g_colorbuf[y][x] = (colorint_str){.r = "255", .g = "255", .b = "255"};
        }
    }
}

void screen_clear(void) {
    framebuf_clear();
    depthbuf_clear();
    colorbuf_clear();
}

void screen_init(void) {
    printf(CSI_ESC CSI_HIDECURSOR);

    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        CLEAR_LINE();
        printf(NEW_LINE);
    }

    screen_clear();
}

void screen_deinit(void) {
    printf(CSI_ESC CSI_SHOWCURSOR);
    printf(CSI_ESC CSI_RESETCOLOR);
}

void screen_restore_line_cursor(void) {
    for (size_t i = 0; i < g_extra_lines; i++) {
        MOVE_UP_LINES(1);
    }
    g_extra_lines = 0;
}

void screen_refresh(void) {
    MOVE_UP_LINES(SCREEN_HEIGHT);
    putchar('\r');

    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            printf(CSI_ESC CSI_SETCOLOR_INITIALS "%.3s;%.3s;%.3s;m", g_colorbuf[y][x].r, g_colorbuf[y][x].g, g_colorbuf[y][x].b);
            putchar(g_framebuf[y][x]);
        }
        printf(NEW_LINE);
    }

    printf(CSI_ESC CSI_RESETCOLOR);
}
