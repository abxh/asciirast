#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw.h"
#include "star.h"
#include "transform.h"

typedef struct {
    int64_t angle_deg;
} star;

object_ops_type star_ops = {
    .flags = OBJ_OPS_ON_KEY, .create = star_create, .destroy = star_destroy, .update = star_update};

#define STAR_OBJ 0

void** star_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));
    context_ptr[STAR_OBJ] = malloc(sizeof(star));
    return context_ptr;
}

void star_destroy(void** context_ptr) {
    free(context_ptr[STAR_OBJ]);
    free(context_ptr);
}

void star_update(void** context_ptr) {
    star* star_ptr = (star*)context_ptr[STAR_OBJ];
    int64_t angle_deg = star_ptr->angle_deg;

    for (size_t l = 0; l < 5; l++) {
        vec2 v_base = {.x = 0.5, -1.};
        vec2 v1 = rotate_around_origo_2d(v_base, conv_to_angle_rad(angle_deg + (72 + 72) * (l + 0)));
        vec2 v2 = rotate_around_origo_2d(v_base, conv_to_angle_rad(angle_deg + (72 + 72) * (l + 1)));
        draw_line_2d(v1, v2, '*');
    }
    if (angle_deg == 350) {
        star_ptr->angle_deg = 0;
    }
    star_ptr->angle_deg += 10;
}
