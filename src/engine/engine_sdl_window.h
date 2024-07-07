#pragma once

#include "sc_map.h"

#ifndef SDL_WINDOW_WIDTH
#define SDL_WINDOW_WIDTH 600
#endif

#ifndef SDL_WINDOW_HEIGHT
#define SDL_WINDOW_HEIGHT 200
#endif

#ifndef SDL_FONT_SIZE
#define SDL_FONT_SIZE 32
#endif

#ifndef SDL_FONT_PATH
#define SDL_FONT_PATH "tff/terminus.ttf"
#endif

struct engine_sdl_window_type;

struct engine_sdl_window_type* engine_sdl_window_create(struct sc_map_str* cmd_map_p);

void engine_sdl_window_destroy(struct engine_sdl_window_type* obj_p);

void engine_sdl_window_update(struct engine_sdl_window_type* obj_p);

void engine_sdl_window_render(struct engine_sdl_window_type* obj_p);
