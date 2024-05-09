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
int abs_int(int expr);
int max_int(int x, int y);
int min_int(int x, int y);

// float math:
int cmp_float(float x, float y);

int round_float_to_int(float expr);
float max_float(float x, float y);
float min_float(float x, float y);
float clamp_float(float x, float min, float max);

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
vec3 vec2_unprojected_as_vec3(vec2 vec, float fov_const);

vec3 rotate_around_x_axis(vec3 vec, float angle_rad);
vec3 rotate_around_y_axis(vec3 vec, float angle_rad);
vec3 rotate_around_z_axis(vec3 vec, float angle_rad);


