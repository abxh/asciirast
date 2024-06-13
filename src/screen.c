#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "misc.h"
#include "screen.h"

char g_framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

float g_depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

static inline void clear_lines(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        CLEAR_LINE();
        putchar('\n');
    }
}

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
            putchar(g_framebuf[y][x]);
        }
        putchar('\n');
    }
}
