#pragma once

#include "color.h"
#include "transform.h"

#include <stdint.h>

#define Z_NEAR 0.1f
#define Z_FAR 100.0f
#define FOV_ANGLE_RAD (FROM_ANGLE_DEG_TO_ANGLE_RAD(60))

// 2d space = {(x,y) where x in [-1;1] and y in [-1;1]}
// 3d space = {(x,y,z) where x in [-1;1] and y in [-1;1] and z in [Z_NEAR;Z_FAR]}
// Any shape outside this range is clipped.

// winding order      : Only triangles with clockwise ordered verticies is drawn.
// filling convention : top-left edges of triangle is filled. bottom-right edges is left out.

extern vec3_type g_camera_position;

typedef struct {
    float x_axis_angle_rad;
    float y_axis_angle_rad;
    float z_axis_angle_rad;
} camera_orientation_type;

extern camera_orientation_type g_camera_orientation;

// 2d - draw point
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(const vec2_type v0, const char c);

void draw_point_2d_w_color(const vec2_type v0, const color_type color0, const char c);

void draw_point_2d_w_color_and_z_order(const vec2_type v0, const color_type color0, const uint8_t z_order, const char c);

// 2d - draw line
// ------------------------------------------------------------------------------------------------------------

void draw_line_2d(const vec2_type v0, const vec2_type v1, const char c);

void draw_line_2d_w_color(const vec2_type v0, const vec2_type v1, const color_type color0, const char c);

void draw_line_2d_w_color_and_z_order(const vec2_type v0, const vec2_type v1, const color_type color0, const uint8_t z_order,
                                      const char c);

void draw_line_2d_w_interpolated_color(const vec2_type v[2], const color_type color[2], const char c);

void draw_line_2d_w_interpolated_color_and_z_order(const vec2_type v[2], const color_type color[2], const uint8_t z_order,
                                                   const char c);

// 2d - draw triangle
// ------------------------------------------------------------------------------------------------------------

void draw_filled_triangle_2d(const vec2_type v0, const vec2_type v1, const vec2_type v2, const char c);

void draw_filled_triangle_2d_w_color(const vec2_type v0, const vec2_type v1, const vec2_type v2, const color_type color0,
                                     const char c);

void draw_filled_triangle_2d_w_color_and_z_order(const vec2_type v0, const vec2_type v1, const vec2_type v2, const color_type color0,
                                                 const uint8_t z_order, const char c);

void draw_filled_triangle_2d_w_interpolated_color(const vec2_type v[3], const color_type color[3], const char c);

void draw_filled_triangle_2d_w_interpolated_color_and_z_order(const vec2_type v[3], const color_type color[3], const uint8_t z_order,
                                                              const char c);
// 3d - draw point
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(const vec3_type v0, const char c);

void draw_point_3d_w_color(const vec3_type v0, const color_type color0, const char c);

// 3d - draw line
// ------------------------------------------------------------------------------------------------------------

void draw_line_3d(const vec3_type v0, const vec3_type v1, const char c);

void draw_line_3d_w_color(const vec3_type v0, const vec3_type v1, const color_type color0, const char c);

void draw_line_3d_w_interpolated_color(const vec3_type v[2], const color_type color[2], const char c);
