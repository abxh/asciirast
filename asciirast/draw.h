#pragma once

#include <stdint.h>

#include "canvas.h"
#include "color_encoding.h"

void draw_point(struct canvas_type* canvas, const uint32_t x, const uint32_t y, const struct rgb_type fg_color,
                struct rgb_type bg_color, const char ascii_char, const uint32_t z_order);
