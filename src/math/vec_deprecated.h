#pragma once

#include "vec.h"

static inline void project_from_world_space_to_screen_space(vec2_type* res, const vec3_type v0, const float fov_angle_rad,
                                                            const float aspect_ratio) {
    assert(!float_is_equal(0.f, v0[2]) && "Cannot divide by 0");
    assert(!float_is_equal(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");

    const float c = v0[2] * tanf(fov_angle_rad / 2.f);

    *res[0] = 1.f / c * aspect_ratio * v0[0];
    *res[1] = 1.f / c * v0[1];
}

static inline void rotate_vec3_around_x_axis(vec3_type* res, const vec3_type v0, const float angle_rad) {
    *res[0] = v0[0];
    *res[1] = v0[1] * cosf(angle_rad) - v0[2] * sinf(angle_rad);
    *res[2] = v0[1] * sinf(angle_rad) + v0[2] * cosf(angle_rad);
}

static inline void rotate_vec3_around_y_axis(vec3_type* res, const vec3_type v0, const float angle_rad) {
    *res[0] = v0[0] * cosf(angle_rad) + v0[2] * sinf(angle_rad);
    *res[1] = v0[1];
    *res[2] = v0[2] * cosf(angle_rad) - v0[0] * sinf(angle_rad);
}

static inline void rotate_vec3_around_z_axis(vec3_type* res, const vec3_type v0, const float angle_rad) {
    *res[0] = v0[0] * cosf(angle_rad) - v0[1] * sinf(angle_rad);
    *res[1] = v0[0] * sinf(angle_rad) + v0[1] * cosf(angle_rad);
    *res[2] = v0[2];
}

static inline void project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(vec4_type* res, const vec3_type v0,
                                                                                             const float fov_angle_rad,
                                                                                             const float aspect_ratio,
                                                                                             const float z_near, const float z_far) {

    assert(!float_is_equal(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");

    const float fov_inv_scalar = tanf(fov_angle_rad / 2.f);

    const float depth_scalar = z_far / (z_far - z_near);

    *res[0] = 1 / fov_inv_scalar * aspect_ratio * v0[0];
    *res[1] = 1 / fov_inv_scalar * v0[1];
    *res[2] = depth_scalar * v0[2] - depth_scalar * z_near;
    *res[3] = v0[2];
}

static inline void move_against_camera(vec3_type* res_final, const vec3_type vec, const vec3_type camera_pos,
                                            const float camera_angle_rad[3]) {
    vec3_type res0, res1, res2, res3;

    vec3_scale(res0, camera_pos, -1);
    vec3_add(res1, res0, vec);
    rotate_vec3_around_x_axis(&res2, res1, -camera_angle_rad[0]);
    rotate_vec3_around_y_axis(&res3, res2, -camera_angle_rad[1]);
    rotate_vec3_around_z_axis(res_final, res3, -camera_angle_rad[2]);
}
