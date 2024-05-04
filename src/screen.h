#pragma once

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 30
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 60
#endif

extern char screen_buf[SCREEN_HEIGHT][SCREEN_WIDTH];

void screen_init(void);

void screen_deinit(void);

void screen_clear(void);

void screen_refresh(void);
