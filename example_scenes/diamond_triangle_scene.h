#pragma once

#include "scene.h"

void** diamond_triangle_scene_create(void);

void diamond_triangle_scene_destroy(void** context_ptr);

void diamond_triangle_scene_update(void** context_ptr);

extern const scene_type g_diamond_triangle_scene;
