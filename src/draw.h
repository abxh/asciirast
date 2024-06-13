#pragma once

#include "transform.h"

#include <math.h>
#include <stdint.h>

/*
Triangle drawing details:
- clockwise winding order (v0 -> v1 -> v2 is clockwise)
- top-left fill convention (of all triangle edges, top-left edge of triangle is filled.)
*/

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

void draw_point_2d(vec2 v0, char c);

void draw_line_2d(vec2 v0, vec2 v1, char c);

void draw_triangle_2d(vec2 v0, vec2 v1, vec2 v2, char c);

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
#define CAMERA_CENTER ((vec3){0, 0, 0}) // changing this doesn't do anything at the moment

void draw_point_3d(vec3 v0, char c);

void draw_line_3d(vec3 v0, vec3 v1, char c);

void draw_triangle_3d(vec3 v0, vec3 v1, vec3 v2, char c); // with backface culling, z-buffering. filled.
