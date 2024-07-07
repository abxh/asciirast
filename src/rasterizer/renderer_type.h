#pragma once

#include "math/vec.h"
#include "rasterizer/ascii_table_type.h"

#include "sc_list.h"

#include <stdbool.h>

typedef struct {
    vec4_type left;
    vec4_type right;
    vec4_type bottom;
    vec4_type top;
    vec4_type near;
    vec4_type far;
} frustum_planes_type;

typedef struct renderer_type {
    float perspective_mat[4][4];
    float model_camera_mat[4][4];
    float mvp[4][4];
    frustum_planes_type plane;

    ascii_table_type table;
    struct sc_list lst;

    struct screen_type* screen_p;
} renderer_type;
