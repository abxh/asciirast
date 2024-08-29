#include <assert.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"

void canvas_plot(struct canvas_type* this, const uint32_t x, const uint32_t y, const float d, const struct rgb_type fg_color,
                 const struct rgb_type bg_color, const char ascii_char)
{
    assert(this);
    assert(x < this->w);
    assert(y < this->h);
    assert(32 <= ascii_char && ascii_char <= 126);
    assert(0.f - 128 * FLT_EPSILON <= d && d <= 1.0f + 128 * FLT_EPSILON);

    const uint32_t index = y * this->w + x;

    {
        const float prev_depth = this->depth_values[index];
        if (d < prev_depth) {
            return;
        }
    }

    this->ascii_char_values[index] = ascii_char;
    this->fg_color_values[index] = color_encode_rgb(fg_color.r, fg_color.g, fg_color.b);
    this->bg_color_values[index] = color_encode_rgb(bg_color.r, bg_color.g, bg_color.b);
    this->depth_values[index] = d;
}

void canvas_print_formatted_wo_bg(const struct canvas_type* this, FILE* out)
{
    assert(this);

    for (uint32_t y = 0; y < this->h; y++) {
        for (uint32_t x = 0; x < this->w; x++) {
            const uint32_t index = y * this->w + x;
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

    for (uint32_t y = 0; y < this->h; y++) {
        for (uint32_t x = 0; x < this->w; x++) {
            const uint32_t index = y * this->w + x;
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

struct canvas_type* canvas_create(const uint32_t w, const uint32_t h, const uint32_t default_fg_color, const uint32_t default_bg_color,
                                  const char default_ascii_char)

{
    {
        const uint32_t temp_prod = w * h;
        assert(w != 0 && temp_prod / w == h);
    }
    assert(32 <= default_ascii_char && default_ascii_char <= 126);

    assert((default_fg_color & 8) <= 255);
    assert(((default_fg_color >> 8) & 8) <= 255);
    assert(((default_fg_color >> 16) & 8) <= 255);

    assert((default_bg_color & 8) <= 255);
    assert(((default_bg_color >> 8) & 8) <= 255);
    assert(((default_bg_color >> 16) & 8) <= 255);

    struct canvas_type* p = calloc(1, sizeof(struct canvas_type));
    if (!p) {
        return NULL;
    }
    p->ascii_char_values = calloc(w * h, sizeof(char));
    if (!p->ascii_char_values) {
        goto cleanup;
    }
    p->fg_color_values = calloc(w * h, sizeof(uint32_t));
    if (!p->fg_color_values) {
        goto cleanup;
    }
    p->bg_color_values = calloc(w * h, sizeof(uint32_t));
    if (!p->bg_color_values) {
        goto cleanup;
    }
    p->depth_values = calloc(w * h, sizeof(float));
    if (!p->depth_values) {
        goto cleanup;
    }

    p->w = w;
    p->h = h;
    p->default_fg_color = default_fg_color;
    p->default_bg_color = default_bg_color;
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
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->ascii_char_values[i] = this->default_ascii_char;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->fg_color_values[i] = this->default_fg_color;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->bg_color_values[i] = this->default_bg_color;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->depth_values[i] = 0.f;
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

const float* canvas_get_raw_depth_values(const struct canvas_type* this)
{
    assert(this);
    return this->depth_values;
}
