#pragma once

#include "scenes/scene_type.h"

struct engine_handle_type;

void engine_run(const scene_type default_scene, const size_t n, const char ascii_palette[n]);

void engine_quit(struct engine_handle_type* handle_p);

struct renderer_type* engine_get_renderer(struct engine_handle_type* handle_p);

struct sc_map_str* engine_get_cmd_text(struct engine_handle_type* handle_p);

void engine_set_next_scene(struct engine_handle_type* handle_p, scene_type next_scene);
