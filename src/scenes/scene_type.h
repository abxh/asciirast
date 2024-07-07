#pragma once

#include <SDL2/SDL_events.h>

#include <stdbool.h>
#include <stdint.h>

struct renderer_type;
struct engine_handle_type;

typedef struct {
    uint64_t id;

    void* (*create)(struct engine_handle_type* handle_p);
    void (*destroy)(void* obj_p, struct engine_handle_type* handle_p);
    void (*on_event)(void* obj_p, struct engine_handle_type* handle_p, const SDL_Event* event_p);
    void (*update)(void* obj_p, uint64_t dt_ms);
    void (*render)(const void* obj_p, struct renderer_type* renderer_p);
} scene_type;

static inline bool scene_is_equal(scene_type s0, scene_type s1) {
    return s0.id == s1.id;
}
