#pragma once

#include <stdint.h>

#include "transform.h"

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

void draw_point_2d(vec2 vec, char c);

void draw_line_2d(vec2 p1, vec2 p2, char c);

void draw_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c);

void draw_filled_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c);


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

/* Fixed details:
    - camera center: (0, 0, 0)
    - fov angle: 60.
    - counterclockwise winding order: p1 -> p2 -> p3
*/
void draw_filled_triangle_3d(vec3 p1, vec3 p2, vec3 p3, char c); // with backface culling
