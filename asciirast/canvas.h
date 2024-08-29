#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "color_encoding.h"

struct canvas_type {
    char* ascii_char_values;
    uint32_t* fg_color_values;
    uint32_t* bg_color_values;
    float* depth_values;

    uint32_t w;
    uint32_t h;
    uint32_t default_fg_color;
    uint32_t default_bg_color;
    char default_ascii_char;
};

struct canvas_type* canvas_create(const uint32_t w, const uint32_t h, const uint32_t default_fg_rgb, const uint32_t default_bg_rgb,
                                  const char default_ascii_char);

void canvas_destroy(struct canvas_type* this);

const uint32_t* canvas_get_raw_fg_color_values(const struct canvas_type* this);

const uint32_t* canvas_get_raw_bg_color_values(const struct canvas_type* this);

const char* canvas_get_raw_ascii_char_values(const struct canvas_type* this);

const float* canvas_get_raw_depth_values(const struct canvas_type* this);

void canvas_print_formatted(const struct canvas_type* this, FILE* out);

void canvas_print_formatted_wo_bg(const struct canvas_type* this, FILE* out);

void canvas_clear(struct canvas_type* this);

void canvas_plot(struct canvas_type* this, const uint32_t x, const uint32_t y, const float d, const struct rgb_type fg_rgb,
                 const struct rgb_type bg_rgb, const char ascii_char);
