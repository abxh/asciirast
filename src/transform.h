#pragma once

#include <math.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
    float x;
    float y;
} vec2_type;

typedef struct {
    int x;
    int y;
} vec2int_type;

typedef struct {
    float x;
    float y;
    float z;
} vec3_type;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4_type;

#define FLOAT_TOLERANCE 0.0000001f
#define PI 3.1415927f

// type change functions:
#define FROM_ANGLE_RAD_TO_ANGLE_DEG(expr) ((expr) * 180.f / PI)
#define FROM_ANGLE_DEG_TO_ANGLE_RAD(expr) ((expr) * PI / 180.f)

static inline float from_int_to_float(const int v0);
static inline int from_float_to_int_truncated(const float v0);
static inline int from_float_to_int_rounded(const float v0);
static inline vec2_type from_vec2int_to_vec2(const vec2int_type v0);
static inline vec2int_type from_vec2_to_vec2int_truncated(const vec2_type v0);
static inline vec2int_type from_vec2_to_vec2int_rounded(const vec2_type v0);

// absolute value functions
static inline int abs_int(const int v0);
static inline float abs_float(const float v0);

// is_equal functions:
static inline bool is_equal_float(const float v0, const float v1);
static inline bool is_equal_int(const int v0, const int v1);
static inline bool is_equal_vec2(const vec2_type v0, const vec2_type v1);
static inline bool is_equal_vec2int(const vec2int_type v0, const vec2int_type v1);
static inline bool is_equal_vec3(const vec3_type v0, const vec3_type v1);
static inline bool is_equal_vec4(const vec4_type v0, const vec4_type v1);

// inside_range functions:
static inline bool inside_range_int(const int v0, const int min, const int max);
static inline bool inside_range_vec2int(const vec2int_type v0, const vec2int_type min, const vec2int_type max);
static inline bool inside_range_float(const float v0, const float min, const float max);
static inline bool inside_range_vec2(const vec2_type v0, const vec2_type min, const vec2_type max);
static inline bool inside_range_vec3(const vec3_type v0, const vec3_type min, const vec3_type max);
static inline bool inside_range_vec4(const vec4_type v0, const vec4_type min, const vec4_type max);

// dot products:
static inline int dot_int(const int v0, const int v1);
static inline int dot_vec2int(const vec2int_type v0, const vec2int_type v1);
static inline float dot_float(const float v0, const float v1);
static inline float dot_vec2(const vec2_type v0, const vec2_type v1);
static inline float dot_vec3(const vec3_type v0, const vec3_type v1);
static inline float dot_vec4(const vec4_type v0, const vec4_type v1);

// cross products:
static inline int cross_vec2int(const vec2int_type v0, const vec2int_type v1);
static inline float cross_vec2(const vec2_type v0, const vec2_type v1);
static inline vec3_type cross_vec3(const vec3_type v0, const vec3_type v1);

// length functions:
static inline float length_vec2(const vec2_type v0);
static inline float length_vec3(const vec3_type v0);
static inline float length_vec4(const vec4_type v0);

// int functions:
static inline int max_int(const int v0, const int v1);
static inline int min_int(const int v0, const int v1);
static inline int clamp_int(const int v0, const int min, const int max);

// vec2int functions:
static inline vec2int_type sum_vec2int(const vec2int_type v0, const vec2int_type v1);
static inline vec2int_type scaled_vec2int(const vec2int_type v0, const int n);
static inline vec2int_type src_to_dest_vec2int(const vec2int_type src, const vec2int_type dest);

// float functions:
static inline float max_float(const float v0, const float v1);
static inline float min_float(const float v0, const float v1);
static inline float clamp_float(const float v0, const float min, const float max);
static inline float lerp_float(const float v0, const float v1, const float t);

// vec2 functions:
static inline vec2_type sum_vec2(const vec2_type v0, const vec2_type v1);
static inline vec2_type scaled_vec2(const vec2_type v0, const float t);
static inline vec2_type src_to_dest_vec2(const vec2_type src, const vec2_type dest);
static inline vec2_type lerp_vec2(const vec2_type v0, const vec2_type v1, const float t);
static inline vec2_type normalize_vec2(const vec2_type v0);

// vec3 functions:
static inline vec3_type sum_vec3(const vec3_type v0, const vec3_type v1);
static inline vec3_type scaled_vec3(const vec3_type v0, const float t);
static inline vec3_type src_to_dest_vec3(const vec3_type src, const vec3_type dest);
static inline vec3_type lerp_vec3(const vec3_type v0, const vec3_type v1, const float t);
static inline vec3_type normalize_vec3(const vec3_type v0);

// vec4 functions:
static inline vec4_type sum_vec4(const vec4_type v0, const vec4_type v1);
static inline vec4_type scaled_vec4(const vec4_type v0, const float t);
static inline vec4_type src_to_dest_vec4(const vec4_type src, const vec4_type dest);
static inline vec4_type lerp_vec4(const vec4_type v0, const vec4_type v1, const float t);
static inline vec4_type normalize_vec4(const vec4_type v0);

// transformation functions:
static inline vec2_type rotate_vec2_around_origo(const vec2_type v0, const float angle_rad);
static inline vec2_type project_from_world_space_to_screen_space(const vec3_type v0, const float fov_angle_rad,
                                                                 const float aspect_ratio);
static inline vec3_type rotate_vec3_around_x_axis(const vec3_type v0, const float angle_rad);
static inline vec3_type rotate_vec3_around_y_axis(const vec3_type v0, const float angle_rad);
static inline vec3_type rotate_vec3_around_z_axis(const vec3_type v0, const float angle_rad);
static inline vec4_type project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(
    const vec3_type vec, const float fov_angle_rad, const float aspect_ratio, const float z_near, const float z_far);

// Implementation (absolute value functions):
// ----------------------------------------------------------------------------------------------------------

static inline int abs_int(const int v0) {
    return abs(v0);
}

static inline float abs_float(const float v0) {
    return fabsf(v0);
}

// Implementation (type change functions):
// ----------------------------------------------------------------------------------------------------------

static inline float from_int_to_float(const int v0) {
    return (float)v0;
}

static inline int from_float_to_int_truncated(const float v0) {
    return (int)v0;
}

static inline int from_float_to_int_rounded(const float v0) {
    const float v1 = roundf(v0);
    return (int)v1;
}

static inline vec2_type from_vec2int_to_vec2(const vec2int_type v0) {
    return (vec2_type){.x = from_int_to_float(v0.x), .y = from_int_to_float(v0.y)};
}

static inline vec2int_type from_vec2_to_vec2int_truncated(const vec2_type v0) {
    return (vec2int_type){.x = from_float_to_int_truncated(v0.x), .y = from_float_to_int_truncated(v0.y)};
}

static inline vec2int_type from_vec2_to_vec2int_rounded(const vec2_type v0) {
    return (vec2int_type){.x = from_float_to_int_rounded(v0.x), .y = from_float_to_int_rounded(v0.y)};
}

// Implementation (is_equal):
// ----------------------------------------------------------------------------------------------------------

static inline bool is_equal_int(const int v0, const int v1) {
    return v0 == v1;
}

static inline bool is_equal_vec2int(const vec2int_type v0, const vec2int_type v1) {
    return is_equal_int(v0.x, v1.x) && is_equal_int(v0.y, v1.y);
}

static inline bool is_equal_float(const float v0, const float v1) {
    return abs_float(v0 - v1) <= FLOAT_TOLERANCE;
}

static inline bool is_equal_vec2(const vec2_type v0, const vec2_type v1) {
    return is_equal_float(v0.x, v1.x) && is_equal_float(v0.y, v1.y);
}

static inline bool is_equal_vec3(const vec3_type v0, const vec3_type v1) {
    return is_equal_float(v0.x, v1.x) && is_equal_float(v0.y, v1.y) && is_equal_float(v0.z, v1.z);
}

static inline bool is_equal_vec4(const vec4_type v0, const vec4_type v1) {
    return is_equal_float(v0.x, v1.x) && is_equal_float(v0.y, v1.y) && is_equal_float(v0.z, v1.z) && is_equal_float(v0.w, v1.w);
}

// Implementation (inside_range functions):
// ----------------------------------------------------------------------------------------------------------

static inline bool inside_range_int(const int v0, const int min, const int max) {
    return min <= v0 && v0 <= max;
}

static inline bool inside_range_vec2int(const vec2int_type v0, const vec2int_type min, const vec2int_type max) {
    return inside_range_int(v0.x, min.x, max.x) && inside_range_int(v0.y, min.y, max.y);
}

static inline bool inside_range_float(const float v0, const float min, const float max) {
    return min <= v0 && v0 <= max;
}

static inline bool inside_range_vec2(const vec2_type v0, const vec2_type min, const vec2_type max) {
    return inside_range_float(v0.x, min.x, max.x) && inside_range_float(v0.y, min.y, max.y);
}

static inline bool inside_range_vec3(const vec3_type v0, const vec3_type min, const vec3_type max) {
    return inside_range_float(v0.x, min.x, max.x) && inside_range_float(v0.y, min.y, max.y) && inside_range_float(v0.z, min.z, max.z);
}

static inline bool inside_range_vec4(const vec4_type v0, const vec4_type min, const vec4_type max) {
    return inside_range_float(v0.x, min.x, max.x) && inside_range_float(v0.y, min.y, max.y) &&
           inside_range_float(v0.z, min.z, max.z) && inside_range_float(v0.w, min.w, max.w);
}

// Implementation (dot products):
// ----------------------------------------------------------------------------------------------------------

static inline int dot_int(const int v0, const int v1) {
    return v0 * v1;
}

static inline int dot_vec2int(const vec2int_type v0, const vec2int_type v1) {
    return dot_int(v0.x, v1.x) + dot_int(v0.y, v1.y);
}

static inline float dot_float(const float v0, const float v1) {
    return v0 * v1;
}

static inline float dot_vec2(const vec2_type v0, const vec2_type v1) {
    return dot_float(v0.x, v1.x) + dot_float(v0.y, v1.y);
}

static inline float dot_vec3(const vec3_type v0, const vec3_type v1) {
    return dot_float(v0.x, v1.x) + dot_float(v0.y, v1.y) + dot_float(v0.z, v1.z);
}

static inline float dot_vec4(const vec4_type v0, const vec4_type v1) {
    return dot_float(v0.x, v1.x) + dot_float(v0.y, v1.y) + dot_float(v0.z, v1.z) + dot_float(v0.w, v1.w);
}

// Implementation (cross product):
// ----------------------------------------------------------------------------------------------------------

static inline vec3_type cross_vec3(const vec3_type v0, const vec3_type v1) {
    //     | x v0.x v1.x |            | v0.y v1.y |            | v0.x v1.x |            | v0.x v1.x |
    // det(| y v0.y v1.y |) = x * det(| v0.z v1.z |) - y * det(| v0.z v1.z |) + z * det(| v0.y v1.y |)
    //     | z v0.z v1.z |
    //
    //   = x * (v0.y * v1.z - v0.z * v1.y) + y * (-v0.x * v1.z + v0.z * v1.x) + z * (v0.x * v1.y - v0.y * v1.x)

    const float x_comp = v0.y * v1.z - v0.z * v1.y;
    const float y_comp = v0.z * v1.x - v0.x * v1.z;
    const float z_comp = v0.x * v1.y - v0.y * v1.x;

    return (vec3_type){.x = x_comp, .y = y_comp, .z = z_comp};
}

static inline int cross_vec2int(const vec2int_type v0, const vec2int_type v1) {
    const int z_comp = v0.x * v1.y - v0.y * v1.x;

    return z_comp;
}

static inline float cross_vec2(const vec2_type v0, const vec2_type v1) {
    const float z_comp = v0.x * v1.y - v0.y * v1.x;

    return z_comp;
}

// Implementation (length):
// ----------------------------------------------------------------------------------------------------------

static inline float length_vec2(const vec2_type v0) {
    return sqrtf(dot_vec2(v0, v0));
}

static inline float length_vec3(const vec3_type v0) {
    return sqrtf(dot_vec3(v0, v0));
}

static inline float length_vec4(const vec4_type v0) {
    return sqrtf(dot_vec4(v0, v0));
}

// Implementation (int):
// ----------------------------------------------------------------------------------------------------------

static inline int max_int(const int v0, const int v1) {
    return (v0 > v1) ? v0 : v1;
}

static inline int min_int(const int v0, const int v1) {
    return (v0 < v1) ? v0 : v1;
}

static inline int clamp_int(const int v0, const int min, const int max) {
    return min_int(max_int(v0, min), max);
}

// Implementation (vec2int):
// ----------------------------------------------------------------------------------------------------------

static inline vec2int_type sum_vec2int(const vec2int_type v0, const vec2int_type v1) {
    return (vec2int_type){.x = v0.x + v1.x, .y = v0.y + v1.y};
}

static inline vec2int_type scaled_vec2int(const vec2int_type v0, const int n) {
    return (vec2int_type){.x = n * v0.x, .y = n * v0.y};
}

static inline vec2int_type src_to_dest_vec2int(const vec2int_type src, const vec2int_type dest) {
    return sum_vec2int(dest, scaled_vec2int(src, -1));
}

// Implementation (float):
// ----------------------------------------------------------------------------------------------------------

static inline float max_float(const float v0, const float v1) {
    return (v0 > v1) ? v0 : v1;
}

static inline float min_float(const float v0, const float v1) {
    return (v0 < v1) ? v0 : v1;
}

static inline float clamp_float(const float v0, const float min, const float max) {
    return min_float(max_float(v0, min), max);
}

static inline float lerp_float(const float v0, const float v1, const float t) {
    if (is_equal_float(v0, v1)) {
        return v0;
    }
    return (1 - t) * v0 + t * v1;
}

// Implementation (vec2):
// ----------------------------------------------------------------------------------------------------------

static inline vec2_type sum_vec2(const vec2_type v0, const vec2_type v1) {
    return (vec2_type){.x = v0.x + v1.x, .y = v0.y + v1.y};
}

static inline vec2_type scaled_vec2(const vec2_type v0, const float t) {
    return (vec2_type){.x = t * v0.x, .y = t * v0.y};
}

static inline vec2_type src_to_dest_vec2(const vec2_type src, const vec2_type dest) {
    return sum_vec2(dest, scaled_vec2(src, -1));
}

static inline vec2_type lerp_vec2(const vec2_type v0, const vec2_type v1, const float t) {
    return (vec2_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t)};
}

static inline vec2_type normalize_vec2(const vec2_type v0) {
    return scaled_vec2(v0, 1.f / length_vec2(v0));
}

// Implementation (vec3):
// ----------------------------------------------------------------------------------------------------------

static inline vec3_type sum_vec3(const vec3_type v0, const vec3_type v1) {
    return (vec3_type){.x = v0.x + v1.x, .y = v0.y + v1.y, .z = v0.z + v1.z};
}

static inline vec3_type scaled_vec3(const vec3_type v0, const float t) {
    return (vec3_type){.x = t * v0.x, .y = t * v0.y, .z = t * v0.z};
}

static inline vec3_type src_to_dest_vec3(const vec3_type src, const vec3_type dest) {
    return sum_vec3(dest, scaled_vec3(src, -1));
}

static inline vec3_type lerp_vec3(const vec3_type v0, const vec3_type v1, const float t) {
    return (vec3_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t), .z = lerp_float(v0.z, v1.z, t)};
}

static inline vec3_type normalize_vec3(const vec3_type v0) {
    return scaled_vec3(v0, 1.f / length_vec3(v0));
}

// Implementation (vec4):
// ----------------------------------------------------------------------------------------------------------

static inline vec4_type sum_vec4(const vec4_type v0, const vec4_type v1) {
    return (vec4_type){.x = v0.x + v1.x, .y = v0.y + v1.y, .z = v0.z + v1.z, .w = v0.w + v1.w};
}

static inline vec4_type scaled_vec4(const vec4_type v0, const float t) {
    return (vec4_type){.x = t * v0.x, .y = t * v0.y, .z = t * v0.z, .w = t * v0.w};
}

static inline vec4_type src_to_dest_vec4(const vec4_type src, const vec4_type dest) {
    return sum_vec4(dest, scaled_vec4(src, -1));
}

static inline vec4_type lerp_vec4(const vec4_type v0, const vec4_type v1, const float t) {
    return (vec4_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t), .z = lerp_float(v0.z, v1.z, t)};
}

static inline vec4_type normalize_vec4(const vec4_type v0) {
    return scaled_vec4(v0, 1.f / length_vec4(v0));
}

// Implementation (transformation functions):
// ----------------------------------------------------------------------------------------------------------

static inline vec2_type rotate_vec2_around_origo(const vec2_type v0, const float angle_rad) {
    const float x_comp = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad);
    const float y_comp = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad);

    return (vec2_type){.x = x_comp, .y = y_comp};
}

static inline vec2_type project_from_world_space_to_screen_space(const vec3_type v0, const float fov_angle_rad,
                                                                 const float aspect_ratio) {
    assert(!is_equal_float(0.f, v0.z) && "Cannot divide by 0");
    assert(!is_equal_float(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");

    const float c = v0.z * tanf(fov_angle_rad / 2.f);

    return (vec2_type){.x = 1.f / c * aspect_ratio * v0.x, .y = 1.f / c * v0.y};
}

static inline vec3_type rotate_vec3_around_x_axis(const vec3_type v0, const float angle_rad) {
    const float x_comp = v0.x;
    const float y_comp = v0.y * cosf(angle_rad) - v0.z * sinf(angle_rad);
    const float z_comp = v0.y * sinf(angle_rad) + v0.z * cosf(angle_rad);

    return (vec3_type){.x = x_comp, .y = y_comp, .z = z_comp};
}

static inline vec3_type rotate_vec3_around_y_axis(const vec3_type v0, const float angle_rad) {
    const float x_comp = v0.x * cosf(angle_rad) + v0.z * sinf(angle_rad);
    const float y_comp = v0.y;
    const float z_comp = v0.z * cosf(angle_rad) - v0.x * sinf(angle_rad);

    return (vec3_type){.x = x_comp, .y = y_comp, .z = z_comp};
}

static inline vec3_type rotate_vec3_around_z_axis(const vec3_type v0, const float angle_rad) {
    const float x_comp = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad);
    const float y_comp = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad);
    const float z_comp = v0.z;

    return (vec3_type){.x = x_comp, .y = y_comp, .z = z_comp};
}

static inline vec4_type project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(
    const vec3_type v0, const float fov_angle_rad, const float aspect_ratio, const float z_near, const float z_far) {

    assert(!is_equal_float(0.f, tanf(fov_angle_rad / 2.f)) && "Cannot divide by 0");

    const float fov_inv_scalar = tanf(fov_angle_rad / 2.f);

    const float depth_scalar = z_far / (z_far - z_near);

    return (vec4_type){.x = 1 / fov_inv_scalar * aspect_ratio * v0.x,
                       .y = 1 / fov_inv_scalar * v0.y,
                       .z = depth_scalar * v0.z - depth_scalar * z_near,
                       .w = v0.z};
}

static inline vec3_type move_against_camera(const vec3_type vec, const vec3_type camera_pos, const float camera_angle_rad[3]) {

    const vec3_type v0 = sum_vec3(vec, scaled_vec3(camera_pos, -1));

    const vec3_type v1 = rotate_vec3_around_x_axis(v0, -camera_angle_rad[0]);

    const vec3_type v2 = rotate_vec3_around_y_axis(v1, -camera_angle_rad[1]);

    const vec3_type v3 = rotate_vec3_around_z_axis(v2, -camera_angle_rad[2]);

    return v3;
}
