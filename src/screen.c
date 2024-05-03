
#include <stdio.h>
#include <string.h>

#include "misc.h"
#include "screen.h"

char screen_buf[SCREEN_HEIGHT][SCREEN_WIDTH];
float depth_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

void clear_lines(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        CLEAR_LINE();
        putchar('\n');
    }
}

void screen_clear(void) {
    memset(screen_buf, ' ', sizeof screen_buf);
}

void screen_init(void) {
    printf(CSI_ESC CSI_HIDECURSOR);
    screen_clear();
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
            putchar(screen_buf[y][x]);
        }
        putchar('\n');
    }
}
