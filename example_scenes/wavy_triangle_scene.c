#include "wavy_triangle_scene.h"
#include "draw.h"
#include "scene.h"
#include "transform.h"
#include "screen.h"

#ifdef DEBUG
#include "misc.h"
#endif

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef struct {
    int64_t angle_deg;
} triangle;

const scene_type g_wavy_triangle_scene = {.flags = SCENE_OPS_NOP,
                                          .create = wavy_triangle_create,
                                          .destroy = wavy_triangle_destroy,
                                          .update = wavy_triangle_update,
                                          .render = wavy_triangle_render};

#define TRIANGLE_OBJ 0

void** wavy_triangle_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));
    context_ptr[TRIANGLE_OBJ] = malloc(sizeof(triangle));
    ((triangle*)context_ptr[TRIANGLE_OBJ])->angle_deg = 0;
    return context_ptr;
}

void wavy_triangle_destroy(void** context_ptr) {
    free(context_ptr[TRIANGLE_OBJ]);
    free(context_ptr);
}

void wavy_triangle_update(void** context_ptr) {
    triangle* triangle_ptr = (triangle*)context_ptr[TRIANGLE_OBJ];
    triangle_ptr->angle_deg += (int)(10.f * MS_PER_UPDATE / 400.f);
}

void wavy_triangle_render(void** context_ptr) {
    triangle* triangle_ptr = (triangle*)context_ptr[TRIANGLE_OBJ];
    int64_t angle_deg = triangle_ptr->angle_deg;
    float angle_rad = to_angle_in_radians((float)-angle_deg);

    float a1 = sinf(PI / 3.f * (angle_rad - 3.f));
    float a2 = sinf(PI / 3.f * (angle_rad - 4.5f));
    float x = sinf(PI / 12.f * (angle_rad - 6.f));

    float b1 = max_float(((a1 * a1 + a1) / 2.f + 1.f) / 2.f, 0.5f);
    float b2 = max_float(((a2 * a2 + a2) / 2.f + 1.f) / 2.f, 0.5f);

    vec2 v1 = {.x = x, .y = 0.5f};
    vec2 v3 = {.x = -b1, .y = -b1};
    vec2 v2 = {.x = b2, .y = -b2};

    draw_triangle_2d((vec2[3]){v1, v2, v3}, (color[3]){color_white, color_white, color_white}, '*');

#ifdef DEBUG
    draw_point_2d(&v1, &color_white, '1');
    draw_point_2d(&v2, &color_white, '2');
    draw_point_2d(&v3, &color_white, '3');
#endif
}
