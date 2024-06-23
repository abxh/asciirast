#pragma once

#include <math.h>
#include <stdbool.h>

#define PI 3.1415927f

typedef struct {
    int x;
    int y;
} vec2int;

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4;

// ----------------------------------------------------------------------------
// int math

static inline int signof_int(int expr) {
    return (expr > 0) - (expr < 0);
}

static inline int abs_int(int expr) {
    return (expr > 0) ? expr : -expr;
}

static inline int max_int(int x, int y) {
    return (x > y) ? x : y;
}

static inline int min_int(int x, int y) {
    return (x < y) ? x : y;
}

static inline int clamp_int(int x, int min, int max) {
    return min_int(max_int(x, min), max);
}

static inline int inside_range_int(int x, int min, int max) {
    return min <= x && x <= max;
}

// ----------------------------------------------------------------------------
// float math

#define FLOAT_DELTA (0.001f)

static inline int round_float_to_int(float expr) {
    float rounded_value = roundf(expr);
    return (int)rounded_value;
}

static inline bool is_equal_float(float x, float y) {
    return x - y <= FLOAT_DELTA;
}

static inline float max_float(float x, float y) {
    return (x > y + FLOAT_DELTA) ? x : y;
}

static inline float min_float(float x, float y) {
    return (x + FLOAT_DELTA < y) ? x : y;
}

static inline float clamp_float(float x, float min, float max) {
    return min_float(max_float(x, min), max);
}

static inline bool inside_range_float(float x, float min, float max) {
    return min <= x && x <= max;
}

static inline float lerp_float(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

// ----------------------------------------------------------------------------
// angle math

static inline float to_angle_in_radians(float angle_deg) {
    return angle_deg * PI / 180.f;
}

static inline float to_angle_in_degrees(float angle_rad) {
    return angle_rad * 180.f / PI;
}

// ----------------------------------------------------------------------------
// vec2int math

static inline vec2int sum_vec2int(vec2int a, vec2int b) {
    return (vec2int){.x = a.x + b.x, .y = a.y + b.y};
}

static inline vec2int scaled_vec2int(vec2int vec, float scalar) {
    return (vec2int){.x = round_float_to_int(scalar * (float)vec.x), .y = round_float_to_int(scalar * (float)vec.y)};
}

static inline int dot_vec2int(vec2int a, vec2int b) {
    return a.x * b.x + a.y * b.y;
}

static inline float length_vec2int(vec2int v) {
    int res = dot_vec2int(v, v);
    return sqrtf((float)res);
}

static inline int cross_vec2int(vec2int a, vec2int b) {
    return a.x * b.y - b.x * a.y;
}

static inline vec2int src_to_dest_vec2int(vec2int src, vec2int dest) {
    return (vec2int){.x = dest.x - src.x, .y = dest.y - src.y};
}

static inline vec2 to_vec2(vec2int v) {
    return (vec2){.x = (float)v.x, .y = (float)v.y};
}

static inline vec2int lerp_vec2int(vec2int v0, vec2int v1, float t) {
    return (vec2int){.x = round_float_to_int(lerp_float((float)v0.x, (float)v1.x, t)),
                     .y = round_float_to_int(lerp_float((float)v0.y, (float)v1.y, t))};
}

static inline vec2int normalize_vec2int(vec2int v) {
    return scaled_vec2int(v, 1.f / length_vec2int(v));
}

// ----------------------------------------------------------------------------
// vec2 math

static inline vec2 sum_vec2(vec2 a, vec2 b) {
    return (vec2){.x = a.x + b.x, .y = a.y + b.y};
}

static inline vec2 scaled_vec2(vec2 vec, float scalar) {
    return (vec2){.x = scalar * vec.x, .y = scalar * vec.y};
}

static inline float dot_vec2(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

static inline float length_vec2(vec2 v) {
    return sqrtf(dot_vec2(v, v));
}

static inline float cross_vec2(vec2 a, vec2 b) {
    return a.x * b.y - b.x * a.y;
}

static inline vec2 src_to_dest_vec2(vec2 src, vec2 dest) {
    return (vec2){.x = dest.x - src.x, .y = dest.y - src.y};
}

static inline vec2 rotate_around_origo_vec2(vec2 vec, float angle_rad) {
    return (vec2){.x = vec.x * cosf(angle_rad) - vec.y * sinf(angle_rad), .y = vec.x * sinf(angle_rad) + vec.y * cosf(angle_rad)};
}

static inline vec2 lerp_vec2(vec2 v0, vec2 v1, float t) {
    return (vec2){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t)};
}

static inline vec2 normalize_vec2(vec2 v) {
    return scaled_vec2(v, 1.f / length_vec2(v));
}

// ----------------------------------------------------------------------------
// vec3 math

static inline vec3 sum_vec3(vec3 a, vec3 b) {
    return (vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

static inline vec3 scaled_vec3(vec3 vec, float scalar) {
    return (vec3){.x = scalar * vec.x, .y = scalar * vec.y, .z = scalar * vec.z};
}

static inline float dot_vec3(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static inline float length_vec3(vec3 v) {
    return sqrtf(dot_vec3(v, v));
}

static inline vec3 cross_vec3(vec3 a, vec3 b) {
    return (vec3){.x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x};
}

static inline vec3 src_to_dest_vec3(vec3 src, vec3 dest) {
    return (vec3){.x = dest.x - src.x, .y = dest.y - src.y, .z = dest.z - src.z};
}

static inline vec3 rotate_around_x_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x, .y = vec.y * cosf(angle_rad) - vec.z * sinf(angle_rad), .z = vec.y * sinf(angle_rad) + vec.z * cosf(angle_rad)};
}

static inline vec3 rotate_around_y_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x * cosf(angle_rad) + vec.z * sinf(angle_rad), .y = vec.y, .z = vec.z * cosf(angle_rad) - vec.x * sinf(angle_rad)};
}

static inline vec3 rotate_around_z_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x * cosf(angle_rad) - vec.y * sinf(angle_rad), .y = vec.x * sinf(angle_rad) + vec.y * cosf(angle_rad), .z = vec.z};
}

static inline vec3 lerp_vec3(vec3 v0, vec3 v1, float t) {
    return (vec3){.x = lerp_float(v0.x, v1.x, t), .y = lerp_float(v0.y, v1.y, t), .z = lerp_float(v0.z, v1.z, t)};
}

static inline vec3 normalize_vec3(vec3 v) {
    return scaled_vec3(v, 1.f / length_vec3(v));
}

// ----------------------------------------------------------------------------
// vec4 math

static inline vec4 sum_vec4(vec4 a, vec4 b) {
    return (vec4){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
}

static inline vec4 scaled_vec4(vec4 vec, float scalar) {
    return (vec4){.x = scalar * vec.x, .y = scalar * vec.y, .z = scalar * vec.z, .w = scalar * vec.w};
}

static inline float dot_vec4(vec4 a, vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

static inline float length_vec4(vec4 v) {
    return sqrtf(dot_vec4(v, v));
}

static inline vec4 lerp_vec4(vec4 v0, vec4 v1, float t) {
    return (vec4){.x = lerp_float(v0.x, v1.x, t),
                  .y = lerp_float(v0.y, v1.y, t),
                  .z = lerp_float(v0.z, v1.z, t),
                  .w = lerp_float(v0.w, v1.w, t)};
}

static inline vec4 normalize_vec4(vec4 v) {
    return scaled_vec4(v, 1.f / length_vec4(v));
}

// ----------------------------------------------------------------------------
// 3d perspective projection

static inline vec2 vec3_projected_to_screen_space(vec3 vec, float fov_angle_rad, float aspect_ratio) {
    float scalar = vec.z * tanf(fov_angle_rad / 2.f);

    // there is a risk of division by 0. this should be caught by sanitizers
    return (vec2){.x = 1 / scalar * aspect_ratio * vec.x, .y = 1 / scalar * vec.y};
}

static inline vec4 vec3_apply_projection_matrix(vec3 vec, float fov_angle_rad, float aspect_ratio, float z_near, float z_far) {
    // fov scalar. to get the basic perspective projection
    float fov_scalar = tanf(fov_angle_rad / 2.f);

    // z-culling schenanigan. used to make near objects more precise, far objects less precise
    float depth_scalar = z_far / (z_far - z_near);

    // there is a risk of division by 0 when dividing by fov_scalar. this should be caught by sanitizers

    return (vec4){.x = 1 / fov_scalar * aspect_ratio * vec.x,
                  .y = 1 / fov_scalar * vec.y,
                  .z = depth_scalar * vec.z - depth_scalar * z_near,
                  .w = vec.z};

    // res to be multiplied by the scalar (1 / w) for z-divide
}

static inline vec3 vec3_apply_basic_camera_matrix(vec3 vec, vec3 camera_pos, float camera_orientation[3]) {
    vec3 v0 = sum_vec3(vec, scaled_vec3(camera_pos, -1));

    vec3 v1 = rotate_around_x_axis(v0, -camera_orientation[0]);
    vec3 v2 = rotate_around_y_axis(v1, -camera_orientation[1]);
    vec3 v3 = rotate_around_z_axis(v2, -camera_orientation[2]);

    return v3;
}
