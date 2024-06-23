#include "draw.h"
#include "color.h"
#include "framebuf.h"
#include "screen.h"
#include "transform.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <assert.h>

vec3 g_camera_position = {0., 0., 0.};

float g_camera_orientation[3] = {0., 0., 0.};

void draw_point_2d(vec2 pos[1], color color[1], char c) {
    plot_point_vec2int_w_specified(to_framebuf_coords(pos[0]), c, Z_NEAR, color[0]);
}

void draw_point_3d(vec3 pos[1], color color[1], char c) {
    vec3 v0c = vec3_apply_basic_camera_matrix(pos[0], g_camera_position, g_camera_orientation);
    vec4 v0p = vec3_apply_projection_matrix(v0c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);

    if (!inside_range_float(pos[0].z, Z_NEAR, Z_FAR)) {
        return;
    }
    v0p = scaled_vec4(v0p, 1.f / v0p.w);

    plot_point_vec2int_w_specified(to_framebuf_coords((vec2){v0p.x, v0p.y}), c, v0p.z, color[0]);
}

static inline void draw_line_internal(vec2int v0fb, vec2int v1fb, char c, float d0, float d1, color c0, color c1) {
    // Linear interpolation algorithm:
    // based on https://www.redblobgames.com/grids/line-drawing/#more
    int dx = abs_int(v1fb.x - v0fb.x);
    int dy = abs_int(v1fb.y - v0fb.y);
    int diagonal_dist = dx > dy ? dx : dy;

    vec2 v0fb_f = sum_vec2(to_vec2(v0fb), (vec2){0.5f, 0.5f});
    vec2 v1fb_f = sum_vec2(to_vec2(v1fb), (vec2){0.5f, 0.5f});

    for (int step = 0; step <= diagonal_dist; step++) {
        float t = (float)step / (float)diagonal_dist;
        vec2 p = lerp_vec2(v0fb_f, v1fb_f, t);
        plot_point_vec2int_w_specified((vec2int){(int)p.x, (int)p.y}, c, lerp_float(d0, d1, t), lerp_color(c0, c1, t));
    }
}

void draw_line_2d(vec2 pos[2], color color[2], char c) {
    vec2int v0fb = to_framebuf_coords(pos[0]);
    vec2int v1fb = to_framebuf_coords(pos[1]);

    // TODO: REDO this
    // if (!point_inside_framebuf(v0fb) && !point_inside_framebuf(v1fb)) {
    //     return;
    // }
    v0fb = (vec2int){.x = clamp_int(v0fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v0fb.y, 0, SCREEN_HEIGHT - 1)};
    v1fb = (vec2int){.x = clamp_int(v1fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v1fb.y, 0, SCREEN_HEIGHT - 1)};

    draw_line_internal(v0fb, v1fb, c, Z_NEAR, Z_NEAR, color[0], color[1]);
}

void draw_line_3d(vec3 pos[2], color color[2], char c) {
    vec3 v0c = vec3_apply_basic_camera_matrix(pos[0], g_camera_position, g_camera_orientation);
    vec3 v1c = vec3_apply_basic_camera_matrix(pos[1], g_camera_position, g_camera_orientation);

    vec4 v0p = vec3_apply_projection_matrix(v0c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);
    vec4 v1p = vec3_apply_projection_matrix(v1c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);

    // TODO: REDO this
    v0p = (vec4){v0p.x, v0p.y, v0p.z, clamp_float(v0p.w, Z_NEAR, Z_FAR)};
    v1p = (vec4){v1p.x, v1p.y, v1p.z, clamp_float(v1p.w, Z_NEAR, Z_FAR)};

    v0p = scaled_vec4(v0p, 1.f / v0p.w);
    v1p = scaled_vec4(v1p, 1.f / v1p.w);

    vec2int v0fb = to_framebuf_coords((vec2){v0p.x, v0p.y});
    vec2int v1fb = to_framebuf_coords((vec2){v1p.x, v1p.y});

    // TODO: REDO this
    // if (!point_inside_framebuf(v0fb) && !point_inside_framebuf(v1fb)) {
    //     return;
    // }
    v0fb = (vec2int){.x = clamp_int(v0fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v0fb.y, 0, SCREEN_HEIGHT - 1)};
    v1fb = (vec2int){.x = clamp_int(v1fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v1fb.y, 0, SCREEN_HEIGHT - 1)};

    draw_line_internal(v0fb, v1fb, c, v0p.z, v1p.z, color[0], color[1]);
}

static inline bool is_top_left_edge_of_triangle(vec2int src, vec2int dest) {
    vec2int edge = src_to_dest_vec2int(src, dest);

    bool points_right = edge.x > 0;
    bool points_up = edge.y < 0; // since y-axis points down for framebuffer

    bool is_top_edge = edge.y == 0 && points_right;
    bool is_left_edge = points_up;

    return is_top_edge || is_left_edge;
}

static inline void draw_triangle_internal(vec2int v0, vec2int v1, vec2int v2, char c, float d0, float d1, float d2, color c0, color c1,
                                          color c2) {
    // baycentric algorithm:
    // https://www.youtube.com/watch?v=k5wtuKWmV48

    /* get the bounding box of the triangle */
    int maxX = max_int(v0.x, max_int(v1.x, v2.x));
    int minX = min_int(v0.x, min_int(v1.x, v2.x));
    int maxY = max_int(v0.y, max_int(v1.y, v2.y));
    int minY = min_int(v0.y, min_int(v1.y, v2.y));

    vec2 p0 = {(float)minX + 0.5f, (float)minY + 0.5f};

    // bias to include top_left edge
    float bias0 = is_top_left_edge_of_triangle(v1, v2) ? 0 : -1;
    float bias1 = is_top_left_edge_of_triangle(v2, v0) ? 0 : -1;
    float bias2 = is_top_left_edge_of_triangle(v0, v1) ? 0 : -1;

    // vectors:
    vec2 v1_to_v2 = src_to_dest_vec2(to_vec2(v1), to_vec2(v2));
    vec2 v2_to_v0 = src_to_dest_vec2(to_vec2(v2), to_vec2(v0));
    vec2 v0_to_v1 = src_to_dest_vec2(to_vec2(v0), to_vec2(v1));
    vec2 v0_to_v2 = src_to_dest_vec2(to_vec2(v0), to_vec2(v2));

    float triangle_area_2 = cross_vec2(v0_to_v1, v0_to_v2);

    // cross product things:
    float delta_w0_col = (float)(v1.y - v2.y);
    float delta_w0_row = (float)(v2.x - v1.x);
    float w0_row = cross_vec2(v1_to_v2, src_to_dest_vec2(to_vec2(v1), p0)) + bias0;

    float delta_w1_col = (float)(v2.y - v0.y);
    float delta_w1_row = (float)(v0.x - v2.x);
    float w1_row = cross_vec2(v2_to_v0, src_to_dest_vec2(to_vec2(v2), p0)) + bias1;

    float delta_w2_col = (float)(v0.y - v1.y);
    float delta_w2_row = (float)(v1.x - v0.x);
    float w2_row = cross_vec2(v0_to_v1, src_to_dest_vec2(to_vec2(v0), p0)) + bias2;

    for (int y = minY; y <= maxY; y++) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = minX; x <= maxX; x++) {
            bool is_inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
            if (is_inside_triangle) {
                float alpha = w0 / triangle_area_2;
                float beta = w1 / triangle_area_2;
                float gamma = w2 / triangle_area_2;

                plot_point_w_specified_unchecked_bounds(
                    x, y, c, alpha * d0 + beta * d1 + gamma * d2,
                    sum_color(sum_color(scaled_color(c0, alpha), scaled_color(c1, beta)), scaled_color(c2, gamma)));
            }
            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;
    }
}

void draw_triangle_2d(vec2 pos[3], color color[3], char c) {
    vec2int v0fb = to_framebuf_coords(pos[0]);
    vec2int v1fb = to_framebuf_coords(pos[1]);
    vec2int v2fb = to_framebuf_coords(pos[2]);

    // TODO: REDO this
    // if (!point_inside_framebuf(v0fb) && !point_inside_framebuf(v1fb) && !point_inside_framebuf(v2fb)) {
    //     return;
    // }
    v0fb = (vec2int){.x = clamp_int(v0fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v0fb.y, 0, SCREEN_HEIGHT - 1)};
    v1fb = (vec2int){.x = clamp_int(v1fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v1fb.y, 0, SCREEN_HEIGHT - 1)};
    v2fb = (vec2int){.x = clamp_int(v2fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v2fb.y, 0, SCREEN_HEIGHT - 1)};

    // backface culling is done automatically with cross product calculations:
    draw_triangle_internal(v0fb, v1fb, v2fb, c, Z_NEAR, Z_NEAR, Z_NEAR, color[0], color[1], color[2]);
}

void draw_triangle_3d(vec3 pos[3], color color[3], char c) {
    vec3 v0c = vec3_apply_basic_camera_matrix(pos[0], g_camera_position, g_camera_orientation);
    vec3 v1c = vec3_apply_basic_camera_matrix(pos[1], g_camera_position, g_camera_orientation);
    vec3 v2c = vec3_apply_basic_camera_matrix(pos[2], g_camera_position, g_camera_orientation);

    vec4 v0p = vec3_apply_projection_matrix(v0c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);
    vec4 v1p = vec3_apply_projection_matrix(v1c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);
    vec4 v2p = vec3_apply_projection_matrix(v2c, FOV_ANGLE_RAD, ASPECT_RATIO, Z_NEAR, Z_FAR);

    // TODO: REDO this
    v0p = (vec4){v0p.x, v0p.y, v0p.z, clamp_float(v0p.w, Z_NEAR, Z_FAR)};
    v1p = (vec4){v1p.x, v1p.y, v1p.z, clamp_float(v1p.w, Z_NEAR, Z_FAR)};
    v2p = (vec4){v2p.x, v2p.y, v2p.z, clamp_float(v2p.w, Z_NEAR, Z_FAR)};

    v0p = scaled_vec4(v0p, 1.f / v0p.w);
    v1p = scaled_vec4(v1p, 1.f / v1p.w);
    v2p = scaled_vec4(v2p, 1.f / v2p.w);

    vec2int v0fb = to_framebuf_coords((vec2){v0p.x, v0p.y});
    vec2int v1fb = to_framebuf_coords((vec2){v1p.x, v1p.y});
    vec2int v2fb = to_framebuf_coords((vec2){v2p.x, v2p.y});

    // TODO: REDO this
    if (!point_inside_framebuf(v0fb) && !point_inside_framebuf(v1fb) && !point_inside_framebuf(v2fb)) {
        return;
    }
    v0fb = (vec2int){.x = clamp_int(v0fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v0fb.y, 0, SCREEN_HEIGHT - 1)};
    v1fb = (vec2int){.x = clamp_int(v1fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v1fb.y, 0, SCREEN_HEIGHT - 1)};
    v2fb = (vec2int){.x = clamp_int(v2fb.x, 0, SCREEN_WIDTH - 1), .y = clamp_int(v2fb.y, 0, SCREEN_HEIGHT - 1)};

    // backface culling is done automatically with cross product calculations:
    draw_triangle_internal(v0fb, v1fb, v2fb, c, v0p.z, v1p.z, v2p.z, color[0], color[1], color[2]);
}
