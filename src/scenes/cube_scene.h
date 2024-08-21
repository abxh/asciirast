#pragma once

#include "scene_type.h"

void* cube_scene_create(struct engine_handle_type* handle_p);

void cube_scene_destroy(void* obj_p, struct engine_handle_type* handle_p);

void cube_scene_on_event(void* obj_p, struct engine_handle_type* handle_p, const SDL_Event* event_p);

void cube_scene_update(void* obj_p, uint64_t dt_ms);

void cube_scene_render(const void* obj_p, struct renderer_type* renderer_p);

static const scene_type g_cube_scene = {.id = 2,
                                        .create = cube_scene_create,
                                        .destroy = cube_scene_destroy,
                                        .on_event = cube_scene_on_event,
                                        .update = cube_scene_update,
                                        .render = cube_scene_render};
