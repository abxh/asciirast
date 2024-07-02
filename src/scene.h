#pragma once

#include <SDL2/SDL_events.h>

#include <stdbool.h>
#include <stdint.h>

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

    void* (*create)(void);
    void (*destroy)(void* context_p);
    void (*render)(const void* context_p);

    void (*update)(void* context_p);
    void (*on_event)(void* context_p, const event_type event_p);
} scene_type;
