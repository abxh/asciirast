
#include "diamond_triangle_scene.h"
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
} diamond_triangle;

const scene_type g_diamond_triangle_scene = {.flags = SCENE_OPS_NOP,
                                             .create = diamond_triangle_scene_create,
                                             .destroy = diamond_triangle_scene_destroy,
                                             .update = diamond_triangle_scene_update,
                                             .render = diamond_triangle_scene_render};

#define TRIANGLE_OBJ 0

void** diamond_triangle_scene_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));

    context_ptr[TRIANGLE_OBJ] = malloc(sizeof(diamond_triangle));

    ((diamond_triangle*)context_ptr[TRIANGLE_OBJ])->angle_deg = 0;

    return context_ptr;
}

void diamond_triangle_scene_destroy(void** context_ptr) {
    free(context_ptr[TRIANGLE_OBJ]);
    free(context_ptr);
}

void diamond_triangle_scene_update(void** context_ptr) {
    diamond_triangle* triangle_ptr = (diamond_triangle*)context_ptr[TRIANGLE_OBJ];
    triangle_ptr->angle_deg += 10;
}

void diamond_triangle_scene_render(void** context_ptr) {
    diamond_triangle* triangle_ptr = (diamond_triangle*)context_ptr[TRIANGLE_OBJ];
    int64_t angle_deg = triangle_ptr->angle_deg;
    float angle_rad = to_angle_in_radians((float)-angle_deg);

    const vec3 shift = {0, 0, 2.f};

    vec3 v1 = {0.5, -0, 0.f};
    vec3 v2 = {-0.5, -0, 0.f};
    vec3 v_top = {0, 1.f, 0.f};
    vec3 v_bottom = {0, -1.f, 0.f};

    v1 = rotate_around_y_axis(v1, angle_rad);
    v2 = rotate_around_y_axis(v2, angle_rad);

    v1 = sum_vec3(v1, shift);
    v2 = sum_vec3(v2, shift);
    v_top = sum_vec3(v_top, shift);
    v_bottom = sum_vec3(v_bottom, shift);

    draw_triangle_3d((vec3[3]){v1, v2, v_bottom}, (color[3]){color_white, color_white, color_white}, '*');
    draw_triangle_3d((vec3[3]){v1, v_bottom, v2}, (color[3]){color_white, color_white, color_white}, '.');

    draw_triangle_3d((vec3[3]){v1, v_top, v2}, (color[3]){color_white, color_white, color_white}, '.');
    draw_triangle_3d((vec3[3]){v1, v2, v_top}, (color[3]){color_white, color_white, color_white}, '*');

    vec3 v_top_to_v1 = src_to_dest_vec3(v_top, v1);
    vec3 v_top_to_v2 = src_to_dest_vec3(v_top, v2);

    vec3 normal_top = cross_vec3(v_top_to_v1, v_top_to_v2);
    vec3 top_center = scaled_vec3(sum_vec3(v_top, sum_vec3(v1, v2)), 1.f / 3.f);

    draw_line_3d((vec3[2]){top_center, sum_vec3(top_center, normal_top)}, (color[2]){color_white, color_white}, '-');

    vec3 v_bottom_to_v1 = src_to_dest_vec3(v_bottom, v1);
    vec3 v_bottom_to_v2 = src_to_dest_vec3(v_bottom, v2);

    vec3 normal_bottom = cross_vec3(v_bottom_to_v1, v_bottom_to_v2);
    vec3 bottom_center = scaled_vec3(sum_vec3(v_bottom, sum_vec3(v1, v2)), 1.f / 3.f);

    draw_line_3d((vec3[2]){bottom_center, sum_vec3(bottom_center, normal_bottom)}, (color[2]){color_white, color_white}, '-');

#ifdef DEBUG
    draw_point_3d(&v1, &color_white, '1');
    draw_point_3d(&v2, &color_white, '2');
    draw_point_3d(&v_bottom, &color_white, 'B');
    draw_point_3d(&v_top, &color_white, 'T');

    CLEAR_LINE();
    printf("angle_rad: %.2f" NEW_LINE, angle_rad);
    g_extra_lines += 1;
#endif
}
