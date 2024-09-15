#include "draw.h"
#include <stdint.h>

void draw_point(struct canvas_type* canvas, const float pixel_x, const float pixel_y, const uint32_t depth, const struct rgb_type fg_color,
                const struct rgb_type bg_color, const char ascii_char)
{
    const uint32_t x = (uint32_t)(pixel_x + 0.5f);
    const uint32_t y = (uint32_t)(pixel_y + 0.5f);

    if (0 < x && x < canvas->width && 0 < y && y < canvas->height) {
        canvas_plot(canvas, x, y, depth, fg_color, bg_color, ascii_char);
    }
}
