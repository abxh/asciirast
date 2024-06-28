#pragma once

#include "external.h"

#include "scene.h"

typedef struct {
    char key_comb[20];
    char command_name[20];
} cmd_type;

#define STACK_PREFIX cmdstk
#define VALUE_TYPE cmd_type
#include "stack.h"

extern cmdstk_type* g_commands_stack;

#ifndef MS_PER_UPDATE
#define MS_PER_UPDATE 100
#endif

void run(const scene_type scene);
