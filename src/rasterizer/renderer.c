#include "rasterizer/renderer.h"
#include "math/i32.h"
#include "math/mat4x4.h"
#include "rasterizer/renderer_type.h"

renderer_type* renderer_create(struct screen_type* screen_context_p, const size_t n, const char ascii_palette[n],
                               const perspective_proj_prop_type prop) {
    renderer_type* this = malloc(sizeof(renderer_type));

    this->screen_p = screen_context_p;
    mat4x4_perspective(this->perspective_mat, prop.fovy_rad, prop.aspect_ratio, prop.z_near, prop.z_far);
    mat4x4_identity(this->model_camera_mat);
    mat4x4_copy(this->mvp, this->perspective_mat);
    mat4x4_extract_planes_from_projmat(this->mvp, this->plane.left, this->plane.right, this->plane.bottom, this->plane.top,
                                       this->plane.near, this->plane.far);
    renderer_use_ascii_palette(this, n, ascii_palette);

    sc_list_init(&this->lst);

    return this;
}

void renderer_use_ascii_palette(struct renderer_type* this, const size_t n, const char ascii_palette[n]) {
    for (size_t i = 0; i <= ASCII_MAX_PRINTABLE; i++) {
        this->table.ascii_to_index[i] = -1;
        this->table.index_to_ascii[i] = ' ';
    }
    assert(ascii_palette[n - 1] == '\0');
    this->table.size = n - 1;

    for (size_t i = 0; i < n - 1; i++) {
        assert(i32_in_range(ascii_palette[i], 32, 126) && "ascii char is not printable");

        const char c = ascii_palette[i];
        this->table.ascii_to_index[(int)c] = (int)i;
        this->table.index_to_ascii[i] = c;
    }
}

void renderer_destroy(renderer_type* this) {
    free(this);
}

void renderer_look_at(struct renderer_type* this, const camera_prop_type prop) {
    mat4x4_look_at(this->model_camera_mat, prop.eye, prop.center, prop.up);
    mat4x4_mul(this->mvp, this->model_camera_mat, this->perspective_mat);
    mat4x4_extract_planes_from_projmat(this->mvp, this->plane.left, this->plane.right, this->plane.bottom, this->plane.top,
                                       this->plane.near, this->plane.far);
}
