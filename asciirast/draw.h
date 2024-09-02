#pragma once

#include <stdint.h>

#include "canvas.h"
#include "color_encoding.h"

void draw_point(struct canvas_type* canvas, const int64_t x, const int64_t y, const uint32_t depth, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char);

void draw_line(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1, const int64_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char);
