
#include "rotating_triangle_scene.h"
#include "draw.h"
#include "scene.h"
#include "transform.h"

#ifdef DEBUG
#include "misc.h"
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef struct {
    int64_t angle_deg;
} rotating_triangle;

const scene_type g_rotating_triangle_scene = {.flags = SCENE_OPS_NOP,
                                      .create = rotating_triangle_scene_create,
                                      .destroy = rotating_triangle_scene_destroy,
                                      .update = rotating_triangle_scene_update};

#define TRIANGLE_OBJ 0

void** rotating_triangle_scene_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));

    context_ptr[TRIANGLE_OBJ] = malloc(sizeof(rotating_triangle));

    ((rotating_triangle*)context_ptr[TRIANGLE_OBJ])->angle_deg = 0;

    return context_ptr;
}

void rotating_triangle_scene_destroy(void** context_ptr) {
    free(context_ptr[TRIANGLE_OBJ]);
    free(context_ptr);
}

void rotating_triangle_scene_update(void** context_ptr) {
    rotating_triangle* triangle_ptr = (rotating_triangle*)context_ptr[TRIANGLE_OBJ];

    int64_t angle_deg = triangle_ptr->angle_deg;
    float angle_rad = to_angle_in_radians((float)-angle_deg);
    static vec3 shift = {0, 0, 1.75f};

    vec3 v1 = {-0.5f, 0.7f, 0.f};
    vec3 v2 = {-0.5f, -0.7f, 0.f};
    vec3 v3 = {0.5f, 0.3f, 0.f};

    v1 = rotate_around_y_axis(v1, angle_rad);
    v2 = rotate_around_y_axis(v2, angle_rad);
    v3 = rotate_around_y_axis(v3, angle_rad);

    v1 = sum_vec3(v1, shift);
    v2 = sum_vec3(v2, shift);
    v3 = sum_vec3(v3, shift);

    draw_triangle_3d(v1, v2, v3, '*');
    draw_triangle_3d(v1, v3, v2, '.');

#ifdef DEBUG
    draw_point_3d(v1, '1');
    draw_point_3d(v2, '2');
    draw_point_3d(v3, '2');

    printf("angle_rad: %.2f\n", angle_rad);
    CLEAR_LINE();
    MOVE_UP_LINES(1);
    sleep_ms(200);
#endif

    if (angle_deg == 360) {
        triangle_ptr->angle_deg = 0;
    }
    triangle_ptr->angle_deg += 10;
}
