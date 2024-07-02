#pragma once

#include "color.h"
#include "math/mat4x4.h"
#include "math/vec.h"
#include "screen.h"

#include <stdint.h>

// 2d space = {(x,y,z) where x in [-1;1] and y in [-1;1] and z_order in [0; UINT8_MAX]}
// 3d space = {(x,y,z,w) where x in [-1;1] and y in [-1;1] and z in [Z_NEAR;Z_FAR] and w = 1}
// Any shape outside this range is clipped.

// winding order      : Only triangles with clockwise ordered verticies is drawn.
// filling convention : top-left edges of triangle is filled. bottom-right edges is left out.

// Note the use of static variables internally. Rendering is done with a fixed pipeline. Is neither made to be thread-safe.

typedef struct {
    union {
        struct {
            float x;
            float y;
        };
        vec2_type pos;
    };
    color_type color;
    char ascii_char;
} vertix_2d_type;

typedef struct {
    union {
        struct {
            float x;
            float y;
            float z;
            float w;
        };
        vec4_type pos;
    };
    color_type color;
    char ascii_char;
} vertix_3d_type;

void renderer_init(struct screen_type* screen_context_p, const mat4x4_type model_view_perspective_matrix, const size_t n,
                   const char acsii_palette[n]);

void renderer_deinit(void);

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(const vertix_2d_type v[1], const uint8_t z_order);

void draw_line_2d(const vertix_2d_type v[2], const uint8_t z_order);

void draw_filled_triangle_2d(const vertix_2d_type v[3], const uint8_t z_order);

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(const vertix_3d_type v[1]);

void draw_line_3d(const vertix_3d_type v[2]);

void draw_filled_triangle_3d(const vertix_2d_type v[3]);

void draw_triangle_mesh_3d(const size_t n, const vertix_3d_type v[n]);
