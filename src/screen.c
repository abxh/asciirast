
#include "screen.h"
#include "color.h"
#include "math/vec.h"

#include <stdint.h>
#include <stdlib.h>

#define CSI_ESC "\033["
#define CSI_MOVEUPLINES "A"
#define CSI_SHOWCURSOR "?25h"
#define CSI_HIDECURSOR "?25l"
#define CSI_CLEARLINE "2K"
#define CSI_SETBG_RGBCOLOR "48;2;"
#define CSI_SETFG_RGBCOLOR "38;2;"
#define CSI_RESETCOLOR "0m"

typedef struct screen_type {
    char framebuf[SCREEN_HEIGHT][SCREEN_WIDTH];
    float depthbuf[SCREEN_HEIGHT][SCREEN_WIDTH];
    color_type colorbuf[SCREEN_HEIGHT][SCREEN_WIDTH];
    FILE* output_stream;
} screen_type;

static inline void framebuf_clear(screen_type* this) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            this->framebuf[y][x] = ' ';
        }
    }
}

static inline void depthbuf_clear(screen_type* this) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            this->depthbuf[y][x] = 0;
        }
    }
}

static inline void colorbuf_clear(screen_type* this) {
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            this->colorbuf[y][x] = color_white;
        }
    }
}

void screen_clear(screen_type* this) {
    framebuf_clear(this);
    depthbuf_clear(this);
    colorbuf_clear(this);
}

screen_type* screen_create(FILE* output_stream) {
    screen_type* this = malloc(sizeof(screen_type));

    if (!this) {
        abort();
    }

    this->output_stream = output_stream;
    fprintf(this->output_stream, CSI_ESC CSI_HIDECURSOR);
    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        fprintf(this->output_stream, CSI_ESC CSI_CLEARLINE "\n");
    }
    screen_clear(this);

    return this;
}

void screen_destroy(screen_type* this) {
    fprintf(this->output_stream, CSI_ESC CSI_SHOWCURSOR);
    fprintf(this->output_stream, CSI_ESC CSI_RESETCOLOR);
    free(this);
}

void screen_refresh(const screen_type* this) {
    screen_type s = *this;
    fprintf(s.output_stream, CSI_ESC "%d" CSI_MOVEUPLINES, SCREEN_HEIGHT);
    fprintf(s.output_stream, "%c", '\r');

    for (size_t y = 0; y < SCREEN_HEIGHT; y++) {
        for (size_t x = 0; x < SCREEN_WIDTH; x++) {
            const size_t y_flipped = (SCREEN_HEIGHT - 1) - y;

            color_type transformed_color;
            vec3_clamp(transformed_color.as_vec3, s.colorbuf[y_flipped][x].as_vec3, (vec3_type){0.f, 0.f, 0.f},
                       (vec3_type){1.f, 1.f, 1.f});
            vec3_scale(transformed_color.as_vec3, transformed_color.as_vec3, 255.f);

            fprintf(s.output_stream, CSI_ESC CSI_SETBG_RGBCOLOR "0;0;0;" CSI_SETFG_RGBCOLOR "%03u;%03u;%03u;m",
                    (unsigned int)transformed_color.r, (unsigned int)transformed_color.g, (unsigned int)transformed_color.b);

            fprintf(s.output_stream, "%c", s.framebuf[y_flipped][x]);
        }
        fprintf(s.output_stream, "\n");
    }

    fprintf(s.output_stream, CSI_ESC CSI_RESETCOLOR);
}

void screen_set_pixel_data(screen_type* this, const vec2int_type pos, const pixel_data_type data) {
    assert(int_is_inside_range(pos[0], 0, SCREEN_WIDTH - 1));
    assert(int_is_inside_range(pos[1], 0, SCREEN_HEIGHT - 1));
    assert(int_is_inside_range(data.ascii_char, 32, 126) && "ascii char is not printable");
    assert(float_is_inside_range(data.depth, 0.f, 1.f));
    assert(vec3_is_inside_range(data.color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f}));

    const float prev_depth = this->depthbuf[pos[1]][pos[0]];
    if (data.depth < prev_depth) {
        return;
    }

    this->framebuf[pos[1]][pos[0]] = data.ascii_char;
    this->depthbuf[pos[1]][pos[0]] = data.depth;
    this->colorbuf[pos[1]][pos[0]] = data.color;
}

pixel_data_type screen_get_pixel_data(struct screen_type* this, const vec2int_type pos) {
    assert(int_is_inside_range(pos[0], 0, SCREEN_WIDTH - 1));
    assert(int_is_inside_range(pos[1], 0, SCREEN_HEIGHT - 1));

    const char ascii_char = this->framebuf[pos[1]][pos[0]];
    const float depth = this->depthbuf[pos[1]][pos[0]];
    const color_type color = this->colorbuf[pos[1]][pos[0]];

    return (pixel_data_type){.ascii_char = ascii_char, .depth = depth, .color = color};
}
