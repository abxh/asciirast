#pragma once

#include "color.h"
#include "transform.h"

#include <stdint.h>

extern vec3 g_camera_position; // default is (0,0,0)

extern float g_camera_orientation[3]; // (rotation by x in rad, rotation by y in rad, rotation by z in rad)

/*
Triangle drawing details:
- clockwise winding order (v0 -> v1 -> v2 is clockwise)
- top-left fill convention (of all triangle edges, top-left edge of triangle is filled.)
*/

/*
world space:
                /|\ (1,0,0)
                 |   -/\ (0,0,1)
                 |   /
                 | /
(-1,0,0) <-------.--------> (1,0,0)
                /|
              /  |
           |/_   |
      (0,0,-1)  \|/ (-1, 0, 0)
*/

/* near and far plane distances for clipping */
#define Z_NEAR (0.1f)
#define Z_FAR (100.0f)

#define FOV_ANGLE_RAD (PI / 3.f) // 60 degrees

void draw_point_3d(vec3 pos[1], color color[1], char c);

void draw_line_3d(vec3 pos[2], color color[2], char c);

void draw_triangle_3d(vec3 pos[3], color color[3], char c);

/*
screen space:
              /|\ (0,1)
               |
               |
               |
(-1,0) <-------.--------> (1,0)
               |
               |
               |
              \|/ (0,-1)
*/


void draw_point_2d(vec2 pos[1], color color[1], char c);

void draw_line_2d(vec2 pos[2], color color[2], char c);

void draw_triangle_2d(vec2 pos[3], color color[3], char c);
