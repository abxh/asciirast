#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw.h"
#include "scenes/scene.h"
#include "transform.h"
#include "triangle_scene.h"

#ifdef DEBUG
#include "misc.h"
#endif

typedef struct {
    int64_t angle_deg;
} triangle;

scene_type triangle_scene = {
    .flags = SCENE_OPS_NOP, .create = triangle_scene_create, .destroy = triangle_scene_destroy, .update = triangle_scene_update};

#define TRIANGLE_OBJ 0

void** triangle_scene_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));
    context_ptr[TRIANGLE_OBJ] = malloc(sizeof(triangle));
    ((triangle*)context_ptr[TRIANGLE_OBJ])->angle_deg = 0;
    return context_ptr;
}

void triangle_scene_destroy(void** context_ptr) {
    free(context_ptr[TRIANGLE_OBJ]);
    free(context_ptr);
}

void triangle_scene_update(void** context_ptr) {
    triangle* triangle_ptr = (triangle*)context_ptr[TRIANGLE_OBJ];
    int64_t angle_deg = triangle_ptr->angle_deg;
    float fov_const = to_fov_constant_from_degrees(60.);

    float angle_rad = to_angle_in_radians(-angle_deg);
    vec2 v1 = {.x = 0., .y = 1.f};
    vec2 v2 = {.x = 0.7, .y = -1.f};
    vec2 v3 = {.x = -0.7, .y = -1.f};

    draw_triangle_vec2(v1, v2, v3, '*');

    if (angle_deg == 170) {
        triangle_ptr->angle_deg = 0;
    }
    triangle_ptr->angle_deg += 10;
}
