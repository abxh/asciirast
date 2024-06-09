#pragma once

#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 20
#endif

#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 40
#endif

extern char framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];  // ' ': no value

extern float depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH]; // INFINITY: no value

void screen_init(void);

void screen_deinit(void);

void screen_clear(void);

void screen_refresh(void);
