#include "math/vec.h"

#include "rasterizer/draw.h"
#include "rasterizer/draw/draw_clip.h"
#include "rasterizer/draw/draw_plot.h"
#include "rasterizer/draw/draw_propi.h"
#include "rasterizer/draw/draw_valid_prop.h"
#include "rasterizer/renderer.h"
#include "rasterizer/screen.h"

// world space -> screen space
// ------------------------------------------------------------------------------------------------------------

static inline void float2_transform_to_screen_space(float res[2], const float pos[2]) {
    const vec2_type shifted_pos = vec2_scale(vec2_add(vec2_from_array(pos), (vec2_type){.array = {1.f, 1.f}}), 0.5f);
    const vec2_type screen_rect_vec = (vec2_type){.x = SCREEN_WIDTH - 1.f, .y = SCREEN_HEIGHT - 1.f};

    vec2_to_array(res, vec2_elementwise_prod(shifted_pos, screen_rect_vec));
}

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(renderer_type* this, const vec2_type v[1], const prop_type prop[1], const uint8_t z_order) {
    assert(valid_prop_check(&this->table, 1, prop));

    bool outside_screen = !vec2_in_range(v[0], (vec2_type){.x = -1.f, .y = -1.f}, (vec2_type){.x = 1.f, .y = 1.f});
    if (outside_screen) {
        return;
    }
    const float depth0 = (float)z_order / UINT8_MAX;

    const propi_rep_type propi0 = prop_to_intermidate_rep(&this->table, prop[0]);

    vec2_type pos0;
    float2_transform_to_screen_space(pos0.array, v[0].array);

    plot_point(this->screen_p, &this->table, pos0, propi0, depth0);
}

void draw_line_2d(renderer_type* this, const vec2_type v[2], const prop_type prop[2], const uint8_t z_order) {
    assert(valid_prop_check(&this->table, 2, prop));

    float t[2];
    bool outside_screen = !clip_line_2d(v[0], v[1], &t[0], &t[1]);
    if (outside_screen) {
        return;
    }
    const float depth0 = (float)z_order / UINT8_MAX;

    propi_rep_type propi[2];
    for (size_t i = 0; i < 2; i++) {
        propi[i] = prop_to_intermidate_rep(&this->table, prop[i]);
    }
    for (size_t i = 0; i < 2; i++) {
        propi[i] = prop_lerp(propi[0], propi[1], t[i]);
    }

    vec2_type pos[2];
    for (size_t i = 0; i < 2; i++) {
        pos[i] = vec2_lerp(v[0], v[1], t[i]);
        float2_transform_to_screen_space(pos[i].array, pos[i].array);
    }

    plot_line(this->screen_p, &this->table, pos, propi, (float[2]){depth0, depth0});
}

void draw_edge_2d(renderer_type* this, const vec2_type v[2], const color_type color0, const uint8_t z_order) {
    assert(valid_color_check(1, &color0));

    float t[2];
    bool outside_screen = !clip_line_2d(v[0], v[1], &t[0], &t[1]);
    if (outside_screen) {
        return;
    }
    const float depth0 = (float)z_order / UINT8_MAX;

    vec2_type pos[2];
    for (size_t i = 0; i < 2; i++) {
        pos[i] = vec2_lerp(v[0], v[1], t[i]);
        float2_transform_to_screen_space(pos[i].array, pos[i].array);
    }

    plot_edge(this->screen_p, pos, color0, (float[2]){depth0, depth0});
}

// void draw_filled_triangle_2d(struct renderer_type* this, const vec2_type v[3], const prop_type prop[3], const uint8_t z_order) {
//     assert(valid_prop_2d_check(&this->table, 3, prop));
// }

// void draw_point_3d(struct renderer_type* this, const vec4_type v[1], const prop_type prop[1]) {
//     assert(valid_prop_check(&this->table, 1, prop));
//
//     vec4_type v0_new;
//     mat4x4_mul_vec4(v0_new, this->mvp, v[0]);
//
//     // -w0 <= x0,y0,z0 <= w0
//     const vec3_type min = (vec3_type){.array = {-v0_new.w, -v0_new.w, -v0_new.w}};
//     const vec3_type max = (vec3_type){.array = {+v0_new.w, +v0_new.w, +v0_new.w}};
//
//     bool outside_screen = !vec3_in_range(vec3_from_array(v[0].array), min, max);
//     if (outside_screen) {
//         return;
//     }
//     v0_new = vec4_scale(v0_new, 1.f / v0_new.w); // z-divide
//     float2_transform_to_screen_space(v0_new.array, v0_new.array);
//
//     const propi_rep_type propi0 = prop_to_intermidate_rep(&this->table, prop[0]);
//
//     plot_point(this->screen_p, &this->table, vec2_from_array(v0_new.array), propi0, v0_new.z);
// }

// void draw_line_3d(struct renderer_type* this, const vec4_type v[2], const prop_type prop[2]) {
//     assert(valid_prop_check(&this->table, 2, prop));
// }
