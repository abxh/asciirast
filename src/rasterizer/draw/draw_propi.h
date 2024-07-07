#pragma once

#include "rasterizer/ascii_table_type.h"
#include "rasterizer/color.h"
#include "rasterizer/draw.h"
#include "math/f32_to_i32.h"

typedef struct {
    color_type color;
    float ascii_char_index_f;
} propi_rep_type; // property intermediate representation

static inline propi_rep_type prop_to_intermidate_rep(const ascii_table_type* table_p, const prop_type prop) {
    return (propi_rep_type){.color = prop.color, .ascii_char_index_f = (float)table_p->ascii_to_index[(int)prop.ascii_char]};
}

static inline char prop_get_ascii_char(const ascii_table_type* table_p, const propi_rep_type prop) {
    return (char)table_p->index_to_ascii[(int)(prop.ascii_char_index_f)];
}

static inline propi_rep_type prop_add(const propi_rep_type p0, const propi_rep_type p1) {
    return (propi_rep_type){.color = {.vec3 = vec3_add(p0.color.vec3, p1.color.vec3)},
                            .ascii_char_index_f = p0.ascii_char_index_f + p1.ascii_char_index_f};
}

static inline propi_rep_type prop_sub(const propi_rep_type p0, const propi_rep_type p1) {
    return (propi_rep_type){.color = {.vec3 = vec3_sub(p0.color.vec3, p1.color.vec3)},
                            .ascii_char_index_f = p0.ascii_char_index_f - p1.ascii_char_index_f};
}

static inline propi_rep_type prop_scale(const propi_rep_type p0, const float t) {
    return (propi_rep_type){.color = {.vec3 = vec3_scale(p0.color.vec3, t)}, .ascii_char_index_f = p0.ascii_char_index_f * t};
}

static inline propi_rep_type prop_lerp(const propi_rep_type p0, const propi_rep_type p1, const float t) {
    return (propi_rep_type){.color = {.vec3 = vec3_lerp(p0.color.vec3, p1.color.vec3, t)},
                            .ascii_char_index_f = f32_lerp(p0.ascii_char_index_f, p1.ascii_char_index_f, t)};
}
