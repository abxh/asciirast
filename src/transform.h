#pragma once

#include "external.h"

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
static inline int sum_int(const int v0, const int v1);
static inline int abs_int(const int v0);
static inline int max_int(const int v0, const int v1);
static inline int min_int(const int v0, const int v1);
static inline int clamp_int(const int v0, const int min, const int max);

// vec2int functions:
static inline vec2int_type sum_vec2int(const vec2int_type v0, const vec2int_type v1);

// float functions:
static inline float add_float(const float v0, const float v1);
static inline float scale_float(const float v0, const float t);
static inline float abs_float(const float v0);
static inline float max_float(const float v0, const float v1);
static inline float min_float(const float v0, const float v1);
static inline float clamp_float(const float v0, const float min, const float max);
static inline float lerp_float(const float v0, const float v1, const float t);

// vec2 functions:
static inline vec2_type add_vec2(const vec2_type v0, const vec2_type v1);
static inline vec2_type scale_vec2(const vec2_type v0, const float t);
static inline vec2_type lerp_vec2(const vec2_type v0, const vec2_type v1, const float t);
static inline vec2_type normalize_vec2(const vec2_type v0);

// vec3 functions:
static inline vec3_type add_vec3(const vec3_type v0, const vec3_type v1);
static inline vec3_type scale_vec3(const vec3_type v0, const float t);
static inline vec3_type lerp_vec3(const vec3_type v0, const vec3_type v1, const float t);
static inline vec3_type normalize_vec3(const vec3_type v0);

// vec4 functions:
static inline vec4_type add_vec4(const vec4_type v0, const vec4_type v1);
static inline vec4_type scale_vec4(const vec4_type v0, const float t);
static inline vec4_type lerp_vec4(const vec4_type v0, const vec4_type v1, const float t);
static inline vec4_type normalize_vec4(const vec4_type v0);

// type change functions:
#define FROM_ANGLE_RAD_TO_ANGLE_DEG(expr) ((expr) * PI / 180.f)
#define FROM_ANGLE_DEG_TO_ANGLE_RAD(expr) ((expr) * 180.f / PI)
static inline float from_int_to_float(const int v0);
static inline int from_float_to_int_truncated(const float v0);
static inline int from_float_to_int_rounded(const float v0);
static inline vec2_type from_vec2int_to_vec2(const vec2int_type v0);
static inline vec2int_type from_vec2_to_vec2int_truncated(const vec2_type v0);
static inline vec2int_type from_vec2_to_vec2int_rounded(const vec2_type v0);

// transformation functions:
static inline vec2_type rotate_vec2_around_origo(const vec2_type v0, const float angle_rad);
static inline vec2_type project_from_world_space_to_screen_space(const vec3_type v0, const float fov_angle_rad,
                                                                 const float aspect_ratio);
static inline vec3_type rotate_vec3_around_x(const vec3_type v0, const float angle_rad);
static inline vec3_type rotate_vec3_around_y(const vec3_type v0, const float angle_rad);
static inline vec3_type rotate_vec3_around_z(const vec3_type v0, const float angle_rad);
static inline vec4_type project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(
    const vec4_type vec, const float fov_angle_rad, const float aspect_ratio, const float z_near, const float z_far);

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
    return (vec3_type){.x = v0.y * v1.z - v0.z * v1.y, .y = v0.z * v1.x - v0.x * v1.z, .z = v0.x * v1.y - v0.y * v1.x};
}

static inline int cross_vec2int(const vec2int_type v0, const vec2int_type v1) {
    return v0.x * v1.y - v1.x * v0.y;
}

static inline float cross_vec2(const vec2_type v0, const vec2_type v1) {
    return v0.x * v1.y - v1.x * v0.y;
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

static inline int sum_int(const int v0, const int v1) {
    return v0 + v1;
}

static inline int abs_int(const int v0) {
    return (v0 >= 0) ? v0 : -v0;
}

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
    return (vec2int_type){.x = sum_int(v0.x, v1.x), .y = sum_int(v0.y, v1.y)};
}

// Implementation (float):
// ----------------------------------------------------------------------------------------------------------

static inline float add_float(const float v0, const float v1) {
    return v0 + v1;
}

static inline float scale_float(const float v0, const float t) {
    return v0 * t;
}

static inline float abs_float(const float v0) {
    return (v0 >= 0.0f) ? v0 : -v0;
}

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
    return (1 - t) * v0 + t * v1;
}

// Implementation (vec2):
// ----------------------------------------------------------------------------------------------------------

static inline vec2_type add_vec2(const vec2_type v0, const vec2_type v1) {
    return (vec2_type){.x = v0.x + v1.x, .y = v0.y + v1.y};
}

static inline vec2_type scale_vec2_type(const vec2_type v0, const float t) {
    return (vec2_type){.x = t * v0.x, .y = t * v0.y};
}

static inline vec2_type lerp_vec2(const vec2_type v0, const vec2_type v1, const float t) {
    return (vec2_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t)};
}

// Implementation (vec3):
// ----------------------------------------------------------------------------------------------------------

static inline vec3_type add_vec3(const vec3_type v0, const vec3_type v1) {
    return (vec3_type){.x = v0.x + v1.x, .y = v0.y + v1.y, .z = v0.z + v1.z};
}

static inline vec3_type scale_vec3(const vec3_type v0, const float t) {
    return (vec3_type){.x = t * v0.x, .y = t * v0.y, .z = t * v0.z};
}

static inline vec3_type lerp_vec3(const vec3_type v0, const vec3_type v1, const float t) {
    return (vec3_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t), .z = lerp_float(v0.z, v1.z, t)};
}

// Implementation (vec4):
// ----------------------------------------------------------------------------------------------------------

static inline vec4_type add_vec4(const vec4_type v0, const vec4_type v1) {
    return (vec4_type){.x = v0.x + v1.x, .y = v0.y + v1.y, .z = v0.z + v1.z, .w = v0.w + v1.w};
}

static inline vec4_type scale_vec4_type(const vec4_type v0, const float t) {
    return (vec4_type){.x = t * v0.x, .y = t * v0.y, .z = t * v0.z, .w = t * v0.w};
}

static inline vec4_type lerp_vec4(const vec4_type v0, const vec4_type v1, const float t) {
    return (vec4_type){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t), .z = lerp_float(v0.z, v1.z, t)};
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
    return (int)roundf(v0);
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

// Implementation (transformation functions):
// ----------------------------------------------------------------------------------------------------------

static inline vec2_type rotate_vec2_around_origo(const vec2_type v0, const float angle_rad) {
    return (vec2_type){.x = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad), .y = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad)};
}

static inline vec2_type project_from_world_space_to_screen_space(const vec3_type v0, const float fov_angle_rad,
                                                                 const float aspect_ratio) {
    assert(!is_equal_float(0.f, v0.z));
    assert(!is_equal_float(0.f, tanf(fov_angle_rad / 2.f)));

    const float fov_depth_scalar = v0.z * tanf(fov_angle_rad / 2.f);
    return (vec2_type){.x = 1 / fov_depth_scalar * aspect_ratio * v0.x, .y = 1 / fov_depth_scalar * v0.y};
}

static inline vec3_type rotate_around_x_axis(const vec3_type v0, const float angle_rad) {
    return (vec3_type){
        .x = v0.x, .y = v0.y * cosf(angle_rad) - v0.z * sinf(angle_rad), .z = v0.y * sinf(angle_rad) + v0.z * cosf(angle_rad)};
}

static inline vec3_type rotate_around_y_axis(const vec3_type v0, const float angle_rad) {
    return (vec3_type){
        .x = v0.x * cosf(angle_rad) + v0.z * sinf(angle_rad), .y = v0.y, .z = v0.z * cosf(angle_rad) - v0.x * sinf(angle_rad)};
}

static inline vec3_type rotate_around_z_axis(const vec3_type v0, const float angle_rad) {
    return (vec3_type){
        .x = v0.x * cosf(angle_rad) - v0.y * sinf(angle_rad), .y = v0.x * sinf(angle_rad) + v0.y * cosf(angle_rad), .z = v0.z};
}

static inline vec4_type project_from_world_space_to_screen_space_w_info_perserved_and_no_z_divide(
    const vec4_type v0, const float fov_angle_rad, const float aspect_ratio, const float z_near, const float z_far) {
    assert(!is_equal_float(0.f, tanf(fov_angle_rad / 2.f)));

    const float fov_scalar = tanf(fov_angle_rad / 2.f);
    const float depth_scalar = z_far / (z_far - z_near);

    return (vec4_type){.x = 1 / fov_scalar * aspect_ratio * v0.x,
                       .y = 1 / fov_scalar * v0.y,
                       .z = depth_scalar * v0.z - depth_scalar * z_near,
                       .w = v0.z};
}

static inline vec3_type apply_camera_transformation(const vec3_type vec, const vec3_type camera_pos, const float camera_angle_rad_x,
                                                    const float camera_angle_rad_y, const float camera_angle_rad_z) {
    const vec3_type v0 = add_vec3(vec, scale_vec3(camera_pos, -1));
    const vec3_type v1 = rotate_around_x_axis(v0, -camera_angle_rad_x);
    const vec3_type v2 = rotate_around_y_axis(v1, -camera_angle_rad_y);
    const vec3_type v3 = rotate_around_z_axis(v2, -camera_angle_rad_z);
    return v3;
}
