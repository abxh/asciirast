#pragma once

#include "color.h"
#include "math/vec.h"
#include "renderer.h"

#include <stdint.h>

// 2d space = {(x,y,z) where x in [-1;1] and y in [-1;1] and z_order in [0; UINT8_MAX]}
// 3d space = {(x,y,z,w) where x in [-1;1] and y in [-1;1] and z in [-Z_NEAR;-Z_FAR] and w = 1}
// Any shape outside this range is to be clipped.

// winding order      : Only triangles with counterclockwise ordered verticies is drawn. [TODO: check this]
// filling convention : top-left edges of triangle is left out. [TODO: do this correctly and expand on this]

typedef struct {
    color_type color;
    char ascii_char;
} vertix_prop_type;

typedef struct {
    vec2_type pos; // x,y
    vertix_prop_type prop;
} vertix_2d_type;

typedef struct {
    vec4_type pos; // x,y,z,w -- leave w to 1. used to make the matrix math work out.
    vertix_prop_type prop;
} vertix_3d_type;

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(struct renderer_type* obj_p, const vertix_2d_type v[1], const uint8_t z_order);

void draw_line_2d(struct renderer_type* obj_p, const vertix_2d_type v[2], const uint8_t z_order);

void draw_filled_triangle_2d(struct renderer_type* obj_p, const vertix_2d_type v[3], const uint8_t z_order);

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(struct renderer_type* obj_p, const vertix_3d_type v[1]);

void draw_line_3d(struct renderer_type* obj_p, const vertix_3d_type v[2]);

void draw_filled_triangle_3d(struct renderer_type* obj_p, const vertix_2d_type v[3]);

void draw_triangle_mesh_3d(struct renderer_type* obj_p, const size_t n, const vertix_3d_type v[n]);
