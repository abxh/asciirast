#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include "cube_scene.h"
#include "draw.h"
#include "scenes/scene.h"
#include "screen.h"
#include "transform.h"

#ifdef DEBUG
#include "misc.h"
#include <stdio.h>
#endif

typedef struct {
    int64_t angle_deg;
} cube;

scene_type cube_scene = {
    .flags = SCENE_OPS_NOP, .create = cube_scene_create, .destroy = cube_scene_destroy, .update = cube_scene_update};

// clang-format off

// order:
//   1 - 2
//  /.  /.
// 3 - 4 .
// . . . .
// . 5 - 6
//  /  ./
// 7 - 8

static const vec3 shifted_verticies[8] = {
      {0, 1, 1}, {1, 1, 1},   // 1, 2
    {0, 1, 0}, {1, 1, 0},     // 3, 4

        {0, 0, 1}, {1, 0, 1}, // 5, 6
    {0, 0, 0}, {1, 0, 0},     // 7, 8
};

static vec3 verticies[8];

static bool adjacency_list[8][8] = {
    {0, 1, 1, 0, 1, 0, 0, 0}, // 1: 2, 3, 5
    {1, 0, 0, 1, 0, 1, 0, 0}, // 2: 1, 4, 6
    {1, 0, 0, 1, 0, 0, 1, 0}, // 3: 1, 4, 7
    {0, 1, 1, 0, 0, 0, 0, 1}, // 4: 2, 3, 8
    {1, 0, 0, 0, 0, 1, 1, 0}, // 5: 1, 6, 7
    {0, 1, 0, 0, 1, 0, 0, 1}, // 6: 2, 5, 8
    {0, 0, 1, 0, 1, 0, 0, 1}, // 7: 3, 5, 8
    {0, 0, 0, 1, 0, 1, 1, 0}, // 8: 4, 6, 7
};

// clang-format on

#define CUBE_OBJ 0

void** cube_scene_create(void) {
    void** context_ptr = calloc(1, sizeof(void*));

    context_ptr[CUBE_OBJ] = malloc(sizeof(cube));
    ((cube*)context_ptr[CUBE_OBJ])->angle_deg = 0;

    for (size_t i = 0; i < 8; i++) {
        verticies[i] = sum_vec3(shifted_verticies[i], (vec3){-0.5f, -0.5f, -0.5f});
    }

    return context_ptr;
}

void cube_scene_destroy(void** context_ptr) {
    free(context_ptr[CUBE_OBJ]);
    free(context_ptr);
}

static int stack[16];
static bool visited[8];

void cube_scene_update(void** context_ptr) {
    cube* cube_ptr = (cube*)context_ptr[CUBE_OBJ];
    int64_t angle_deg = cube_ptr->angle_deg;

    size_t count = 0;
    for (size_t i = 0; i < sizeof(stack) / sizeof(*stack); i++) {
        stack[i] = -1;
    }
    for (size_t i = 0; i < 8; i++) {
        visited[i] = false;
    }

#ifdef DEBUG
    printf("edges:\n");
    size_t edge_count = 0;
#endif
    float angle_rad = to_angle_in_radians(-angle_deg);
    static float fov_angle_rad = M_PI / 3;
    static vec3 shift = {0, 0, 1.75f};

    stack[count++] = 0;

    while (count > 0) {
        int current = stack[--count];
        visited[current] = true;

        for (int i = 0; i < 8; i++) {
            if (!adjacency_list[current][i] || visited[i]) {
                continue;
            }

            vec3 v1_3d = sum_vec3(rotate_around_y_axis(verticies[current], angle_rad), shift);
            vec2 v1 = vec3_projected_as_vec2(v1_3d, fov_angle_rad, aspect_ratio);

            vec3 v2_3d = sum_vec3(rotate_around_y_axis(verticies[i], angle_rad), shift);
            vec2 v2 = vec3_projected_as_vec2(v2_3d, fov_angle_rad, aspect_ratio);

            draw_line_2d(v1, v2, '.');
#ifdef DEBUG
            CLEAR_LINE();
            printf("%zu: (%.2f, %.2f) -> (%.2f, %2.f)\n", ++edge_count, v1.x, v1.y, v2.x, v2.y);
#endif
            stack[count++] = i;
        }
    }
#ifdef DEBUG
    CLEAR_LINE();
    MOVE_UP_LINES(14);
#endif

    if (angle_deg == 360) {
        cube_ptr->angle_deg = 0;
    }
    cube_ptr->angle_deg += 10;
}
