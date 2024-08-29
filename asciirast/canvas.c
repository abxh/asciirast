#include <assert.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>

#include "canvas.h"
#include "color.h"

struct canvas_type {
    char* char_values;
    uint32_t* fg_color_values;
    uint32_t* bg_color_values;
    float* depth_values;

    uint32_t w;
    uint32_t h;
    uint32_t default_fg_color;
    uint32_t default_bg_color;
    float default_depth;
    char default_ascii_char;
};

void canvas_plot(struct canvas_type* this, const uint32_t x, const uint32_t y, const float d, const uint32_t fg_color,
                 const uint32_t bg_color, const char c)
{
    assert(this);
    assert(x < this->w);
    assert(y < this->h);
    assert(32 <= c && c <= 126);
    assert(0.f - 128 * FLT_EPSILON <= d && d <= 1.0f + 128 * FLT_EPSILON);

    const uint32_t index = y * this->w + x;

    {
        const float prev_depth = this->depth_values[index];
        if (d < prev_depth) {
            return;
        }
    }

    this->char_values[index] = c;
    this->fg_color_values[index] = fg_color;
    this->bg_color_values[index] = bg_color;
    this->depth_values[index] = d;
}

void canvas_print_formatted_wo_bg(const struct canvas_type* this, FILE* out)
{
    assert(this);

    for (uint32_t y = 0; y < this->h; y++) {
        for (uint32_t x = 0; x < this->w; x++) {
            const uint32_t index = y * this->w + x;
            const struct rgb_type fg_rgb = color_decode_rgb_unchecked(this->fg_color_values[index]);
            const char c = this->char_values[index];

            fprintf(out,
                    "\033[" // ESC
                    "38;2;" // FG
                    "%03u;%03u;%03u;m"
                    "%c",
                    fg_rgb.r, fg_rgb.g, fg_rgb.b, c);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\033[" // ESC
                 "0m"    // RESET
    );
}

void canvas_print_formatted(const struct canvas_type* this, FILE* out)
{
    assert(this);

    for (uint32_t y = 0; y < this->h; y++) {
        for (uint32_t x = 0; x < this->w; x++) {
            const uint32_t index = y * this->w + x;
            const struct rgb_type fg_rgb = color_decode_rgb_unchecked(this->fg_color_values[index]);
            const struct rgb_type bg_rgb = color_decode_rgb_unchecked(this->bg_color_values[index]);
            const char c = this->char_values[index];

            fprintf(out,
                    "\033[" // ESC
                    "38;2;" // FG
                    "%03u;%03u;%03u;"
                    "48;2;" // BG
                    "%03u;%03u;%03u;"
                    "m"
                    "%c",
                    fg_rgb.r, fg_rgb.g, fg_rgb.b, bg_rgb.r, bg_rgb.g, bg_rgb.b, c);
        }
        fprintf(out, "\n");
    }
    fprintf(out, "\033[" // ESC
                 "0"     // RESET
                 "m");
}

struct canvas_type* canvas_create(const uint32_t w, const uint32_t h, const uint32_t default_fg_color, const uint32_t default_bg_color,
                                  const float default_depth, const char default_ascii_char)

{
    {
        const uint32_t temp_prod = w * h;
        assert(w != 0 && temp_prod / w == h);
    }
    assert(32 <= default_ascii_char && default_ascii_char <= 126);
    assert(0.0f <= default_depth && default_depth <= 1.0f);

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
    p->char_values = calloc(w * h, sizeof(char));
    if (!p->char_values) {
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
    p->default_depth = default_depth;
    p->default_ascii_char = default_ascii_char;

    return p;
cleanup:
    if (!p) {
        if (!p->char_values) {
            free(p->char_values);
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
    free(this->char_values);
    free(this->fg_color_values);
    free(this->bg_color_values);
    free(this->depth_values);
}

void canvas_clear(struct canvas_type* this)
{
    assert(this);
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->char_values[i] = this->default_ascii_char;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->fg_color_values[i] = this->default_fg_color;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->bg_color_values[i] = this->default_bg_color;
    }
    for (uint32_t i = 0; i < this->w * this->h; i++) {
        this->depth_values[i] = this->default_depth;
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

const char* canvas_get_raw_char_values(const struct canvas_type* this)
{
    assert(this);
    return this->char_values;
}

const float* canvas_get_raw_depth_values(const struct canvas_type* this)
{
    assert(this);
    return this->depth_values;
}
