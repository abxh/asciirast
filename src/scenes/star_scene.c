#include <stdint.h>
#include <stdlib.h>

#include "draw.h"
#include "scenes/scene.h"
#include "star_scene.h"
#include "transform.h"

#ifdef DEBUG
#include <stdio.h>
#include "misc.h"
#endif

typedef struct {
    int64_t angle_deg;
} star;

scene_type star_scene = {.flags = SCENE_OPS_NOP, .create = star_scene_create, .destroy = star_scene_destroy, .update = star_scene_update};

#define STAR_OBJ 0

void** star_scene_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));
    context_ptr[STAR_OBJ] = malloc(sizeof(star));
    ((star*)context_ptr[STAR_OBJ])->angle_deg = 350;
    return context_ptr;
}

void star_scene_destroy(void** context_ptr) {
    free(context_ptr[STAR_OBJ]);
    free(context_ptr);
}

void star_scene_update(void** context_ptr) {
    star* star_ptr = (star*)context_ptr[STAR_OBJ];
    int64_t angle_deg = star_ptr->angle_deg;

    for (size_t l = 0; l < 6; l++) {
        vec2 v_base = {.x = 0.5, .y = -1.};

        float angle_rad1 = to_angle_in_radians(angle_deg + (72 + 72) * (l + 0));
        float angle_rad2 = to_angle_in_radians(angle_deg + (72 + 72) * (l + 1));

        vec2 v1 = rotate_around_origo_vec2(v_base, angle_rad1);
        vec2 v2 = rotate_around_origo_vec2(v_base, angle_rad2);

        draw_line_2d(v1, v2, '*');

#ifdef DEBUG
        CLEAR_LINE();
        printf("l: %zu\n", l);
        CLEAR_LINE();
        printf("angle_deg: %ld, angle_rad1: %.2f, angle_rad2: %.2f\n", angle_deg, angle_rad1, angle_rad2);
        CLEAR_LINE();
        printf("v_base.x: %.2f, v_base.y: %.2f\n", v_base.x, v_base.y);
        CLEAR_LINE();
        printf("v1.x: %.2f, v1.y: %.2f\n", v1.x, v1.y);
        CLEAR_LINE();
        printf("v2.x: %.2f, v2.y: %.2f\n", v2.x, v2.y);
        MOVE_UP_LINES(5);
        sleep_portable(100);
#endif
    }

    if (angle_deg == 0) {
        star_ptr->angle_deg = 360;
    }
    star_ptr->angle_deg -= 10;
}
