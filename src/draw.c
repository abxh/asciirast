#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "draw.h"
#include "screen.h"
#include "transform.h"

void plot(size_t x, size_t y, char c) {
    screen_buf[y][x] = c;
}

int to_index(float x, size_t length) {
    return (clamp(x, -1., 1.) + 1.) / 2 * (length - 1);
}

void draw_point(float xf, float yf, char c) {
    size_t x = to_index(xf, SCREEN_WIDTH);
    size_t y = to_index(yf, SCREEN_HEIGHT);

    plot(x, y, c);
}
