// taken from - and further modified:
// https://github.com/datenwolf/linmath.h/blob/master/linmath.h
//
// Originally under public domain.
//
// Modifications may have introduced errors. The modifications are not tested. Use the original if in doubt.

#pragma once

#include "math/vec.h"

typedef float mat4x4_type[4][4];
typedef float quat_type[4];

static inline void quat_add(quat_type res, const quat_type q0, const quat_type q1) {
    vec4_to_array(res, vec4_add(vec4_from_array(q0), vec4_from_array(q1)));
}

static inline void quat_sub(quat_type res, const quat_type q0, const quat_type q1) {
    vec4_to_array(res, vec4_sub(vec4_from_array(q0), vec4_from_array(q1)));
}

static inline void quat_norm(quat_type res, const quat_type q0) {
    vec4_to_array(res, vec4_norm(vec4_from_array(q0)));
}

static inline void quat_scale(quat_type res, const quat_type q0, const float t) {
    vec4_to_array(res, vec4_scale(vec4_from_array(q0), t));
}

static inline float quat_dot(const quat_type q0, const quat_type q1) {
    return vec4_dot(vec4_from_array(q0), vec4_from_array(q1));
}

static inline void quat_identity(quat_type res) {
    res[0] = res[1] = res[2] = 0.f;
    res[3] = 1.f;
}

static inline void quat_mul(quat_type res, const quat_type p, const quat_type q) {
    // vec3_type w, tmp;

    vec3_type tmp = vec3_cross(vec3_from_array(p), vec3_from_array(q));
    vec3_type w = vec3_scale(vec3_from_array(p), q[3]);
    tmp = vec3_add(tmp, w);
    w = vec3_scale(vec3_from_array(q), p[3]);
    tmp = vec3_add(tmp, w);

    vec3_to_array(res, tmp);
    res[3] = p[3] * q[3] - vec3_dot(vec3_from_array(p), vec3_from_array(q));
}

static inline void quat_conj(quat_type res, const quat_type q) {
    for (size_t i = 0; i < 3; ++i) {
        res[i] = -q[i];
    }
    res[3] = q[3];
}

static inline void quat_rotate(quat_type res, const float angle, const vec3_type axis) {
    vec3_type axis_norm = vec3_norm(axis);
    float s = sinf(angle / 2);
    float c = cosf(angle / 2);
    vec3_to_array(res, vec3_scale(axis_norm, s));
    res[3] = c;
}

static inline vec3_type quat_mul_vec3(const quat_type q, const vec3_type v) {
    /*
     * Method by Fabian 'ryg' Giessen (of Farbrausch)
    t = 2 * cross(q.xyz, v)
    v' = v + q.w * t + cross(q.xyz, t)
     */
    vec3_type t;
    vec3_type q_xyz = vec3_from_array(q);
    vec3_type u = vec3_from_array(q);

    t = vec3_scale(vec3_cross(q_xyz, v), 2);
    u = vec3_cross(q_xyz, t);

    return vec3_add(vec3_add(v, vec3_scale(t, q[3])), u);
}

static inline void quat_mul_vec3_alt(float res[3], const quat_type q, const float v[3]) {
    /*
     * Method by Fabian 'ryg' Giessen (of Farbrausch)
    t = 2 * cross(q.xyz, v)
    v' = v + q.w * t + cross(q.xyz, t)
     */
    vec3_type t;
    vec3_type q_xyz = vec3_from_array(q);
    vec3_type u = vec3_from_array(q);

    t = vec3_scale(vec3_cross(q_xyz, vec3_from_array(v)), 2);
    u = vec3_cross(q_xyz, t);
    vec3_to_array(res, vec3_add(vec3_add(vec3_from_array(v), vec3_scale(t, q[3])), u));
}

static inline void quat_from_mat4x4(quat_type res, const mat4x4_type M) {
    float r = 0.f;

    int perm[] = {0, 1, 2, 0, 1};
    int* p = perm;

    for (size_t i = 0; i < 3; i++) {
        float m = M[i][i];
        if (m < r) {
            continue;
        }
        m = r;
        p = &perm[i];
    }

    r = sqrtf(1.f + M[p[0]][p[0]] - M[p[1]][p[1]] - M[p[2]][p[2]]);

    if (r < (float)(1e-6)) {
        res[0] = 1.f;
        res[1] = res[2] = res[3] = 0.f;
        return;
    }

    res[0] = r / 2.f;
    res[1] = (M[p[0]][p[1]] - M[p[1]][p[0]]) / (2.f * r);
    res[2] = (M[p[2]][p[0]] - M[p[0]][p[2]]) / (2.f * r);
    res[3] = (M[p[2]][p[1]] - M[p[1]][p[2]]) / (2.f * r);
}
