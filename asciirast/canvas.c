#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"

void canvas_plot(struct canvas_type* this, const uint32_t x, const uint32_t y, const uint32_t depth, const struct rgb_type fg_color,
                 const struct rgb_type bg_color, const char ascii_char)

{
    assert(this);
    assert(x < this->width);
    assert(y < this->height);

    const uint32_t index = y * this->width + x;
    const uint32_t prev_depth = this->depth_values[index];

    if (depth > prev_depth) {
        this->ascii_char_values[index] = ascii_char;
        this->fg_color_values[index] = color_encode_rgb(fg_color.r, fg_color.g, fg_color.b);
        this->bg_color_values[index] = color_encode_rgb(bg_color.r, bg_color.g, bg_color.b);
        this->depth_values[index] = depth;
    }
}

void canvas_print_formatted_wo_bg(const struct canvas_type* this, FILE* out)
{
    assert(this);

    for (uint32_t y = 0; y < this->height; y++) {
        for (uint32_t x = 0; x < this->width; x++) {
            const uint32_t index = y * this->width + x;
            const struct rgb_type fg_rgb = color_decode_rgb(this->fg_color_values[index]);
            const char ascii_char = this->ascii_char_values[index];

            fprintf(out,
                    "\033[" // ESC
                    "38;2;" // FG
                    "%03u;%03u;%03u;m"
                    "%c",
                    fg_rgb.r, fg_rgb.g, fg_rgb.b, ascii_char);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\033[0m");
    fflush(out);
}

void canvas_print_formatted(const struct canvas_type* this, FILE* out)
{
    assert(this);

    for (uint32_t y = 0; y < this->height; y++) {
        for (uint32_t x = 0; x < this->width; x++) {
            const uint32_t index = y * this->width + x;
            const struct rgb_type fg_rgb = color_decode_rgb(this->fg_color_values[index]);
            const struct rgb_type bg_rgb = color_decode_rgb(this->bg_color_values[index]);
            const char ascii_char = this->ascii_char_values[index];

            fprintf(out,
                    "\033[" // ESC
                    "38;2;" // FG
                    "%u;%u;%u;"
                    "48;2;" // BG
                    "%u;%u;%u;"
                    "m"
                    "%c",
                    fg_rgb.r, fg_rgb.g, fg_rgb.b, bg_rgb.r, bg_rgb.g, bg_rgb.b, ascii_char);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\033[0m");
    fflush(out);
}

struct canvas_type* canvas_create(const uint32_t width, const uint32_t height, const struct rgb_type default_fg_color,
                                  const struct rgb_type default_bg_color, const char default_ascii_char)

{
    const uint32_t area = width * height;
    assert(width != 0 && area / width == height);

    struct canvas_type* p = calloc(1, sizeof(struct canvas_type));
    if (!p) {
        return NULL;
    }
    p->ascii_char_values = calloc(area, sizeof(char));
    if (!p->ascii_char_values) {
        goto cleanup;
    }
    p->fg_color_values = calloc(area, sizeof(uint32_t));
    if (!p->fg_color_values) {
        goto cleanup;
    }
    p->bg_color_values = calloc(area, sizeof(uint32_t));
    if (!p->bg_color_values) {
        goto cleanup;
    }
    p->depth_values = calloc(area, sizeof(uint32_t));
    if (!p->depth_values) {
        goto cleanup;
    }

    p->width = width;
    p->height = height;
    p->default_fg_color = color_encode_rgb(default_fg_color.r, default_fg_color.g, default_fg_color.b);
    p->default_bg_color = color_encode_rgb(default_bg_color.r, default_bg_color.g, default_bg_color.b);
    p->default_ascii_char = default_ascii_char;

    canvas_clear(p);

    return p;
cleanup:
    if (!p) {
        if (!p->ascii_char_values) {
            free(p->ascii_char_values);
        }
        if (!p->fg_color_values) {
            free(p->fg_color_values);
        }
        if (!p->bg_color_values) {
            free(p->bg_color_values);
        }
        if (!p->depth_values) {
            free(p->depth_values);
        }
        free(p);
    }
    return NULL;
}

void canvas_destroy(struct canvas_type* this)
{
    assert(this);

    free(this->ascii_char_values);
    free(this->fg_color_values);
    free(this->bg_color_values);
    free(this->depth_values);
}

void canvas_clear(struct canvas_type* this)
{
    assert(this);

    for (uint32_t i = 0; i < this->width * this->height; i++) {
        this->ascii_char_values[i] = this->default_ascii_char;
    }
    for (uint32_t i = 0; i < this->width * this->height; i++) {
        this->fg_color_values[i] = this->default_fg_color;
    }
    for (uint32_t i = 0; i < this->width * this->height; i++) {
        this->bg_color_values[i] = this->default_bg_color;
    }
    for (uint32_t i = 0; i < this->width * this->height; i++) {
        this->depth_values[i] = 0;
    }
}

const uint32_t* canvas_get_raw_fg_color_values(const struct canvas_type* this)
{
    assert(this);
    return this->fg_color_values;
}

const uint32_t* canvas_get_raw_bg_color_values(const struct canvas_type* this)
{
    assert(this);
    return this->bg_color_values;
}

const char* canvas_get_raw_ascii_char_values(const struct canvas_type* this)
{
    assert(this);
    return this->ascii_char_values;
}

const uint32_t* canvas_get_raw_depth_values(const struct canvas_type* this)
{
    assert(this);
    return this->depth_values;
}
