#pragma once

#include "scene.h"

void** cube_scene_create(void);

void cube_scene_destroy(void** context_ptr);

void cube_scene_update(void** context_ptr);

void cube_scene_render(void** context_ptr);

extern const scene_type g_cube_scene;
