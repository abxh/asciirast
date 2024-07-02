// taken from - and further modified:
// https://github.com/datenwolf/linmath.h/blob/master/linmath.h
//
// Under public domain.
//
// May calculate inplace operations incorrectly. 

#pragma once

#include "vec.h"

typedef float quat_type[4];

#define quat_add vec4_add
#define quat_sub vec4_sub
#define quat_norm vec4_norm
#define quat_scale vec4_scale
#define quat_dot vec4_dot

static inline void quat_identity(quat_type res) {
    res[0] = res[1] = res[2] = 0.f;
    res[3] = 1.f;
}

static inline void quat_mul(quat_type res, const quat_type p, const quat_type q) {
    vec3_type w, tmp;

    vec3_cross(tmp, p, q);
    vec3_scale(w, p, q[3]);
    vec3_add(tmp, tmp, w);
    vec3_scale(w, q, p[3]);
    vec3_add(tmp, tmp, w);

    vec3_copy(res, tmp);
    res[3] = p[3] * q[3] - vec3_dot(p, q);
}

static inline void quat_conj(quat_type res, const quat_type q) {
    for (size_t i = 0; i < 3; ++i) {
        res[i] = -q[i];
    }
    res[3] = q[3];
}

static inline void quat_rotate(quat_type res, const float angle, const vec3_type axis) {
    vec3_type axis_norm;
    vec3_norm(axis_norm, axis);
    float s = sinf(angle / 2);
    float c = cosf(angle / 2);
    vec3_scale(res, axis_norm, s);
    res[3] = c;
}

static inline void quat_mul_vec3(vec3_type res, const quat_type q, const vec3_type v) {
    /*
     * Method by Fabian 'ryg' Giessen (of Farbrausch)
    t = 2 * cross(q.xyz, v)
    v' = v + q.w * t + cross(q.xyz, t)
     */
    vec3_type t;
    vec3_type q_xyz = {q[0], q[1], q[2]};
    vec3_type u = {q[0], q[1], q[2]};

    vec3_cross(t, q_xyz, v);
    vec3_scale(t, t, 2);

    vec3_cross(u, q_xyz, t);
    vec3_scale(t, t, q[3]);

    vec3_add(res, v, t);
    vec3_add(res, res, u);
}


