
#include "ascii_palettes.h"
#include "draw.h"
#include "draw_vertix_prop.h"
#include "math/vec.h"
#include "screen.h"

// internal plot routines
// ------------------------------------------------------------------------------------------------------------

static inline void internal_plot_point(struct screen_type* screen_p, const vec2_type* v, const vertix_prop_type* prop,
                                       const float* depth) {
    vec2int_type r_v;
    vec2_truncated_to_vec2int(r_v, *v);
    screen_set_pixel_data(screen_p, r_v, (pixel_data_type){.color = prop->color, .depth = *depth, .ascii_char = prop->ascii_char});
}

static inline void internal_plot_line(struct screen_type* screen_p, const ascii_index_conversion_table* conv, const vec2_type pos[2],
                                      const vertix_prop_type prop[2], const float depth[2]) {
    // based on:
    // https://www.redblobgames.com/grids/line-drawing/#more

    vec2_type v[2];
    vec2_copy(v[0], pos[0]); // vec2_copy
    vec2_copy(v[1], pos[1]);
    // vec2_add(v[0], v[0], (vec2_type){0.5f, 0.5f});
    // vec2_add(v[1], v[1], (vec2_type){0.5f, 0.5f});

    const float dx = v[1][0] - v[0][0];
    const float dy = v[1][1] - v[0][1];

    const int abs_dxi = int_abs(float_truncated_to_int(dx));
    const int abs_dyi = int_abs(float_truncated_to_int(dy));
    const int diagonal_dist = abs_dxi > abs_dyi ? abs_dxi : abs_dyi;
    const float diagonal_dist_f = int_to_float(diagonal_dist);

    if (diagonal_dist == 0) {
        return;
    }

    vec2_type v_curr;
    vec2_copy(v_curr, v[0]);
    vec2_type pos_dir = {dx / diagonal_dist_f, dy / diagonal_dist_f};

    for (int step = 0; step <= diagonal_dist; step += 1) {
        const float t = int_to_float(step) / diagonal_dist_f;
        const float d = float_lerped(depth[0], depth[1], t);

        vec2int_type v_curri;
        vec2_truncated_to_vec2int(v_curri, v_curr);

        const vertix_prop_type prop_curr = vertix_prop_lerped(conv, prop[0], prop[1], t);

        screen_set_pixel_data(screen_p, v_curri,
                              (pixel_data_type){.color = prop_curr.color, .depth = d, .ascii_char = prop_curr.ascii_char});

        vec2_add(v_curr, v_curr, pos_dir);
    }
}

static inline void internal_plot_triangle(struct screen_type* screen_p, const ascii_index_conversion_table* conv,
                                          const vec2_type pos[3], const vertix_prop_type prop[3], const float depth[3],
                                          const float bias[3]) {
    // baycentric algorithm:
    // https://www.youtube.com/watch?v=k5wtuKWmV48

    vec2_type v[3];
    vec2_truncated(v[0], pos[0]);
    vec2_truncated(v[1], pos[1]);
    vec2_truncated(v[2], pos[2]);

    // get the bounding box of the triangle
    const int maxX = float_truncated_to_int(float_max(v[0][0], float_max(v[1][0], v[2][0])));
    const int minX = float_truncated_to_int(float_min(v[0][0], float_min(v[1][0], v[2][0])));
    const int maxY = float_truncated_to_int(float_max(v[0][1], float_max(v[1][1], v[2][1])));
    const int minY = float_truncated_to_int(float_min(v[0][1], float_min(v[1][1], v[2][1])));

    const vec2_type p0 = {int_to_float(minX), int_to_float(minY)};

    // relevant vectors:
    vec2_type v1_to_v2, v2_to_v0, v0_to_v1, v0_to_v2, v1_to_p0, v2_to_p0, v0_to_p0;

    vec2_sub(v1_to_v2, v[2], v[1]);
    vec2_sub(v2_to_v0, v[0], v[2]);
    vec2_sub(v0_to_v1, v[1], v[0]);
    vec2_sub(v0_to_v2, v[2], v[0]);

    vec2_sub(v0_to_p0, p0, v[0]);
    vec2_sub(v1_to_p0, p0, v[1]);
    vec2_sub(v2_to_p0, p0, v[2]);

    const float triangle_area_2 = vec2_cross(v0_to_v1, v0_to_v2);

    if (float_is_equal(triangle_area_2, 0.f)) {
        return;
    }

    // for efficient cross product calculation for each point in the bounding box. see video for derivation.
    const float delta_w0_col = v[1][1] - v[2][1];
    const float delta_w0_row = v[2][0] - v[1][0];
    float w0_row = vec2_cross(v1_to_v2, v1_to_p0) + bias[0];

    const float delta_w1_col = v[2][1] - v[0][1];
    const float delta_w1_row = v[0][0] - v[2][0];
    float w1_row = vec2_cross(v2_to_v0, v2_to_p0) + bias[1];

    const float delta_w2_col = v[0][1] - v[1][1];
    const float delta_w2_row = v[1][0] - v[0][0];
    float w2_row = vec2_cross(v0_to_v1, v0_to_p0) + bias[2];

    for (int y = minY; y <= maxY; y++) {
        float w0 = w0_row;
        float w1 = w1_row;
        float w2 = w2_row;

        for (int x = minX; x <= maxX; x++) {
            // non-optimal way to get (w0):
            // vec2_type v1_to_p0;
            // vec2_sub(v1_to_p0, (vec2_type){x,y}, pos[1]);
            // const float w0_at_point = vec2_cross(v1_to_v2, v1_to_p0);

            bool is_inside_triangle = w0 >= 0 && w1 >= 0 && w2 >= 0;
            if (is_inside_triangle) {

                assert(!float_is_equal(triangle_area_2, 0.f));
                const float alpha = w0 / triangle_area_2;
                const float beta = w1 / triangle_area_2;
                const float gamma = w2 / triangle_area_2;

                const float d = alpha * depth[0] + beta * depth[1] + gamma * depth[2];

                color_type color;
                color_type color_comp0, color_comp1, color_comp2;
                vec3_scale(color_comp0.as_vec3, prop[0].color.as_vec3, alpha);
                vec3_scale(color_comp1.as_vec3, prop[1].color.as_vec3, beta);
                vec3_scale(color_comp2.as_vec3, prop[2].color.as_vec3, gamma);
                vec3_add(color.as_vec3, color_comp0.as_vec3, color_comp1.as_vec3);
                vec3_add(color.as_vec3, color.as_vec3, color_comp2.as_vec3);

                const float i0 = int_to_float(conv->ascii_to_index[(int)prop[0].ascii_char]);
                const float i1 = int_to_float(conv->ascii_to_index[(int)prop[1].ascii_char]);
                const float i2 = int_to_float(conv->ascii_to_index[(int)prop[2].ascii_char]);
                const int i = float_rounded_to_int(alpha * i0 + beta * i1 + gamma * i2);

                assert(int_is_inside_range(i, 0, (int)(conv->ascii_palette_size - 1)) && "ascii character is inside table");

                const char c = conv->index_to_ascii[i];

                screen_set_pixel_data(screen_p, (int[2]){x, y}, (pixel_data_type){.color = color, .depth = d, .ascii_char = c});
            }
            w0 += delta_w0_col;
            w1 += delta_w1_col;
            w2 += delta_w2_col;
        }
        w0_row += delta_w0_row;
        w1_row += delta_w1_row;
        w2_row += delta_w2_row;
    }
}
