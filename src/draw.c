#include "ascii_palettes.h"
#include "log.h"
#include "math/mat4x4.h"
#include "math/vec.h"
#include "screen.h"

#include "draw.h"
#include "draw_clip.h"
#include "draw_plot.h"

typedef struct {
    vec4_type left;
    vec4_type right;
    vec4_type bottom;
    vec4_type top;
    vec4_type near;
    vec4_type far;
} frustum_planes_type;

typedef struct renderer_type {
    mat4x4_type perspective_mat;
    mat4x4_type model_camera_mat;
    mat4x4_type mvp;
    frustum_planes_type plane;

    struct screen_type* screen_p;
    lst_vert3_2d vert3_lst;

    ascii_index_conversion_table conv;

    bool initialized;
} renderer_type;

// renderer functions
// ------------------------------------------------------------------------------------------------------------

renderer_type* renderer_create(struct screen_type* screen_context_p, const size_t n, const char ascii_palette[n],
                               const perspective_proj_prop_type prop) {
    renderer_type* this = malloc(sizeof(renderer_type));

    HANDLE_NULL(this, "malloc");

    this->initialized = true;

    this->screen_p = screen_context_p;
    mat4x4_perspective(this->perspective_mat, prop.fovy_rad, prop.aspect_ratio, prop.z_near, prop.z_far);
    mat4x4_identity(this->model_camera_mat);
    mat4x4_copy(this->mvp, this->perspective_mat);
    mat4x4_extract_planes_from_projmat(this->mvp, this->plane.left, this->plane.right, this->plane.bottom, this->plane.top,
                                       this->plane.near, this->plane.far);

    for (size_t i = 0; i < 128; i++) {
        this->conv.ascii_to_index[i] = -1;
        this->conv.index_to_ascii[i] = ' ';
    }

    for (size_t i = 0; i < n; i++) {
        assert(int_is_inside_range(ascii_palette[i], 32, 126) && "ascii char is not printable");

        const char c = ascii_palette[i];
        this->conv.ascii_to_index[(int)c] = (int)i;
        this->conv.index_to_ascii[i] = c;
    }

    this->vert3_lst = lst_vert3_2d_init();

    return this;
}

void renderer_destroy(renderer_type* this) {
    lst_vert3_2d_free(&this->vert3_lst);
    free(this);
}

void renderer_frustum(struct renderer_type* this, const frustum_prop_type prop) {
    mat4x4_frustum(this->perspective_mat, prop.left, prop.right, prop.bottom, prop.top, prop.z_near, prop.z_far);
    mat4x4_mul(this->mvp, this->model_camera_mat, this->perspective_mat);
    mat4x4_extract_planes_from_projmat(this->mvp, this->plane.left, this->plane.right, this->plane.bottom, this->plane.top,
                                       this->plane.near, this->plane.far);
}

void renderer_look_at(struct renderer_type* this, const camera_prop_type prop) {
    mat4x4_look_at(this->model_camera_mat, prop.eye, prop.center, prop.up);
    mat4x4_mul(this->mvp, this->model_camera_mat, this->perspective_mat);
    mat4x4_extract_planes_from_projmat(this->mvp, this->plane.left, this->plane.right, this->plane.bottom, this->plane.top,
                                       this->plane.near, this->plane.far);
}

// world space -> screen space
// ------------------------------------------------------------------------------------------------------------

static inline void vec2_transform_to_screen_space(vec2_type res, const vec2_type pos) {
    vec2_add(res, pos, (vec2_type){1.f, 1.f});
    vec2_scale(res, res, 1.f / 2.f);
    vec2_elementwise_prod(res, res, (vec2_type){(float)SCREEN_WIDTH - 1.f, (float)SCREEN_HEIGHT - 1.f});
}

// 2d
// ------------------------------------------------------------------------------------------------------------

void draw_point_2d(renderer_type* this, const vertix_2d_type v[1], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 1, v));

    if (!vec2_is_inside_range(v[0].pos, (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f})) {
        return;
    }
    vec2_type pos0;
    float depth0;
    vertix_prop_type prop0;

    vec2_transform_to_screen_space(pos0, v[0].pos);
    prop0 = v[0].prop;
    depth0 = (float)z_order / UINT8_MAX;

    internal_plot_point(this->screen_p, &pos0, &prop0, &depth0);
}

void draw_line_2d(renderer_type* this, const vertix_2d_type v[2], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 2, v));

    float t[2];
    if (!internal_clip_line_2d(v[0].pos, v[1].pos, (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f}, &t[0], &t[1])) {
        return;
    }
    float depth0;
    vec2_type pos[2];
    vertix_prop_type prop[2];

    for (size_t i = 0; i < 2; i++) {
        vec2_lerp(pos[i], v[0].pos, v[1].pos, t[i]);
        vec2_transform_to_screen_space(pos[i], pos[i]);
    }

    for (size_t i = 0; i < 2; i++) {
        prop[i] = vertix_prop_lerped(&this->conv, v[0].prop, v[1].prop, t[i]);
    }

    depth0 = (float)z_order / UINT8_MAX;

    internal_plot_line(this->screen_p, &this->conv, pos, prop, (float[2]){depth0, depth0});
}

void draw_filled_triangle_2d(renderer_type* this, const vertix_2d_type v[3], const uint8_t z_order) {
    assert(valid_vertix_2d_check(&this->conv, 3, v));

    const vert3_2d vert3 = {.value = {v[0], v[1], v[2]}};

    if (internal_vert3_2d_back_face_cull(vert3)) {
        return;
    }

    internal_clip_triangle_2d(&this->conv, &this->vert3_lst, vert3, (vec2_type){-1.f, -1.f}, (vec2_type){1.f, 1.f});

    foreach (lst_vert3_2d, &this->vert3_lst, it) {
        float depth0;
        vec2_type pos[3];
        vertix_prop_type prop[3];

        for (size_t i = 0; i < 3; i++) {
            vec2_transform_to_screen_space(pos[i], it.ref->value[i].pos);
        }

        for (size_t i = 0; i < 3; i++) {
            prop[i] = it.ref->value[i].prop;
        }

        depth0 = (float)z_order / UINT8_MAX;

        internal_plot_triangle(this->screen_p, &this->conv, pos, prop, (float[3]){depth0, depth0, depth0}, (float[3]){0.f, 0.f, 0.f});
    }

    lst_vert3_2d_clear(&this->vert3_lst);
}

// 3d
// ------------------------------------------------------------------------------------------------------------

void draw_point_3d(renderer_type* this, const vertix_3d_type v[1]) {
    assert(valid_vertix_3d_check(&this->conv, 1, v));

    vec4_type pos0;
    vertix_prop_type prop0;
    vec2_type pos2d0;

    mat4x4_mul_vec4(pos0, this->mvp, v[0].pos);

    // -w0 <= x0,y0,z0 <= w0
    const float w0 = pos0[3];
    if (!vec3_is_inside_range(pos0, (vec3_type){-w0, -w0, -w0}, (vec3_type){w0, w0, w0})) {
        return;
    }

    vec4_scale(pos0, pos0, 1.f / w0); // z-divide

    prop0 = v[0].prop;

    vec2_transform_to_screen_space(pos0, pos0);

    pos2d0[0] = pos0[0];
    pos2d0[1] = pos0[1];

    internal_plot_point(this->screen_p, &pos2d0, &prop0, &pos0[2]);
}

void draw_line_3d(struct renderer_type* this, const vertix_3d_type v[2]) {
    assert(valid_vertix_3d_check(&this->conv, 2, v));

    vec4_type pos[2];
    vec2_type pos2d[2];
    float t[2];
    vertix_prop_type prop[2];
    float depth[2];

    for (size_t i = 0; i < 2; i++) {
        mat4x4_mul_vec4(pos[i], this->mvp, v[i].pos);
    }

    // TODO: culling / clipping

    for (size_t i = 0; i < 2; i++) {
        vec4_lerp(pos[i], pos[0], pos[1], t[i]);
        vec4_scale(pos[i], pos[i], 1.f / pos[i][3]); // z-divide
        vec2_transform_to_screen_space(pos[i], pos[i]);
        pos2d[i][0] = pos[i][0];
        pos2d[i][1] = pos[i][1];
    }

    for (size_t i = 0; i < 2; i++) {
        prop[i] = vertix_prop_lerped(&this->conv, v[0].prop, v[1].prop, t[i]);
    }

    for (size_t i = 0; i < 2; i++) {
        depth[i] = pos[i][2];
    }

    internal_plot_line(this->screen_p, &this->conv, pos2d, prop, depth);
}
