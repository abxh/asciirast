#pragma once

#include <stdint.h>

#include "canvas.h"
#include "color_encoding.h"

void draw_point(struct canvas_type* canvas, const float pixel_x, const float pixel_y, const uint32_t depth,
                const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char);

void draw_line(struct canvas_type* canvas, const float pixel_x0, const float pixel_y0, const float pixel_x1, const float pixel_y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char);
