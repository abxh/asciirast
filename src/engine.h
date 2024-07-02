#pragma once

#include "engine_cmd_ht.h"
#include "scene.h"

typedef struct {
    cmdht_type* cmdht_p;
    scene_type next_scene;
} engine_settings_type;

extern engine_settings_type g_engine_settings;

void run(void); //(const scene_type scene);
