#pragma once

#include "scene.h"

void** triangle_scene_create(void);

void triangle_scene_destroy(void** context_ptr);

void triangle_scene_update(void** context_ptr);

extern scene_type triangle_scene;
