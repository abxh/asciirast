#pragma once

#include <stdlib.h>

#define SCREEN_HEIGHT 20
#define SCREEN_WIDTH 40

extern char screen_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

void screen_init(void);

void screen_deinit(void);

void screen_clear(void);

void screen_refresh(void);
