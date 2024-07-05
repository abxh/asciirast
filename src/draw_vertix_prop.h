
#pragma once

#include "ascii_palettes.h"
#include "draw.h"

// vertix prop operations
// ------------------------------------------------------------------------------------------------------------

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-function"

static inline vertix_prop_type vertix_prop_sum(const ascii_index_conversion_table* conv, const vertix_prop_type v0,
                                               const vertix_prop_type v1) {
    color_type color;
    vec3_add(color.as_vec3, v0.color.as_vec3, v1.color.as_vec3);
    const int i = conv->ascii_to_index[(int)v0.ascii_char] + conv->ascii_to_index[(int)v1.ascii_char];
    const char c = conv->index_to_ascii[i];

    return (vertix_prop_type){.color = color, .ascii_char = c};
}

static inline vertix_prop_type vertix_prop_scaled(const ascii_index_conversion_table* conv, const vertix_prop_type v0, const float t) {
    color_type color;
    vec3_scale(color.as_vec3, v0.color.as_vec3, t);
    const int i = float_rounded_to_int(t * int_to_float(conv->ascii_to_index[(int)v0.ascii_char]));
    const char c = conv->index_to_ascii[i];

    return (vertix_prop_type){.color = color, .ascii_char = c};
}

#pragma clang diagnostic pop

static inline vertix_prop_type vertix_prop_lerped(const ascii_index_conversion_table* conv, const vertix_prop_type v0,
                                                  vertix_prop_type v1, const float t) {
    color_type color;
    vec3_lerp(color.as_vec3, v0.color.as_vec3, v1.color.as_vec3, t);
    const int i0 = conv->ascii_to_index[(int)v0.ascii_char];
    const int i1 = conv->ascii_to_index[(int)v1.ascii_char];
    const int i = int_lerped_rounded(i0, i1, t);
    const char c = conv->index_to_ascii[i];

    return (vertix_prop_type){.color = color, .ascii_char = c};
}

// valid vertix check
// ------------------------------------------------------------------------------------------------------------

static inline bool valid_vertix_2d_check(const ascii_index_conversion_table* conv, const size_t n, const vertix_2d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        const char c = v[i].prop.ascii_char;
        is_valid &= conv->ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].prop.color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}

static inline bool valid_vertix_3d_check(const ascii_index_conversion_table* conv, const size_t n, const vertix_3d_type v[n]) {
    bool is_valid = true;
    for (size_t i = 0; i < n; i++) {
        is_valid &= float_is_equal(v[i].pos[3], 1.f);
        const char c = v[i].prop.ascii_char;
        is_valid &= conv->ascii_to_index[(int)c] != -1;
        is_valid &= vec3_is_inside_range(v[i].prop.color.as_vec3, (vec3_type){0.f, 0.f, 0.f}, (vec3_type){1.f, 1.f, 1.f});
    }
    return is_valid;
}
