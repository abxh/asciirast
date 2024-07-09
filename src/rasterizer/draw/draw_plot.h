#pragma once

#include "math/int.h"

#include "rasterizer/draw/draw_propi.h"
#include "rasterizer/screen.h"

static inline void plot_point(struct screen_type* screen_p, const ascii_table_type* table_p, const vec2_type v,
                              const propi_rep_type prop, const float depth) {
    const char ascii_char = prop_get_ascii_char(table_p, prop.ascii_char_index_f);

    screen_set_pixel_data(screen_p, vec2_floor(v), (pixel_data_type){.color = prop.color, .depth = depth, .ascii_char = ascii_char});
}

static inline void plot_line(struct screen_type* screen_p, const ascii_table_type* table_p, const vec2_type v[2],
                             const propi_rep_type prop[2], const float depth[2]) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    // calculate the max diagnonal distance of the bounding box:
    const vec2_type v0 = vec2_add(vec2_floor(v[0]), (vec2_type){.x = 0.5f, .y = 0.5f});
    const vec2_type v1 = vec2_add(vec2_floor(v[1]), (vec2_type){.x = 0.5f, .y = 0.5f});

    const vec2_type diagonal_vec = vec2_sub(v1, v0);
    const int max_diagonal_abs_dist = int_max(int_abs((int)(diagonal_vec.x)), int_abs((int)(diagonal_vec.y)));

    if (max_diagonal_abs_dist == 0) {
        return;
    }

    const float t_step = 1.f / (float)max_diagonal_abs_dist;

    const vec2_type v_step = vec2_scale(diagonal_vec, t_step);
    const color_type c_step = color_scale(color_sub(prop[1].color, prop[0].color), t_step);
    const float aci_step = t_step * (prop[1].ascii_char_index_f - prop[0].ascii_char_index_f);
    const float d_step = t_step * (depth[1] - depth[0]);

    vec2_type v_curr = v0;
    color_type c_curr = prop[0].color;
    float aci_curr = prop[0].ascii_char_index_f;
    float d_curr = depth[0];

    // traverse the diagonal line step-by-step and lerp values
    for (int step = 0; step <= max_diagonal_abs_dist; step += 1) {
        const char ac_curr = prop_get_ascii_char(table_p, aci_curr);

        pixel_data_type data = {.color = c_curr, .depth = d_curr, .ascii_char = ac_curr};

        screen_set_pixel_data(screen_p, v_curr, data);

        v_curr = vec2_add(v_curr, v_step);
        c_curr = color_add(c_curr, c_step);
        aci_curr += aci_step;
        d_curr += d_step;
    }
}

// clang-format off
static const char g_edge_map[3][3] = {
    {'\\', '|', '/'},
    {'_', '.', '_'},
    {'/', '|', '\\'}
};
// clang-format on

static inline void plot_edge(struct screen_type* screen_p, const vec2_type v[2], const color_type color0, const float depth[2]) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    // calculate the max diagnonal distance of the bounding box:
    const vec2_type v0 = vec2_add(vec2_floor(v[0]), (vec2_type){.x = 0.5f, .y = 0.5f});
    const vec2_type v1 = vec2_add(vec2_floor(v[1]), (vec2_type){.x = 0.5f, .y = 0.5f});

    const vec2_type diagonal_vec = vec2_sub(v1, v0);
    const int max_diagonal_abs_dist = int_max(int_abs((int)(diagonal_vec.x)), int_abs((int)(diagonal_vec.y)));

    if (max_diagonal_abs_dist == 0) {
        return;
    }

    const float t_step = 1.f / (float)max_diagonal_abs_dist;

    const vec2_type v_step = vec2_scale(diagonal_vec, t_step);
    const float d_step = t_step * (depth[1] - depth[0]);

    vec2_type v_curr_prev = vec2_sub(v0, vec2_scale(v_step, 1.499f));
    vec2_type v_curr = v0;
    float d_curr = depth[0];

    const bool points_up = (diagonal_vec.y > 0);

    // traverse the diagonal line step-by-step and lerp values
    for (int step = 0; step <= max_diagonal_abs_dist; step += 1) {
        const int dx = +((int)v_curr.x - (int)v_curr_prev.x);
        const int dy = -((int)v_curr.y - (int)v_curr_prev.y);
        char ac_curr = g_edge_map[dy + 1][dx + 1];

        pixel_data_type data = {.color = color0, .depth = d_curr, .ascii_char = ac_curr};

        vec2_type v_curr_new = v_curr;
        v_curr_new.y += (float)(points_up && dy == 0);
        v_curr_new.y = float_min(v_curr_new.y, SCREEN_HEIGHT - 1.f);

        // generally try to keep the edges closed:
        const char prev_char_at_pos = screen_p->framebuf[(int)v_curr_new.y][(int)v_curr_new.x];
        if (!(ac_curr == '_' && (prev_char_at_pos == '/' || prev_char_at_pos == '\\'))) {
            screen_set_pixel_data(screen_p, v_curr_new, data);
        }

        v_curr_prev = v_curr;
        v_curr = vec2_add(v_curr, v_step);
        d_curr += d_step;
    }
}
