#pragma once

#include "scene.h"

void** rgb_triangle_scene_create(void);

void rgb_triangle_scene_destroy(void** context_ptr);

void rgb_triangle_scene_update(void** context_ptr);

void rgb_triangle_scene_render(void** context_ptr);

void rgb_triangle_scene_on_key(void** context_ptr, char key);

extern const scene_type g_rgb_triangle_scene;
