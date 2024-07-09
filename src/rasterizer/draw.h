#pragma once

#include "math/vec.h"

#include "rasterizer/color.h"

#include <stdint.h>

// 2d space = {(x,y,z) where x in [-1;1] and y in [-1;1] and z_order in [0; UINT8_MAX]}
// 3d space = {(x,y,z,w) where x in [-1;1] and y in [-1;1] and z in [-Z_NEAR;-Z_FAR] and w = 1}

// winding order: counter-clockwise pointing to the camera for (v0 -> v1 -> v2)

typedef struct {
    color_type color;
    char ascii_char;
} prop_type;

struct renderer_type;

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(struct renderer_type* obj_p, const vec2_type v[1], const prop_type prop[1], const uint8_t z_order);

void draw_line_2d(struct renderer_type* obj_p, const vec2_type v[2], const prop_type prop[2], const uint8_t z_order);

void draw_edge_2d(struct renderer_type* obj_p, const vec2_type v[2], const color_type color0, const uint8_t z_order);

void draw_filled_triangle_2d(struct renderer_type* obj_p, const vec2_type v[3], const prop_type prop[3], const uint8_t z_order);

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(struct renderer_type* obj_p, const vec4_type v[1], const prop_type prop[1]);

void draw_line_3d(struct renderer_type* obj_p, const vec4_type v[2], const prop_type prop[2]);

void draw_filled_triangle_3d(struct renderer_type* obj_p, const vec4_type v[3], const prop_type porp[3]);

void draw_triangle_mesh_3d(struct renderer_type* obj_p, const size_t n, const vec4_type v[n], const prop_type prop[n]);
