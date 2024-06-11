#include <assert.h>
#include <math.h>

#include "transform.h"

// ----------------------------------------------------------------------------
// int math

int signof_int(int expr) {
    return (expr > 0) - (expr < 0);
}

int abs_int(int expr) {
    return (expr > 0) ? expr : -expr;
}

int max_int(int x, int y) {
    return (x > y) ? x : y;
}

int min_int(int x, int y) {
    return (x < y) ? x : y;
}

int clamp_int(int x, int min, int max) {
    return min_int(max_int(x, min), max);
}

int inside_range_int(int x, int min, int max) {
    return min <= x && x <= max;
}

// ----------------------------------------------------------------------------
// float math

static const float float_delta = 0.001f;

int round_float_to_int(float expr) {
    return (expr > (floorf(expr) + 0.5f + float_delta)) ? ceilf(expr) : floorf(expr);
}

int cmp_float(float x, float y) {
    return (x > y + float_delta) - (x + float_delta < y);
}

float max_float(float x, float y) {
    return (x > y + float_delta) ? x : y;
}

float min_float(float x, float y) {
    return (x + float_delta < y) ? x : y;
}

float clamp_float(float x, float min, float max) {
    return min_float(max_float(x, min), max);
}

float inside_range_float(float x, float min, float max) {
    return min <= x && x <= max;
}

float lerp_float(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}

// ----------------------------------------------------------------------------
// angle math

float to_angle_in_radians(float angle_deg) {
    return angle_deg * M_PI / 180.f;
}

float to_angle_in_degrees(float angle_rad) {
    return angle_rad * 180.f / M_PI;
}

// ----------------------------------------------------------------------------
// vec2int math

vec2int sum_vec2int(vec2int a, vec2int b) {
    return (vec2int){.x = a.x + b.x, .y = a.y + b.y};
}

vec2int scaled_vec2int(vec2int vec, float scalar) {
    return (vec2int){.x = scalar * vec.x, .y = scalar * vec.y};
}

int dot_vec2int(vec2int a, vec2int b) {
    return a.x * b.x + a.y * b.y;
}

float length_vec2int(vec2int v) {
    return sqrtf(dot_vec2int(v, v));
}

int cross_vec2int(vec2int a, vec2int b) {
    // magnitude of the "imaginary" z-component, if a and b were 3d vectors and the cross product is taken.
    return a.x * b.y - b.x * a.y;
}

vec2int src_to_dest_vec2int(vec2int src, vec2int dest) {
    return (vec2int){.x = dest.x - src.x, .y = dest.y - src.y};
}

vec2 to_vec2(vec2int v) {
    return (vec2){.x = v.x, .y = v.y};
}

// ----------------------------------------------------------------------------
// vec2 math

vec2 sum_vec2(vec2 a, vec2 b) {
    return (vec2){.x = a.x + b.x, .y = a.y + b.y};
}

vec2 scaled_vec2(vec2 vec, float scalar) {
    return (vec2){.x = scalar * vec.x, .y = scalar * vec.y};
}

float dot_vec2(vec2 a, vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float length_vec2(vec2 v) {
    return sqrtf(dot_vec2(v, v));
}

float cross_vec2(vec2 a, vec2 b) {
    // magnitude of the "imaginary" z-component, if a and b were 3d vectors and the cross product is taken.
    return a.x * b.y - b.x * a.y;
}

vec2 src_to_dest_vec2(vec2 src, vec2 dest) {
    return (vec2){.x = dest.x - src.x, .y = dest.y - src.y};
}

vec2 rotate_around_origo_vec2(vec2 vec, float angle_rad) {
    return (vec2){.x = vec.x * cosf(angle_rad) - vec.y * sinf(angle_rad), .y = vec.x * sinf(angle_rad) + vec.y * cosf(angle_rad)};
}

// ----------------------------------------------------------------------------
// vec3 math

vec3 sum_vec3(vec3 a, vec3 b) {
    return (vec3){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z};
}

vec3 scaled_vec3(vec3 vec, float scalar) {
    return (vec3){.x = scalar * vec.x, .y = scalar * vec.y, .z = scalar * vec.z};
}

float dot_vec3(vec3 a, vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float length_vec3(vec3 v) {
    return sqrtf(dot_vec3(v, v));
}

vec3 cross_vec3(vec3 a, vec3 b) {
    // (a2b3 - a3b2, a3b1 - a1b3, a1b2 - a2b1)
    return (vec3){.x = a.y * b.z - a.z * b.y, .y = a.z * b.x - a.x * b.z, .z = a.x * b.y - a.y * b.x};
}

vec3 src_to_dest_vec3(vec3 src, vec3 dest) {
    return (vec3){.x = dest.x - src.x, .y = dest.y - src.y, .z = dest.z - src.z};
}

vec3 rotate_around_x_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x, .y = vec.y * cosf(angle_rad) - vec.z * sinf(angle_rad), .z = vec.y * sinf(angle_rad) + vec.z * cosf(angle_rad)};
}

vec3 rotate_around_y_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x * cosf(angle_rad) + vec.z * sinf(angle_rad), .y = vec.y, .z = vec.z * cosf(angle_rad) - vec.x * sinf(angle_rad)};
}

vec3 rotate_around_z_axis(vec3 vec, float angle_rad) {
    return (vec3){
        .x = vec.x * cosf(angle_rad) - vec.y * sinf(angle_rad), .y = vec.x * sinf(angle_rad) + vec.y * cosf(angle_rad), .z = vec.z};
}

// ----------------------------------------------------------------------------
// vec4 math

vec4 sum_vec4(vec4 a, vec4 b) {
    return (vec4){.x = a.x + b.x, .y = a.y + b.y, .z = a.z + b.z, .w = a.w + b.w};
}

vec4 scaled_vec4(vec4 vec, float scalar) {
    return (vec4){.x = scalar * vec.x, .y = scalar * vec.y, .z = scalar * vec.z, .w = scalar * vec.w};
}

float dot_vec4(vec4 a, vec4 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

float length_vec4(vec4 v) {
    return sqrtf(dot_vec4(v, v));
}

// ----------------------------------------------------------------------------
// 3d perspective projection

vec2 vec3_projected_to_screen_space(vec3 vec, float fov_angle_rad, float aspect_ratio) {
    float scalar = vec.z * tanf(fov_angle_rad / 2.f);
    assert("fov angle or z-value does not lead to division by zero" && cmp_float(scalar, 0) != 0);

    return (vec2){.x = 1 / scalar * aspect_ratio * vec.x, .y = 1 / scalar * vec.y};
}

vec4 vec3_apply_projection_matrix(vec3 vec, float fov_angle_rad, float aspect_ratio, float z_near, float z_far) {
    float fov_scalar = tanf(fov_angle_rad / 2.f);
    assert("fov angle does not lead to division by zero" && cmp_float(fov_scalar, 0) != 0);

    // z-culling schenanigan. used to make near objects more precise, far objects less precise
    float depth_scalar = z_far / (z_far - z_near);

    // res to be multiplied by the scalar (1 / w) for z-divide
    return (vec4){.x = 1 / fov_scalar * aspect_ratio * vec.x,
                  .y = 1 / fov_scalar * vec.y,
                  .z = depth_scalar * vec.z - depth_scalar * z_near,
                  .w = vec.z};
}
