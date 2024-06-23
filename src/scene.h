#pragma once

#include <stdalign.h>
#include <stdint.h>

typedef enum {
    SCENE_OPS_NOP = 0,    // 0b0
    SCENE_OPS_ON_KEY = 1, // 0b1
} EXTENDED_OBJ_OPS_ENUM;

typedef struct {
    uint64_t flags;

    void** (*create)(void);
    void (*destroy)(void** context_ptr);
    void (*update)(void** context_ptr);
    void (*render)(void** context_ptr);

    void (*on_key)(void** context_ptr, char key);
} scene_type;
