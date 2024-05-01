#pragma once

#include <stdlib.h>

#define SCREEN_HEIGHT 20
#define SCREEN_WIDTH 40

extern char screen_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

extern void screen_init(void);

extern void screen_deinit(void);

extern void screen_clear(void);

extern void screen_refresh(void);
