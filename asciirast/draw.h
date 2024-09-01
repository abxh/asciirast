#pragma once

#include <stdint.h>

#include "canvas.h"
#include "color_encoding.h"

void draw_point(struct canvas_type* canvas, const uint32_t x, const uint32_t y, const uint32_t depth, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char);
