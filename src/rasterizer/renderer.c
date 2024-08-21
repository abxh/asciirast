#include "math/int.h"
#include "math/mat4x4.h"
#include <stdbool.h>

#include "rasterizer/renderer.h"

renderer_type* renderer_create(struct screen_type* screen_context_p, const size_t n, const char ascii_palette[n],
                               const perspective_proj_prop_type prop) {
    renderer_type* this = malloc(sizeof(renderer_type));

    this->screen_p = screen_context_p;

    mat4x4_perspective(this->perspective_mat, prop.fovy_rad, prop.aspect_ratio, prop.z_near, prop.z_far);
    mat4x4_identity(this->model_camera_mat);
    mat4x4_mul(this->mvp, this->model_camera_mat, this->perspective_mat);

    renderer_use_ascii_palette(this, n, ascii_palette);

    return this;
}

void renderer_use_ascii_palette(struct renderer_type* this, const size_t n, const char ascii_palette[n]) {
    for (size_t i = 0; i <= ASCII_MAX_PRINTABLE; i++) {
        this->table.ascii_to_index[i] = -1;
        this->table.index_to_ascii[i] = ' ';
    }
    assert(ascii_palette[n - 1] == '\0');
    this->table.size = n - 1;

    bool space_in_ascii_palette = false;
    for (size_t i = 0; i < n - 1; i++) {
        assert(int_in_range(ascii_palette[i], 32, 126) && "ascii char is not printable");

        const char c = ascii_palette[i];
        this->table.ascii_to_index[(int)c] = (int)i;
        this->table.index_to_ascii[i] = c;

        if (c == ' ') {
            space_in_ascii_palette = true;
        }
    }
    assert(space_in_ascii_palette);
}

void renderer_destroy(renderer_type* this) {
    free(this);
}

void renderer_look_at(struct renderer_type* this, const camera_prop_type prop) {
    mat4x4_look_at(this->model_camera_mat, prop.eye, prop.center, prop.up);
    mat4x4_mul(this->mvp, this->model_camera_mat, this->perspective_mat);
}
