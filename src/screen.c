
#include "screen.h"
#include "color.h"

#include <stdint.h>

#define CSI_ESC "\033["
#define CSI_MOVEUPLINES "A"
#define CSI_SHOWCURSOR "?25h"
#define CSI_HIDECURSOR "?25l"
#define CSI_CLEARLINE "2K"
#define CSI_SETCOLOR_INITIALS "38;2;"
#define CSI_RESETCOLOR "0m"

static char framebuf[FRAMEBUF_HEIGHT][FRAMEBUF_WIDTH];
static float depthbuf[FRAMEBUF_HEIGHT][FRAMEBUF_WIDTH];
static color_type colorbuf[FRAMEBUF_HEIGHT][FRAMEBUF_WIDTH];

static char color_str_buf_r[sizeof("000")];
static char color_str_buf_g[sizeof("000")];
static char color_str_buf_b[sizeof("000")];
static FILE* output_stream;

static inline void framebuf_clear(void) {
    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        for (size_t x = 0; x < FRAMEBUF_WIDTH; x++) {
            framebuf[y][x] = DEFAULT_CHAR;
        }
    }
}

static inline void depthbuf_clear(void) {
    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        for (size_t x = 0; x < FRAMEBUF_WIDTH; x++) {
            depthbuf[y][x] = 0;
        }
    }
}

static inline void colorbuf_clear(void) {
    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        for (size_t x = 0; x < FRAMEBUF_WIDTH; x++) {
            colorbuf[y][x] = DEFAULT_COLOR;
        }
    }
}

void screen_clear(void) {
    framebuf_clear();
    depthbuf_clear();
    colorbuf_clear();
}

void screen_init(FILE* stream) {
    output_stream = stream;

    fprintf(output_stream, CSI_ESC CSI_HIDECURSOR);

    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        fprintf(output_stream, CSI_ESC CSI_CLEARLINE "\n");
    }
    
    screen_clear();
}

void screen_deinit(void) {
    fprintf(output_stream, CSI_ESC CSI_SHOWCURSOR);
    fprintf(output_stream, CSI_ESC CSI_RESETCOLOR);
}

void screen_refresh(void) {
    fprintf(output_stream, CSI_ESC "%d" CSI_MOVEUPLINES, FRAMEBUF_HEIGHT);
    putchar('\r');

    for (size_t y = 0; y < FRAMEBUF_HEIGHT; y++) {
        for (size_t x = 0; x < FRAMEBUF_WIDTH; x++) {
            const size_t y_flipped = (FRAMEBUF_HEIGHT - 1) - y;

            const color_type current_color = clamp_color(colorbuf[y_flipped][x], color_black, color_white);
            snprintf(color_str_buf_r, sizeof(color_str_buf_r), "%03hhu", (uint8_t)(current_color.r * 255.f));
            snprintf(color_str_buf_g, sizeof(color_str_buf_b), "%03hhu", (uint8_t)(current_color.g * 255.f));
            snprintf(color_str_buf_b, sizeof(color_str_buf_g), "%03hhu", (uint8_t)(current_color.b * 255.f));
            fprintf(output_stream, CSI_ESC CSI_SETCOLOR_INITIALS "%s;%s;%s;m", color_str_buf_r, color_str_buf_g, color_str_buf_b);

            putchar(framebuf[y_flipped][x]);
        }
        putchar('\n');
    }

    fprintf(output_stream, CSI_ESC CSI_RESETCOLOR);
}

void screen_set_pixel_data(const vec2int_type framebuf_pos, const pixel_data_type data) {
    assert(inside_range_int(framebuf_pos.x, 0, FRAMEBUF_WIDTH - 1));
    assert(inside_range_int(framebuf_pos.y, 0, FRAMEBUF_HEIGHT - 1));
    assert(inside_range_int(data.ascii_char, 32, 126) && "ascii char is not printable");
    assert(inside_range_float(data.depth, 0.f, 1.f));
    assert(inside_range_color(data.color, color_black, color_white));

    const float prev_depth = depthbuf[framebuf_pos.y][framebuf_pos.x];
    if (data.depth < prev_depth) {
        return;
    }

    framebuf[framebuf_pos.y][framebuf_pos.x] = data.ascii_char;
    depthbuf[framebuf_pos.y][framebuf_pos.x] = data.depth;
    colorbuf[framebuf_pos.y][framebuf_pos.x] = data.color;
}

pixel_data_type screen_get_pixel_data(const vec2int_type framebuf_pos) {
    assert(inside_range_int(framebuf_pos.x, 0, FRAMEBUF_WIDTH - 1));
    assert(inside_range_int(framebuf_pos.y, 0, FRAMEBUF_HEIGHT - 1));

    const char ascii_char = framebuf[framebuf_pos.y][framebuf_pos.x];
    const float depth = depthbuf[framebuf_pos.y][framebuf_pos.x];
    const color_type color = colorbuf[framebuf_pos.y][framebuf_pos.x];

    return (pixel_data_type){.ascii_char = ascii_char, .depth = depth, .color = color};
}
