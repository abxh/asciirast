#include "rasterizer/draw.h"
#include "math/vec.h"
#include "rasterizer/draw/draw_plot.h"
#include "rasterizer/draw/draw_propi.h"
#include "rasterizer/draw/draw_valid_prop.h"
#include "rasterizer/renderer_type.h"
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
    assert(valid_prop_2d_check(&this->table, 1, prop));

    bool outside_screen = !vec2_in_range(v[0], (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f});
    if (outside_screen) {
        return;
    }

    vec2_type pos0;
    float depth0;
    propi_rep_type prop0;

    float2_transform_to_screen_space(pos0.array, v[0].array);

    prop0 = prop_to_intermidate_rep(&this->table, prop[0]);

    depth0 = (float)z_order / UINT8_MAX;

    plot_point(this->screen_p, &this->table, pos0, prop0, depth0);
}
