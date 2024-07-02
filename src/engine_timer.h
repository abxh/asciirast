#pragma once

#include <stdbool.h>

#ifndef MS_PER_UPDATE
#define MS_PER_UPDATE 100
#endif

struct engine_timer_type;

struct engine_timer_type* engine_timer_create(void);

void engine_timer_destroy(struct engine_timer_type* context_p);

void engine_timer_tick(struct engine_timer_type* context_p);

bool engine_timer_scene_should_update(const struct engine_timer_type* context_p);

void engine_timer_scene_tick(struct engine_timer_type* context_p);
