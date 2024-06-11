#pragma once

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

// int math:
int signof_int(int expr);
int abs_int(int expr);
int max_int(int x, int y);
int min_int(int x, int y);
int clamp_int(int x, int min, int max);
int inside_range_int(int x, int min, int max);

// float math:
int cmp_float(float x, float y);
int round_float_to_int(float expr);
float max_float(float x, float y);
float min_float(float x, float y);
float clamp_float(float x, float min, float max);
float inside_range_float(float x, float min, float max);
float lerp_float(float v0, float v1, float t);

// angle math:
float to_angle_in_radians(float angle_deg);
float to_angle_in_degrees(float angle_rad);

// vec2int math:
vec2int sum_vec2int(vec2int a, vec2int b);
vec2int scaled_vec2int(vec2int vec, float scalar);
int dot_vec2int(vec2int a, vec2int b);
float length_vec2int(vec2int v);
int cross_vec2int(vec2int a, vec2int b);
vec2int src_to_dest_vec2int(vec2int src, vec2int dest);
vec2 to_vec2(vec2int v);

// vec2 math:
vec2 sum_vec2(vec2 a, vec2 b);
vec2 scaled_vec2(vec2 vec, float scalar);
float dot_vec2(vec2 a, vec2 b);
float length_vec2(vec2 v);
float cross_vec2(vec2 a, vec2 b);
vec2 src_to_dest_vec2(vec2 src, vec2 dest);
vec2 rotate_around_origo_vec2(vec2 vec, float angle_rad);

// vec3 math:
vec3 sum_vec3(vec3 a, vec3 b);
vec3 scaled_vec3(vec3 vec, float scalar);
float dot_vec3(vec3 a, vec3 b);
float length_vec3(vec3 v);
vec3 cross_vec3(vec3 a, vec3 b);
vec3 src_to_dest_vec3(vec3 src, vec3 dest);
vec3 rotate_around_x_axis(vec3 vec, float angle_rad);
vec3 rotate_around_y_axis(vec3 vec, float angle_rad);
vec3 rotate_around_z_axis(vec3 vec, float angle_rad);

// vec4 math:
vec4 sum_vec4(vec4 a, vec4 b);
vec4 scaled_vec4(vec4 vec, float scalar);
float dot_vec4(vec4 a, vec4 b);
float length_vec4(vec4 v);

// 3d perspective projection:
vec2 vec3_projected_to_screen_space(vec3 vec, float fov_angle_rad, float aspect_ratio);
vec4 vec3_apply_projection_matrix(vec3 vec, float fov_angle_rad, float aspect_ratio, float z_near, float z_far);
