#pragma once

typedef struct {
    float x;
    float y;
} vec2;

typedef struct {
    float x;
    float y;
    float z;
} vec3;

// int math:
int diff_int(int left_expr, int right_expr);
int abs_int(int expr);

// float math:
float flip_sign_float(float x);
float clamp_float(float x, float min, float max);
int cmp_float(float x, float y);

// angle math:
float to_angle_in_radians(float angle_deg);
float to_angle_in_degrees(float angle_rad);

float to_fov_constant_from_radians(float angle_deg);
float to_fov_constant_from_degrees(float angle_rad);

// vec2 math:
vec2 sum_vec2(vec2 a, vec2 b);
vec2 scaled_vec2(vec2 vec, float scalar);
vec2 rotate_around_origo_vec2(vec2 vec, float angle_rad);

// vec3 math:
vec3 sum_vec3(vec3 a, vec3 b);
vec3 scaled_vec3(vec3 vec, float scalar);
vec2 vec3_projected_as_vec2(vec3 vec, float fov_const);

vec3 rotate_around_x_axis(vec3 vec, float angle_rad);
vec3 rotate_around_y_axis(vec3 vec, float angle_rad);
vec3 rotate_around_z_axis(vec3 vec, float angle_rad);
