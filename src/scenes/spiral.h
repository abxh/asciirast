#pragma once

#include "scene.h"

void** spiral_create(void);

void spiral_destroy(void** context_ptr);

void spiral_update(void** context_ptr);

extern scene_type spiral_scene;
