#pragma once

#include "renderer.h"

#include <SDL2/SDL_events.h>

#include <stdbool.h>
#include <stdint.h>

struct engine_handle_type;

typedef enum {
    SCENE_NOP = 0b0,
    SCENE_UPDATE = 0b1,
    SCENE_ON_EVENT = 0b10,
} scene_flags_type;

typedef struct {
    SDL_Event* sdl_event_p;
} event_type;

typedef struct {
    uint64_t flags;

    void* (*create)(struct engine_handle_type* handle_p);
    void (*destroy)(void* obj_p, struct engine_handle_type* handle_p);
    void (*render)(const void* obj_p, const struct renderer_type* renderer_p);

    void (*update)(void* obj_p, const uint64_t dt);
    void (*on_event)(void* obj_p, const event_type event_p);
} scene_type;
