#pragma once

#include "color.h"
#include "transform.h"

#define Z_NEAR (0.1f)
#define Z_FAR (100.0f)
#define FOV_ANGLE_RAD (FROM_ANGLE_DEG_TO_ANGLE_RAD(60))

// winding order      : Only triangles with clockwise order [for v0 -> v1 -> p2] is drawn.
// filling convention : top-left edge of triangle is filled. bottom-right is left out.

// 2d space = {(x,y) where x in [-1;1] and y in [-1;1]}
// 3d space = {(x,y,z) where x in [-1;1] and y in [-1;1] and z in [Z_NEAR;Z_FAR]}
// Any shape outside this range is clipped.

extern vec3_type g_camera_position;

typedef struct {
    union {
        struct {
            float x_axis_angle_rad;
            float y_axis_angle_rad;
            float z_axis_angle_rad;
        };
        vec3_type as_vec3;
        float as_float3[3];
    };
} camera_orientation_type;

extern camera_orientation_type g_camera_orientation;

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(vec2_type v0, char c);

void draw_point_2d_w_color(vec2_type v0, color_type color0, char c);

void draw_line_2d(vec2_type v0, vec2_type v1, char c);

void draw_line_2d_w_color(vec2_type v0, vec2_type v1, color_type color0, char c);

void draw_line_2d_w_interpolated_color(vec2_type v[2], color_type color[2], char c);

void draw_filled_triangle_2d(vec2_type v0, vec2_type v1, vec2_type p2, char c);

void draw_filled_triangle_2d_w_color(vec2_type v0, vec2_type v1, vec2_type p2, color_type color0, char c);

void draw_filled_triangle_2d_w_interpolated_color(vec2_type vertix_pos[3], color_type vertix_color[3], char c);

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(vec3_type v0, char c);

void draw_point_3d_w_color(vec3_type v0, color_type color0, char c);

void draw_line_3d(vec3_type v0, vec3_type v1, char c);

void draw_line_3d_w_color(vec3_type v0, vec3_type v1, color_type color0, char c);

void draw_line_3d_w_interpolated_color(vec3_type v[2], color_type color[2], char c);

void draw_filled_triangle_3d(vec3_type v0, vec3_type v1, vec3_type p2, char c);

void draw_filled_triangle_3d_w_color(vec3_type v0, vec3_type v1, vec3_type p2, color_type color0, char c);

void draw_filled_triangle_3d_w_interpolated_color(vec3_type vertix_pos[3], color_type vertix_color[3], char c);
