#pragma once

#include <stdbool.h>

#include "color.h"
#include "screen.h"
#include "transform.h"

static inline bool point_inside_framebuf(vec2int v0) {
    return inside_range_int(v0.x, 0, SCREEN_WIDTH - 1) && inside_range_int(v0.y, 0, SCREEN_HEIGHT - 1);
}

static inline bool point_inside_framebuf_split(int x0, int y0) {
    return inside_range_int(x0, 0, SCREEN_WIDTH - 1) && inside_range_int(y0, 0, SCREEN_HEIGHT - 1);
}

static inline int to_framebuf_x(float screen_x) {
    return (int)((screen_x + 1.f) / 2.f * (SCREEN_WIDTH - 1));
}

static inline int to_framebuf_y(float screen_y) {
    return (int)((-screen_y + 1.f) / 2.f * (SCREEN_HEIGHT - 1));
}

static inline vec2int to_framebuf_coords(vec2 v) {
    return (vec2int){.x = to_framebuf_x(v.x), .y = to_framebuf_y(v.y)};
}

static inline void plot_point_w_specified_unchecked_bounds(int x0, int y0, char c, float depth, color color) {
    if (depth > g_depthbuf[y0][x0]) {
        return;
    }
    g_framebuf[y0][x0] = c;
    g_depthbuf[y0][x0] = depth;
    g_colorbuf[y0][x0] = to_colorint_str(color);
}

static inline void plot_point_w_specified(int x0, int y0, char c, float depth, color color) {
    if (!point_inside_framebuf_split(x0, y0) || !inside_range_float(depth, 0.f, 1.f)) {
        return;
    }
    plot_point_w_specified_unchecked_bounds(x0, y0, c, depth, color);
}

static inline void plot_point(int x0, int y0, char c) {
    plot_point_w_specified(x0, y0, c, 0, (color){.r = 1.f, .g = 1.f, .b = 1.f});
}

static inline void plot_point_vec2int_w_specified(vec2int v0, char c, float depth, color color) {
    plot_point_w_specified_unchecked_bounds(v0.x, v0.y, c, depth, color);
}

static inline void plot_point_vec2int(vec2int v0, char c) {
    plot_point(v0.x, v0.y, c);
}
