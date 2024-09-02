#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "color_encoding.h"

struct canvas_type {
    uint32_t* fg_color_values;
    uint32_t* bg_color_values;
    uint32_t* depth_values;
    char* ascii_char_values;

    uint32_t width;
    uint32_t height;

    uint32_t default_fg_color;
    uint32_t default_bg_color;
    char default_ascii_char;
};

struct canvas_type* canvas_create(const uint32_t width, const uint32_t height, const struct rgb_type default_fg_color,
                                  const struct rgb_type default_bg_color, const char default_ascii_char);

void canvas_destroy(struct canvas_type* this);

void canvas_print_formatted(const struct canvas_type* this, FILE* out);

void canvas_print_formatted_wo_bg(const struct canvas_type* this, FILE* out);

void canvas_clear(struct canvas_type* this);

void canvas_plot(struct canvas_type* this, const uint32_t x, const uint32_t y, const uint32_t depth, const struct rgb_type fg_color,
                 const struct rgb_type bg_color, const char ascii_char);

const uint32_t* canvas_get_raw_fg_color_values(const struct canvas_type* this);

const uint32_t* canvas_get_raw_bg_color_values(const struct canvas_type* this);

const uint32_t* canvas_get_raw_depth_values(const struct canvas_type* this);

const char* canvas_get_raw_ascii_char_values(const struct canvas_type* this);
