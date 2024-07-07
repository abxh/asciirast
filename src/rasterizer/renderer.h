#pragma once

#include "math/vec.h"

#include <stddef.h>

struct renderer_type;
struct screen_type;

typedef struct {
    float fovy_rad;
    float aspect_ratio;
    float z_near;
    float z_far;
} perspective_proj_prop_type;

typedef struct {
    vec3_type eye;
    vec3_type center;
    vec3_type up;
} camera_prop_type;

struct renderer_type;

struct renderer_type* renderer_create(struct screen_type* screen_context_p, const size_t n, const char acsii_palette[n],
                                      const perspective_proj_prop_type prop);

void renderer_destroy(struct renderer_type* obj_p);

void renderer_use_ascii_palette(struct renderer_type* obj_p, const size_t n, const char ascii_palette[n]);

void renderer_look_at(struct renderer_type* obj_p, const camera_prop_type prop);