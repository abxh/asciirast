#pragma once

#include <stdint.h>

typedef enum {
    OBJ_OPS_NOP = 0b0,
    OBJ_OPS_ON_KEY = 0b1,
} EXTENDED_OBJ_OPS_ENUM;

typedef struct {
    uint32_t flags;

    void** (*create)();
    void (*destroy)(void** context_ptr);
    void (*update)(void** context_ptr);

    void (*on_key)(void** context_ptr, char key);
} object_ops_type;
