#include "draw.h"
#include <stdint.h>

void draw_point(struct canvas_type* canvas, const uint32_t x, const uint32_t y, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char, const uint32_t z_order)
{

    if (x >= canvas->w || y >= canvas->h) {
        return;
    }
    canvas_plot(canvas, x, y, (float)z_order / (float)UINT32_MAX, fg_color, bg_color, ascii_char);
}
