
#include <stdio.h>
#include <string.h>

#include "screen.h"

char screen_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

#define STR(s) #s
#define XSTR(s) STR(s)

#define CSI_ESC "\033["
#define CSI_UP "A"
#define CSI_SHOWCURSOR "?25h"
#define CSI_HIDECURSOR "?25l"
#define CSI_CLEARLINE "2K"

void clear_lines(void) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        printf(CSI_ESC CSI_CLEARLINE "\n");
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
    printf(CSI_ESC XSTR(SCREEN_HEIGHT) CSI_UP);
    putchar('\r');
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            putchar(screen_buf[y][x]);
        }
        putchar('\n');
    }
}
