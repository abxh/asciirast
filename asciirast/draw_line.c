// using the known Bresenham's line algorithm.

#include "canvas.h"
#include "clip.h"
#include "draw.h"
#include <stdint.h>

static inline void draw_straight_line_step_with_x(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                                  const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                                  const struct rgb_type bg_color, const char ascii_char)
{
    const int64_t dx = x1 - x0;

    assert(dx >= 0);
    assert(y0 == y1);

    for (uint32_t i = 0; i < dx + 1; i++) {
        canvas_plot(canvas, (uint32_t)(x0 + i), (uint32_t)y0, depth, fg_color, bg_color, ascii_char);
    }
}

static inline void draw_line_step_with_x(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                         const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                         const struct rgb_type bg_color, const char ascii_char)
{
    const int64_t dx = x1 - x0;
    const int64_t dy_initial = y1 - y0;
    const int64_t dir = (dy_initial < 0) ? -1 : 1;
    const int64_t dy = dir * dy_initial;

    assert(dx >= 0);
    assert(dx >= dy);

    int64_t y = y0;
    int64_t D = dx - 2 * dy;

    for (int64_t i = 0; i < dx + 1; i++) {
        canvas_plot(canvas, (uint32_t)(x0 + i), (uint32_t)y, depth, fg_color, bg_color, ascii_char);

        if (D < 0) {
            y += dir;
            D += 2 * dx;
        }
        D -= 2 * dy;
    }
}

static inline void draw_line_horizontal(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                        const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                        const struct rgb_type bg_color, const char ascii_char)
{
    if (x0 <= x1) {
        if (y0 == y1) {
            draw_straight_line_step_with_x(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
        }
        else {
            draw_line_step_with_x(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
        }
    }
    else {
        if (y0 == y1) {
            draw_straight_line_step_with_x(canvas, x1, y1, x0, y0, depth, fg_color, bg_color, ascii_char);
        }
        else {
            draw_line_step_with_x(canvas, x1, y1, x0, y0, depth, fg_color, bg_color, ascii_char);
        }
    }
}

static inline void draw_straight_line_step_with_y(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                                  const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                                  const struct rgb_type bg_color, const char ascii_char)
{
    const int64_t dy = y1 - y0;

    assert(dy >= 0);
    assert(x0 == x1);

    for (int64_t i = 0; i < dy + 1; i++) {
        canvas_plot(canvas, (uint32_t)x0, (uint32_t)(y0 + i), depth, fg_color, bg_color, ascii_char);
    }
}

static inline void draw_line_step_with_y(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                         const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                         const struct rgb_type bg_color, const char ascii_char)
{
    const int64_t dy = y1 - y0;
    const int64_t dx_initial = x1 - x0;
    const int64_t dir = (dx_initial < 0) ? -1 : 1;
    const int64_t dx = dir * dx_initial;

    assert(dy >= 0);
    assert(dy >= dx);

    int64_t x = x0;
    int64_t D = dy - 2 * dx;

    for (int64_t i = 0; i < dy + 1; i++) {
        canvas_plot(canvas, (uint32_t)x, (uint32_t)(y0 + i), depth, fg_color, bg_color, ascii_char);

        if (D < 0) {
            x += dir;
            D += 2 * dy;
        }
        D -= 2 * dx;
    }
}

static inline void draw_line_vertical(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                      const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                      const struct rgb_type bg_color, const char ascii_char)
{
    if (y0 <= y1) {
        if (x0 == x1) {
            draw_straight_line_step_with_y(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
        }
        else {
            draw_line_step_with_y(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
        }
    }
    else {
        if (x0 == x1) {
            draw_straight_line_step_with_y(canvas, x1, y1, x0, y0, depth, fg_color, bg_color, ascii_char);
        }
        else {
            draw_line_step_with_y(canvas, x1, y1, x0, y0, depth, fg_color, bg_color, ascii_char);
        }
    }
}

static inline int64_t int64_abs(const int64_t expr)
{
    return (expr >= 0) ? expr : -expr;
}

static inline void draw_line_unclipped(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1,
                                       const int64_t y1, const uint32_t depth, const struct rgb_type fg_color,
                                       const struct rgb_type bg_color, const char ascii_char)
{
    if (int64_abs(y1 - y0) < int64_abs(x1 - x0)) {
        draw_line_horizontal(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
    }
    else {
        draw_line_vertical(canvas, x0, y0, x1, y1, depth, fg_color, bg_color, ascii_char);
    }
}

void draw_line(struct canvas_type* canvas, const int64_t x0, const int64_t y0, const int64_t x1, const int64_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char)
{
    const struct AABB_type aabb = {.xmin = 0.f, .ymin = 0.f, .xmax = (float)(canvas->width - 1), .ymax = (float)(canvas->height - 1)};

    float x0_new = (float)x0;
    float y0_new = (float)y0;
    float x1_new = (float)x1;
    float y1_new = (float)y1;

    if (clip_line_cohen_sutherland(aabb, &x0_new, &y0_new, &x1_new, &y1_new)) {
        const int64_t x0_rounded = (int64_t)(x0_new + 0.5f);
        const int64_t y0_rounded = (int64_t)(y0_new + 0.5f);
        const int64_t x1_rounded = (int64_t)(x1_new + 0.5f);
        const int64_t y1_rounded = (int64_t)(y1_new + 0.5f);

        draw_line_unclipped(canvas, x0_rounded, y0_rounded, x1_rounded, y1_rounded, depth, fg_color, bg_color, ascii_char);
    }
}
