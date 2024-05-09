#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#include "draw.h"
#include "scenes/scene.h"
#include "transform.h"
#include "triangle_scene.h"

#ifdef DEBUG
#include "misc.h"
#include <stdio.h>
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
    float angle_rad = to_angle_in_radians(-angle_deg);

    float a1 = sinf(M_PI / 3. * (angle_rad - 3.f));
    float a2 = sinf(M_PI / 3. * (angle_rad - 4.5f));
    float x = sinf(M_PI / 12. * (angle_rad - 6.f));

    float b1 = max_float(((a1 * a1 + a1) / 2.f + 1.f) / 2.f, 0.5f);
    float b2 = max_float(((a2 * a2 + a2) / 2.f + 1.f) / 2.f, 0.5f);

    vec2 v1 = {.x = x, .y = 0.5f};
    vec2 v3 = {.x = -b1, .y = -b1};
    vec2 v2 = {.x = b2, .y = -b2};

    draw_filled_triangle_2d(v1, v3, v2, '*');
    draw_triangle_2d(v1, v3, v2, '+');

    triangle_ptr->angle_deg += 10;
}
