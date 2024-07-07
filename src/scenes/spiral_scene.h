#pragma once

#include "scene_type.h"

void* spiral_scene_create(struct engine_handle_type* handle_p);

void spiral_scene_destroy(void* obj_p, struct engine_handle_type* handle_p);

void spiral_scene_on_event(void* obj_p, struct engine_handle_type* handle_p, const SDL_Event* event_p);

void spiral_scene_update(void* obj_p, uint64_t dt_ms);

void spiral_scene_render(const void* obj_p, struct renderer_type* renderer_p);

static const scene_type g_spiral_scene = {.id = 0,
                                          .create = spiral_scene_create,
                                          .destroy = spiral_scene_destroy,
                                          .on_event = spiral_scene_on_event,
                                          .update = spiral_scene_update,
                                          .render = spiral_scene_render};
