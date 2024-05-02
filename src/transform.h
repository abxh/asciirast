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

float conv_to_angle_rad(float angle_deg);

float flip_sign(float x);

float clamp(float x, float min, float max);

vec2 rotate_around_origo_2d(vec2 vec, float angle_rad);
