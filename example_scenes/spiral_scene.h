#pragma once

#include "scene.h"

void** spiral_scene_create(void);

void spiral_scene_destroy(void** context_ptr);

void spiral_scene_update(void** context_ptr);

extern const scene_type g_spiral_scene;
