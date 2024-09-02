// using modified example from:
// https://en.wikipedia.org/wiki/Cohen-Sutherland_algorithm#Example_C/C++_implementation
//
// comments are left out.

#include "clip.h"

enum outcode_type {
    INSIDE = 0b0000,
    LEFT = 0b0001,
    RIGHT = 0b0010,
    BOTTOM = 0b0100,
    TOP = 0b1000,
};

static inline enum outcode_type compute_out_code(const struct AABB_type aabb, const float x, const float y)
{
    enum outcode_type code = INSIDE;

    if (x < aabb.xmin)
        code |= LEFT;
    else if (x > aabb.xmax)
        code |= RIGHT;
    if (y < aabb.ymin)
        code |= BOTTOM;
    else if (y > aabb.ymax)
        code |= TOP;

    return code;
}

bool clip_line_cohen_sutherland(const struct AABB_type aabb, float* x0_ptr, float* y0_ptr, float* x1_ptr, float* y1_ptr)
{
    enum outcode_type outcode0 = compute_out_code(aabb, *x0_ptr, *y0_ptr);
    enum outcode_type outcode1 = compute_out_code(aabb, *x1_ptr, *y1_ptr);

    bool accept = false;

    while (true) {
        if (!(outcode0 | outcode1)) {
            accept = true;
            break;
        }
        else if (outcode0 & outcode1) {
            break;
        }
        else {
            const float x0 = *x0_ptr;
            const float y0 = *y0_ptr;
            const float x1 = *x1_ptr;
            const float y1 = *y1_ptr;

            const enum outcode_type outcodeOut = outcode1 > outcode0 ? outcode1 : outcode0;

            float x, y;
            if (outcodeOut & TOP) {
                x = x0 + (x1 - x0) * (aabb.ymax - y0) / (y1 - y0);
                y = aabb.ymax;
            }
            else if (outcodeOut & BOTTOM) {
                x = x0 + (x1 - x0) * (aabb.ymin - y0) / (y1 - y0);
                y = aabb.ymin;
            }
            else if (outcodeOut & RIGHT) {
                y = y0 + (y1 - y0) * (aabb.xmax - x0) / (x1 - x0);
                x = aabb.xmax;
            }
            else if (outcodeOut & LEFT) {
                y = y0 + (y1 - y0) * (aabb.xmin - x0) / (x1 - x0);
                x = aabb.xmin;
            }

            if (outcodeOut == outcode0) {
                *x0_ptr = x;
                *y0_ptr = y;
                outcode0 = compute_out_code(aabb, x0, y0);
            }
            else {
                *x1_ptr = x;
                *y1_ptr = y;
                outcode1 = compute_out_code(aabb, x1, y1);
            }
        }
    }
    return accept;
}
