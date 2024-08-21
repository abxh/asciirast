#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifndef MS_PER_UPDATE
#define MS_PER_UPDATE 100
#endif

struct engine_timer_type;

struct engine_timer_type* engine_timer_create(void);

void engine_timer_destroy(struct engine_timer_type* obj_p);

void engine_timer_frame_tick(struct engine_timer_type* obj_p);

bool engine_timer_scene_should_update(struct engine_timer_type* obj_p);

void engine_timer_scene_tick(struct engine_timer_type* obj_p);
