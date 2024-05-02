#include <math.h>

#include "transform.h"

float flip_sign(float x) {
    return -x;
}

float clamp(float x, float min, float max) {
    float delta = 0.001f;
    if (x + delta < min) {
        return min;
    } else if (max < x - delta) {
        return max;
    }
    return x;
}

float conv_to_angle_rad(float angle_deg) {
    return angle_deg * M_PI / 180;
}

vec2 rotate_around_origo_2d(vec2 vec, float angle_rad) {
    return (vec2){.x = vec.x * cosf(angle_rad) - vec.y * sinf(angle_rad), .y = vec.x * sinf(angle_rad) + vec.y * cosf(angle_rad)};
}
