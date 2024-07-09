#pragma once

#include "math/vec.h"

static inline vec2_type project_from_world_space_to_screen_space(const vec3_type v0, const float fov_angle_rad,
                                                                 const float aspect_ratio) {
    // this gets rid of useful info like z value that can be used for clipping/culling
    // as z approaches 0 from the positive side

    assert(!float_is_equal(0.f, v0.z) && "Cannot divide by 0");
    assert(!float_is_equal(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");

    const float c = v0.z * tanf(fov_angle_rad / 2.f);

    const float res_x = 1.f / c * aspect_ratio * v0.x;
    const float res_y = 1.f / c * v0.y;

    return (vec2_type){.x = res_x, .y = res_y};
}

static inline vec3_type rotate_vec3_around_x_axis(const vec3_type v0, const float angle_rad) {
    const float res_x = v0.x;
    const float res_y = v0.y * cosf(angle_rad) - v0.z * sinf(angle_rad);
    const float res_z = v0.y * sinf(angle_rad) + v0.z * cosf(angle_rad);

    return (vec3_type){.x = res_x, .y = res_y, .z = res_z};
}

static inline vec3_type rotate_vec3_around_y_axis(const vec3_type v0, const float angle_rad) {
    const float res_x = v0.x * cosf(angle_rad) + v0.z * sinf(angle_rad);
    const float res_y = v0.y;
    const float res_z = v0.z * cosf(angle_rad) - v0.x * sinf(angle_rad);

    return (vec3_type){.x = res_x, .y = res_y, .z = res_z};
}

static inline vec3_type rotate_vec3_around_z_axis(const vec3_type v0, const float angle_rad) {
    const float res_x = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad);
    const float res_y = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad);
    const float res_z = v0.z;

    return (vec3_type){.x = res_x, .y = res_y, .z = res_z};
}

static inline vec4_type project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(
    const vec3_type v0, const float fov_angle_rad, const float aspect_ratio, const float z_near, const float z_far) {

    assert(!float_is_equal(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");
    assert(0 < z_near && z_near < z_far);

    const float fov_inv_scalar = tanf(fov_angle_rad / 2.f);

    const float res_x = 1 / fov_inv_scalar * aspect_ratio * v0.x;
    const float res_y = 1 / fov_inv_scalar * v0.y;
    const float res_z = z_far * (v0.z - z_near) / (z_far - z_near);
    const float res_w = v0.z;

    return (vec4_type){.x = res_x, .y = res_y, .z = res_z, .w = res_w};
}

static inline vec3_type move_against_camera(const vec3_type vec, const vec3_type camera_pos, const float camera_angle_rad[3]) {
    // this cannot be expressed with a 3x3 matrix.

    const vec3_type res0 = vec3_sub(vec, camera_pos);
    const vec3_type res1 = rotate_vec3_around_x_axis(res0, -camera_angle_rad[0]);
    const vec3_type res2 = rotate_vec3_around_y_axis(res1, -camera_angle_rad[1]);
    const vec3_type res3 = rotate_vec3_around_z_axis(res2, -camera_angle_rad[2]);

    return res3;
}
