#pragma once

#include <stdbool.h>

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 20
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 40
#endif

#define ASPECT_RATIO (2.f * (float)SCREEN_HEIGHT / (float)(SCREEN_WIDTH))

extern char g_framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];

extern float g_depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];

void screen_init(void);

void screen_deinit(void);

void screen_clear(void);

void screen_refresh(void);
