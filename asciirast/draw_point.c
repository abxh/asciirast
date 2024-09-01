#include "draw.h"
#include <stdint.h>

void draw_point(struct canvas_type* canvas, const uint32_t x, const uint32_t y, const uint32_t depth, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char)
{

    if (x < canvas->width && y < canvas->height) {
        canvas_plot(canvas, x, y, depth, fg_color, bg_color, ascii_char);
    }
}
