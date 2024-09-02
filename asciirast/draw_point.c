#include "draw.h"
#include <stdint.h>

void draw_point(struct canvas_type* canvas, const int64_t x, const int64_t y, const uint32_t depth, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char)
{

    if (0 < x && x < canvas->width && 0 < y && y < canvas->height) {
        canvas_plot(canvas, (uint32_t)x, (uint32_t)y, depth, fg_color, bg_color, ascii_char);
    }
}
