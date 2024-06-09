#pragma once

#include <math.h>
#include <stdint.h>

#include "transform.h"

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

static const vec3 camera_center = {0, 0, -1};
static const float camera_near = 1.f;
static const float camera_far = INFINITY; // for now

// Winding rule, order of verticies: p1 -> p2 -> p3 is clockwise.
// (use right-hand rule to find if triangle is facing front or back)

// with backface culling
void draw_filled_triangle_3d(vec3 p1, vec3 p2, vec3 p3, char c);
