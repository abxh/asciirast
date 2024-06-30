#pragma once

#include <SDL2/SDL_events.h>

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    SCENE_NOP = 0,    // 0b0
    SCENE_ON_KEY = 1, // 0b1
} SCENE_EXTENSIONS;

typedef struct {
    uint64_t id;
    uint64_t flags;

    void* (*create)(void);
    void (*destroy)(void* context_ptr);
    void (*update)(void* context_ptr);
    void (*render)(const void* context_ptr);

    void (*on_key_up)(void* context_ptr, const SDL_KeyboardEvent key);
    void (*on_key_down)(void* context_ptr, const SDL_KeyboardEvent key);
} scene_type;

static inline bool is_equal_scene(const scene_type s0, const scene_type s1) {
    return s0.id == s1.id;
}
