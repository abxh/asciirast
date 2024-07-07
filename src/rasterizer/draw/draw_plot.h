#pragma once

#include "draw_propi.h"
#include "rasterizer/screen.h"

// internal plot routines
// ------------------------------------------------------------------------------------------------------------

static inline void plot_point(struct screen_type* screen_p, const ascii_table_type* table_p, const vec2_type v,
                              const propi_rep_type prop, const float depth) {
    screen_set_pixel_data(screen_p, vec2_floor(v),
                          (pixel_data_type){.color = prop.color, .depth = depth, .ascii_char = prop_get_ascii_char(table_p, prop)});
}
