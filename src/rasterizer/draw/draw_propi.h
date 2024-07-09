#pragma once

#include "math/float.h"

#include "rasterizer/ascii_table_type.h"
#include "rasterizer/color.h"
#include "rasterizer/draw.h"

typedef struct {
    color_type color;
    float ascii_char_index_f;
} propi_rep_type; // property intermediate representation

static inline propi_rep_type prop_to_intermidate_rep(const ascii_table_type* table_p, const prop_type prop) {
    return (propi_rep_type){.color = prop.color, .ascii_char_index_f = (float)table_p->ascii_to_index[(int)prop.ascii_char]};
}

static inline char prop_get_ascii_char(const ascii_table_type* table_p, const float ascii_char_index_f) {
    return (char)table_p->index_to_ascii[float_to_int_round(ascii_char_index_f)];
}

static inline propi_rep_type prop_add(const propi_rep_type p0, const propi_rep_type p1) {
    return (propi_rep_type){.color = color_add(p0.color, p1.color),
                            .ascii_char_index_f = p0.ascii_char_index_f + p1.ascii_char_index_f};
}

static inline propi_rep_type prop_sub(const propi_rep_type p0, const propi_rep_type p1) {
    return (propi_rep_type){.color = color_sub(p0.color, p1.color),
                            .ascii_char_index_f = p0.ascii_char_index_f - p1.ascii_char_index_f};
}

static inline propi_rep_type prop_scale(const propi_rep_type p0, const float t) {
    return (propi_rep_type){.color = color_scale(p0.color, t), .ascii_char_index_f = p0.ascii_char_index_f * t};
}

static inline propi_rep_type prop_lerp(const propi_rep_type p0, const propi_rep_type p1, const float t) {
    return (propi_rep_type){.color = color_lerp(p0.color, p1.color, t),
                            .ascii_char_index_f = float_lerp(p0.ascii_char_index_f, p1.ascii_char_index_f, t)};
}
