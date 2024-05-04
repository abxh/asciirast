#pragma once

#include "scene.h"

void** spiral_scene_create(void);

void spiral_scene_destroy(void** context_ptr);

void spiral_scene_update(void** context_ptr);

extern scene_type spiral_scene;
