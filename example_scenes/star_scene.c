
#include "star_scene.h"
#include "color.h"
#include "draw.h"
#include "scene.h"
#include "transform.h"

#ifdef DEBUG
#include "misc.h"
#include "screen.h"
#endif

#include <stdint.h>
#include <stdlib.h>

#ifdef DEBUG
#include <stdio.h>
#endif

typedef struct {
    int64_t angle_deg;
} star;

const scene_type g_star_scene = {.flags = SCENE_OPS_NOP,
                                 .create = star_scene_create,
                                 .destroy = star_scene_destroy,
                                 .update = star_scene_update,
                                 .render = star_scene_render};

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
    star_ptr->angle_deg -= 10;
}

void star_scene_render(void** context_ptr) {
    star* star_ptr = (star*)context_ptr[STAR_OBJ];
    int64_t angle_deg = star_ptr->angle_deg;

    for (size_t l = 0; l < 6; l++) {
        vec2 v_base = {.x = 0.5, .y = -1.};

        float angle_rad1 = to_angle_in_radians((float)(angle_deg + (72 + 72) * ((int64_t)l + 0)));
        float angle_rad2 = to_angle_in_radians((float)(angle_deg + (72 + 72) * ((int64_t)l + 1)));

        vec2 v1 = rotate_around_origo_vec2(v_base, angle_rad1);
        vec2 v2 = rotate_around_origo_vec2(v_base, angle_rad2);

        draw_line_2d((vec2[2]){v1, v2}, (color[2]){color_white, color_white}, '*');

#ifdef DEBUG
        CLEAR_LINE();
        printf("l: %zu" NEW_LINE, l);

        CLEAR_LINE();
        printf("angle_deg: %ld, angle_rad1: %.2f, angle_rad2: %.2f" NEW_LINE, angle_deg, angle_rad1, angle_rad2);

        CLEAR_LINE();
        printf("v_base.x: %.2f, v_base.y: %.2f" NEW_LINE, v_base.x, v_base.y);

        CLEAR_LINE();
        printf("v1.x: %.2f, v1.y: %.2f" NEW_LINE, v1.x, v1.y);

        CLEAR_LINE();
        printf("v2.x: %.2f, v2.y: %.2f" NEW_LINE, v2.x, v2.y);

        g_extra_lines += 5;
#endif
    }
}
