#include <math.h>
#include <stdlib.h>

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

float to_angle_rad(float angle_deg) {
    return angle_deg * M_PI / 180;
}

float rotate_x_around_origo(float x, float y, float angle_rad) {
    return x * cosf(angle_rad) - y * sinf(angle_rad);
}

float rotate_y_around_origo(float x, float y, float angle_rad) {
    return x * sinf(angle_rad) + y * cosf(angle_rad);
}
