#include "rgb_triangle_scene.h"
#include "draw.h"
#include "misc.h"
#include "scene.h"
#include "screen.h"
#include "transform.h"

#include <stdint.h>
#include <stdlib.h>

const scene_type g_rgb_triangle_scene = {.flags = SCENE_OPS_ON_KEY,
                                         .create = rgb_triangle_scene_create,
                                         .destroy = rgb_triangle_scene_destroy,
                                         .update = rgb_triangle_scene_update,
                                         .render = rgb_triangle_scene_render,
                                         .on_key = rgb_triangle_scene_on_key};

typedef struct {
    bool processed;
    struct dir_member {
        bool up;
        bool down;
        bool camera_left_rot;
        bool camera_right_rot;
        bool obj_left_rot;
        bool obj_right_rot;
    } dir;
} movement_info;

typedef struct {
    float angle_rad;
} rgb_triangle;

#define MOVEMENT_INFO_OBJ 0
#define RGB_TRIANGLE_OBJ 1

void** rgb_triangle_scene_create(void) {
    void** context_ptr = calloc(2, sizeof(void*));

    context_ptr[MOVEMENT_INFO_OBJ] = malloc(sizeof(movement_info));

    ((movement_info*)context_ptr[MOVEMENT_INFO_OBJ])->processed = true;
    ((movement_info*)context_ptr[MOVEMENT_INFO_OBJ])->dir = (struct dir_member){false, false, false, false, false, false};

    context_ptr[RGB_TRIANGLE_OBJ] = malloc(sizeof(rgb_triangle));
    ((rgb_triangle*)context_ptr[RGB_TRIANGLE_OBJ])->angle_rad = 0;

    g_camera_position = (vec3){0.f, 0.f, -1.75f};

    return context_ptr;
}

void rgb_triangle_scene_destroy(void** context_ptr) {
    free(context_ptr[MOVEMENT_INFO_OBJ]);
    free(context_ptr[RGB_TRIANGLE_OBJ]);
    free(context_ptr);

    g_camera_position = (vec3){0.f, 0.f, 0.f};
}

void rgb_triangle_scene_on_key(void** context_ptr, char key) {
    movement_info* info = (movement_info*)context_ptr[MOVEMENT_INFO_OBJ];

    switch (key) {
    case 'a':
        info->processed = false;
        info->dir.camera_left_rot = true;
        break;
    case 'd':
        info->processed = false;
        info->dir.camera_right_rot = true;
        break;
    case 'h':
        info->processed = false;
        info->dir.obj_left_rot = true;
        break;
    case 'l':
        info->processed = false;
        info->dir.obj_right_rot = true;
        break;
    case 'w':
        info->processed = false;
        info->dir.up = true;
        break;
    case 's':
        info->processed = false;
        info->dir.down = true;
        break;
    }
}

void rgb_triangle_scene_update(void** context_ptr) {
    movement_info* info = (movement_info*)context_ptr[MOVEMENT_INFO_OBJ];
    rgb_triangle* triangle = (rgb_triangle*)context_ptr[RGB_TRIANGLE_OBJ];

    if (info->processed) {
        return;
    }

    if (info->dir.down || info->dir.up) {
        vec3 g_camera_orientation_vec = {0.f, 0.f, 1.f}; // (forward)
        g_camera_orientation_vec = rotate_around_x_axis(g_camera_orientation_vec, g_camera_orientation[0]);
        g_camera_orientation_vec = rotate_around_y_axis(g_camera_orientation_vec, g_camera_orientation[1]);
        g_camera_orientation_vec = rotate_around_z_axis(g_camera_orientation_vec, g_camera_orientation[2]);

        g_camera_position =
            sum_vec3(g_camera_position, scaled_vec3(g_camera_orientation_vec, (-(info->dir.down) + (info->dir.up)) * 0.5f));
    }

    g_camera_orientation[1] += (-(info->dir.camera_left_rot) + (info->dir.camera_right_rot)) * 0.2f;

    triangle->angle_rad += (-(info->dir.obj_left_rot) + (info->dir.obj_right_rot)) * 0.5f;

    info->processed = true;
    info->dir = (struct dir_member){false, false, false, false, false, false};
}

void rgb_triangle_scene_render(void** context_ptr) {
    movement_info* info = (movement_info*)context_ptr[MOVEMENT_INFO_OBJ];
    rgb_triangle* triangle = (rgb_triangle*)context_ptr[RGB_TRIANGLE_OBJ];

    CLEAR_LINE();
    printf("w|a|s|d: movement, h|l: rotate triangle" NEW_LINE);
    g_extra_lines += 1;


    vec3 v1 = {-0.5f, -0.5f, 0.f};
    vec3 v2 = {0.f, 0.5f, 0.f};
    vec3 v3 = {0.5f, -0.5f, 0.f};

    v1 = rotate_around_y_axis(v1, triangle->angle_rad);
    v2 = rotate_around_y_axis(v2, triangle->angle_rad);
    v3 = rotate_around_y_axis(v3, triangle->angle_rad);

    draw_triangle_3d((vec3[3]){v1, v2, v3}, (color[3]){color_red, color_blue, color_green}, '*');
    draw_triangle_3d((vec3[3]){v1, v3, v2}, (color[3]){color_white, color_white, color_grey}, '*');
}
