#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "draw.h"
#include "objects/object.h"
#include "spiral.h"
#include "transform.h"

#ifdef DEBUG
#include "misc.h"
#endif

typedef struct {
    int64_t angle_deg;
} spiral;

object_ops_type spiral_ops = {.flags = OBJ_OPS_NOP, .create = spiral_create, .destroy = spiral_destroy, .update = spiral_update};

#define SPIRAL_OBJ 0

void** spiral_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));
    context_ptr[SPIRAL_OBJ] = malloc(sizeof(spiral));
    return context_ptr;
}

void spiral_destroy(void** context_ptr) {
    free(context_ptr[SPIRAL_OBJ]);
    free(context_ptr);
}

void spiral_update(void** context_ptr) {
    spiral* spiral_ptr = (spiral*)context_ptr[SPIRAL_OBJ];
    int64_t angle_deg = spiral_ptr->angle_deg;
    for (size_t o = 0; o < 6; o++) {
        for (size_t i = 0; i <= 6; i++) {
            float angle_rad = conv_to_angle_rad(-angle_deg - 20. * i - 60 * o);
            vec2 v_base = {.x = 0.6 - 0.1 * (float)i, .y = 0.6 - 0.1 * (float)i};
            vec2 v = rotate_around_origo_2d(v_base, angle_rad);

            draw_point_2d(v, '*');

#ifdef DEBUG
            CLEAR_LINE();
            printf("i: %zu, o: %zu\n", i, o);
            CLEAR_LINE();
            printf("angle_deg: %ld, angle_rad: %.2f\n", angle_deg, angle_rad);
            CLEAR_LINE();
            printf("v_base.x: %.2f, v_base.y: %.2f\n", v_base.x, v_base.y);
            CLEAR_LINE();
            printf("v.x: %.2f, v.y: %.2f\n", v.x, v.y);
            MOVE_UP_LINES(4);
            sleep_portable(100);
#endif
        }
    }
    if (angle_deg == 350) {
        spiral_ptr->angle_deg = 0;
    }
    spiral_ptr->angle_deg += 10;
}
