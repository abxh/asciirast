#pragma once

#include <stdbool.h>

#include "screen.h"
#include "transform.h"

static inline bool point_inside_framebuf(vec2int v0) {
    bool inside_framebuf_x = 0 <= v0.x && v0.y < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= v0.x && v0.y < SCREEN_HEIGHT;

    return inside_framebuf_x && inside_framebuf_y;
}

static inline bool point_inside_framebuf_split(int x0, int y0) {
    bool inside_framebuf_x = 0 <= x0 && x0 < SCREEN_WIDTH;
    bool inside_framebuf_y = 0 <= y0 && y0 < SCREEN_HEIGHT;

    return inside_framebuf_x && inside_framebuf_y;
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

static inline void plot_point_w_depth_unchecked_bounds(int x0, int y0, char c, float depth) {
    if (depth > g_depthbuf[y0][x0]) {
        return;
    }
    g_framebuf[y0][x0] = c;
    g_depthbuf[y0][x0] = depth;
}

static inline void plot_point_w_depth(int x0, int y0, char c, float depth) {
    if (!point_inside_framebuf_split(x0, y0) || !inside_range_float(depth, 0.f, 1.f)) {
        return;
    }
    plot_point_w_depth_unchecked_bounds(x0, y0, c, depth);
}

static inline void plot_point(int x0, int y0, char c) {
    plot_point_w_depth(x0, y0, c, 0);
}

static inline void plot_point_vec2int_w_depth_unchecked_bounds(vec2int v0, char c, float depth) {
    plot_point_w_depth_unchecked_bounds(v0.x, v0.y, c, depth);
}

static inline void plot_point_vec2int_w_depth(vec2int v0, char c, float depth) {
    plot_point_w_depth(v0.x, v0.y, c, depth);
}

static inline void plot_point_vec2int(vec2int v0, char c) {
    plot_point(v0.x, v0.y, c);
}
