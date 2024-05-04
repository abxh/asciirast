#pragma once

#include "scene.h"

void** cube_scene_create(void);

void cube_scene_destroy(void** context_ptr);

void cube_scene_update(void** context_ptr);

extern scene_type cube_scene;
