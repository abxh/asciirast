#pragma once

#include "scene.h"

void** star_create(void);

void star_destroy(void** context_ptr);

void star_update(void** context_ptr);

void star_on_key(void** context_ptr, char c);

extern scene_type star_scene;
