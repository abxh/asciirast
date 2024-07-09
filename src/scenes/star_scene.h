#pragma once

#include "scene_type.h"

void* star_scene_create(struct engine_handle_type* handle_p);

void star_scene_destroy(void* obj_p, struct engine_handle_type* handle_p);

void star_scene_on_event(void* obj_p, struct engine_handle_type* handle_p, const SDL_Event* event_p);

void star_scene_update(void* obj_p, uint64_t dt_ms);

void star_scene_render(const void* obj_p, struct renderer_type* renderer_p);

static const scene_type g_star_scene = {.id = 1,
                                        .create = star_scene_create,
                                        .destroy = star_scene_destroy,
                                        .on_event = star_scene_on_event,
                                        .update = star_scene_update,
                                        .render = star_scene_render};
