#pragma once

#include <stdint.h>

#include "transform.h"

#ifndef MAX_POLYGON_EDGES
#define MAX_POYLGON_EDGES 1024
#endif

void draw_point_2d(vec2 vec, char c);

void draw_line_2d(vec2 p1, vec2 p2, char c);

void draw_triangle_2d(vec2 p1, vec2 p2, vec2 p3, char c);
