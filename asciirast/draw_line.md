# draw_line.c variations

Simple:

```c
void draw_line(struct canvas_type* canvas, const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char) {
    const int64_t dx = (int64_t)x1 - (int64_t)x0;
    const int64_t dy = (int64_t)y1 - (int64_t)y0;

    assert(dx >= 0);
    assert(dy >= 0);
    assert(dx >= dy);

    if (dx != 0) {
        const float m = (float)dy / (float)dx;

        for (uint32_t i = 0; i < (uint32_t)(dx + 1); i++) {
            canvas_plot(canvas, x0 + i, (uint32_t)roundf(m * (float)(x0 + i)), depth, fg_color, bg_color, ascii_char);
        }
    }
}
```

Setup for derivation. See additionally [this video](https://www.youtube.com/watch?v=CceepU1vIKo).

```c
void draw_line(struct canvas_type* canvas, const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char) {

    const int64_t dx = (int64_t)x1 - (int64_t)x0;
    const int64_t dy = (int64_t)y1 - (int64_t)y0;

    assert(dx >= 0);
    assert(dy >= 0);
    assert(dx >= dy);

    if (dx != 0) {
        const float m = (float)dy / (float)dx;

        uint32_t y = y0;

        for (uint32_t i = 0; i < dx; i++) {
            canvas_plot(canvas, x0 + i, (uint32_t)y, depth, fg_color, bg_color, ascii_char);

            // ----> x
            // |
            // |
            // \/ y

            // (x,y) -> P0
            //   \___      } (dist_0)
            //       \--(x + 1, y_new)
            //             } (dist_1)
            //          P1

            const float y_new = m * (float)(i + 1) + (float)y0;
            const float dist_0 = y_new - (float)y;
            const float dist_1 = (float)(y + 1) - y_new;

            const float D = dist_1 - dist_0;

            if (D < 0) {
                y++;
            }
        }
    }
}
```

Integer only (inefficient) --- achieved by substiting the floating values.

```c
void draw_line(struct canvas_type* canvas, const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char) {

    const int64_t dx = (int64_t)x1 - (int64_t)x0;
    const int64_t dy = (int64_t)y1 - (int64_t)y0;


    assert(dx >= 0);
    assert(dy >= 0);
    assert(dx >= dy);

    uint32_t y = y0;

    for (uint32_t i = 0; i < dx + 1; i++) {
        canvas_plot(canvas, x0 + i, y, depth, fg_color, bg_color, ascii_char);

        const int64_t D = 2 * y * dx - 2 * dy * (i + 1) - 2 * y0 * dx + dx;

        if (D < 0) {
            y++;
        }
    }
}
```

Integer only (efficient):
```c
void draw_line(struct canvas_type* canvas, const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char) {

    const int64_t dx = (int64_t)x1 - (int64_t)x0;
    const int64_t dy = (int64_t)y1 - (int64_t)y0;

    assert(dx >= 0);
    assert(dy >= 0);
    assert(dx >= dy);

    uint32_t y = y0;
    int64_t D = dx - 2 * dy;

    for (uint32_t i = 0; i < dx + 1; i++) {
        canvas_plot(canvas, x0 + i, y, depth, fg_color, bg_color, ascii_char);

        if (D < 0) {
            y += 1;
            D += 2 * dx;
        }
        D -= 2 * dy;
    }
}
```

Integer only for (one more quadrant):
```c
void draw_line(struct canvas_type* canvas, const uint32_t x0, const uint32_t y0, const uint32_t x1, const uint32_t y1,
               const uint32_t depth, const struct rgb_type fg_color, const struct rgb_type bg_color, const char ascii_char)
{
    const int64_t dx = (int64_t)x1 - (int64_t)x0;
    int64_t dy = (int64_t)y1 - (int64_t)y0;
    int64_t dir = (dy < 0) ? -1 : 1;

    dy *= dir;

    assert(dx >= 0);
    assert(dx >= dy);

    uint32_t y = y0;
    int64_t D = dx - 2 * dy;

    for (uint32_t i = 0; i < dx + 1; i++) {
        canvas_plot(canvas, x0 + i, y, depth, fg_color, bg_color, ascii_char);

        if (D < 0) {
            y += (uint32_t)dir;
            D += 2 * dx;
        }
        D -= 2 * dy;
    }
}
```
