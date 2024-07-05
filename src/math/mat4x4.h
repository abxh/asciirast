// taken from - and further modified:
// https://github.com/datenwolf/linmath.h/blob/master/linmath.h
//
// Under public domain.
//
// May calculate inplace operations incorrectly.
//
// Column-major matrix

#pragma once

#include "vec.h"

#include <string.h>

typedef vec4_type mat4x4_type[4];

static inline void mat4x4_identity(mat4x4_type res) {
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            res[i][j] = i == j ? 1.f : 0.f;
        }
    }
}

static inline void mat4x4_copy(mat4x4_type res, const mat4x4_type N) {
    for (size_t i = 0; i < 4; ++i) {
        vec4_copy(res[i], N[i]);
    }
}

static inline void mat4x4_row(vec4_type res, const mat4x4_type M, const size_t i) {
    for (size_t k = 0; k < 4; ++k) {
        res[k] = M[k][i];
    }
}

static inline void mat4x4_col(vec4_type res, const mat4x4_type M, const size_t i) {
    for (size_t k = 0; k < 4; ++k) {
        res[k] = M[i][k];
    }
}

static inline void mat4x4_transpose(mat4x4_type res, const mat4x4_type N) {
    // Note: if M and N are the same, the user has to
    // explicitly make a copy of M and set it to N.
    for (size_t j = 0; j < 4; ++j) {
        for (size_t i = 0; i < 4; ++i) {
            res[i][j] = N[j][i];
        }
    }
}

static inline void mat4x4_add(mat4x4_type res, const mat4x4_type a, const mat4x4_type b) {
    for (size_t i = 0; i < 4; ++i) {
        vec4_add(res[i], a[i], b[i]);
    }
}

static inline void mat4x4_sub(mat4x4_type res, const mat4x4_type a, const mat4x4_type b) {
    for (size_t i = 0; i < 4; ++i) {
        vec4_sub(res[i], a[i], b[i]);
    }
}

static inline void mat4x4_scale(mat4x4_type res, const mat4x4_type a, const float k) {
    for (size_t i = 0; i < 4; ++i)
        vec4_scale(res[i], a[i], k);
}

static inline void mat4x4_scale_aniso(mat4x4_type res, const mat4x4_type a, const float x, const float y, const float z) {
    vec4_scale(res[0], a[0], x);
    vec4_scale(res[1], a[1], y);
    vec4_scale(res[2], a[2], z);
    vec4_copy(res[3], a[3]);
}

static inline void mat4x4_mul(mat4x4_type res, const mat4x4_type a, const mat4x4_type b) {
    mat4x4_type temp;
    for (size_t c = 0; c < 4; ++c) {
        for (size_t r = 0; r < 4; ++r) {
            temp[c][r] = 0.f;
            for (size_t k = 0; k < 4; ++k) {
                temp[c][r] += a[k][r] * b[c][k];
            }
        }
    }
    mat4x4_copy(res, temp);
}

static inline void mat4x4_mul_vec4(vec4_type res, const mat4x4_type M, const vec4_type v) {
    for (size_t j = 0; j < 4; ++j) {
        res[j] = 0.f;
        for (size_t i = 0; i < 4; ++i) {
            res[j] += M[i][j] * v[i];
        }
    }
}

static inline void mat4x4_translate(mat4x4_type res, const float x, const float y, const float z) {
    mat4x4_identity(res);
    res[3][0] = x;
    res[3][1] = y;
    res[3][2] = z;
}

static inline void mat4x4_translate_in_place(mat4x4_type res, const float x, const float y, const float z) {
    const vec4_type t = {x, y, z, 0};
    vec4_type r;
    for (size_t i = 0; i < 4; ++i) {
        mat4x4_row(r, res, i);
        res[3][i] += vec4_dot(r, t);
    }
}

static inline void mat4x4_from_vec3_mul_outer(mat4x4_type res, const vec3_type a, const vec3_type b) {
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            res[i][j] = i < 3 && j < 3 ? a[i] * b[j] : 0.f;
        }
    }
}

static inline void mat4x4_rotate(mat4x4_type res, const mat4x4_type M, const float x, const float y, const float z,
                                 const float angle_rad) {
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    vec3_type u = {x, y, z};

    if (vec3_length(u) > (float)(1e-4)) {
        vec3_norm(u, u);
        mat4x4_type T;
        mat4x4_from_vec3_mul_outer(T, u, u);

        mat4x4_type S = {{0, u[2], -u[1], 0}, {-u[2], 0, u[0], 0}, {u[1], -u[0], 0, 0}, {0, 0, 0, 0}};
        mat4x4_scale(S, S, s);

        mat4x4_type C;
        mat4x4_identity(C);
        mat4x4_sub(C, C, T);

        mat4x4_scale(C, C, c);

        mat4x4_add(T, T, C);
        mat4x4_add(T, T, S);

        T[3][3] = 1.f;
        mat4x4_mul(res, M, T);
    } else {
        mat4x4_copy(res, M);
    }
}
static inline void mat4x4_rotate_X(mat4x4_type res, const mat4x4_type M, const float angle_rad) {
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    mat4x4_type R = {{1.f, 0.f, 0.f, 0.f}, {0.f, c, s, 0.f}, {0.f, -s, c, 0.f}, {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(res, M, R);
}

static inline void mat4x4_rotate_Y(mat4x4_type res, const mat4x4_type M, const float angle_rad) {
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    mat4x4_type R = {{c, 0.f, -s, 0.f}, {0.f, 1.f, 0.f, 0.f}, {s, 0.f, c, 0.f}, {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(res, M, R);
}

static inline void mat4x4_rotate_Z(mat4x4_type res, const mat4x4_type M, const float angle_rad) {
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    mat4x4_type R = {{c, s, 0.f, 0.f}, {-s, c, 0.f, 0.f}, {0.f, 0.f, 1.f, 0.f}, {0.f, 0.f, 0.f, 1.f}};
    mat4x4_mul(res, M, R);
}

static inline void mat4x4_invert(mat4x4_type res, const mat4x4_type M) {
    float s[6];
    float c[6];
    s[0] = M[0][0] * M[1][1] - M[1][0] * M[0][1];
    s[1] = M[0][0] * M[1][2] - M[1][0] * M[0][2];
    s[2] = M[0][0] * M[1][3] - M[1][0] * M[0][3];
    s[3] = M[0][1] * M[1][2] - M[1][1] * M[0][2];
    s[4] = M[0][1] * M[1][3] - M[1][1] * M[0][3];
    s[5] = M[0][2] * M[1][3] - M[1][2] * M[0][3];

    c[0] = M[2][0] * M[3][1] - M[3][0] * M[2][1];
    c[1] = M[2][0] * M[3][2] - M[3][0] * M[2][2];
    c[2] = M[2][0] * M[3][3] - M[3][0] * M[2][3];
    c[3] = M[2][1] * M[3][2] - M[3][1] * M[2][2];
    c[4] = M[2][1] * M[3][3] - M[3][1] * M[2][3];
    c[5] = M[2][2] * M[3][3] - M[3][2] * M[2][3];

    /* Assumes it is invertible */
    float idet = 1.0f / (s[0] * c[5] - s[1] * c[4] + s[2] * c[3] + s[3] * c[2] - s[4] * c[1] + s[5] * c[0]);

    res[0][0] = (M[1][1] * c[5] - M[1][2] * c[4] + M[1][3] * c[3]) * idet;
    res[0][1] = (-M[0][1] * c[5] + M[0][2] * c[4] - M[0][3] * c[3]) * idet;
    res[0][2] = (M[3][1] * s[5] - M[3][2] * s[4] + M[3][3] * s[3]) * idet;
    res[0][3] = (-M[2][1] * s[5] + M[2][2] * s[4] - M[2][3] * s[3]) * idet;

    res[1][0] = (-M[1][0] * c[5] + M[1][2] * c[2] - M[1][3] * c[1]) * idet;
    res[1][1] = (M[0][0] * c[5] - M[0][2] * c[2] + M[0][3] * c[1]) * idet;
    res[1][2] = (-M[3][0] * s[5] + M[3][2] * s[2] - M[3][3] * s[1]) * idet;
    res[1][3] = (M[2][0] * s[5] - M[2][2] * s[2] + M[2][3] * s[1]) * idet;

    res[2][0] = (M[1][0] * c[4] - M[1][1] * c[2] + M[1][3] * c[0]) * idet;
    res[2][1] = (-M[0][0] * c[4] + M[0][1] * c[2] - M[0][3] * c[0]) * idet;
    res[2][2] = (M[3][0] * s[4] - M[3][1] * s[2] + M[3][3] * s[0]) * idet;
    res[2][3] = (-M[2][0] * s[4] + M[2][1] * s[2] - M[2][3] * s[0]) * idet;

    res[3][0] = (-M[1][0] * c[3] + M[1][1] * c[1] - M[1][2] * c[0]) * idet;
    res[3][1] = (M[0][0] * c[3] - M[0][1] * c[1] + M[0][2] * c[0]) * idet;
    res[3][2] = (-M[3][0] * s[3] + M[3][1] * s[1] - M[3][2] * s[0]) * idet;
    res[3][3] = (M[2][0] * s[3] - M[2][1] * s[1] + M[2][2] * s[0]) * idet;
}

static inline void mat4x4_orthonormalize(mat4x4_type res, const mat4x4_type M) {
    mat4x4_copy(res, M);
    float s = 1.f;
    vec3_type h;

    vec3_norm(res[2], res[2]);

    s = vec3_dot(res[1], res[2]);
    vec3_scale(h, res[2], s);
    vec3_sub(res[1], res[1], h);
    vec3_norm(res[1], res[1]);

    s = vec3_dot(res[0], res[2]);
    vec3_scale(h, res[2], s);
    vec3_sub(res[0], res[0], h);

    s = vec3_dot(res[0], res[1]);
    vec3_scale(h, res[1], s);
    vec3_sub(res[0], res[0], h);
    vec3_norm(res[0], res[0]);
}

static inline void mat4x4_frustum(mat4x4_type res, const float l, const float r, const float b, const float t, const float n,
                                  const float f) {
    res[0][0] = 2.f * n / (r - l);
    res[0][1] = res[0][2] = res[0][3] = 0.f;

    res[1][1] = 2.f * n / (t - b);
    res[1][0] = res[1][2] = res[1][3] = 0.f;

    res[2][0] = (r + l) / (r - l);
    res[2][1] = (t + b) / (t - b);
    res[2][2] = -(f + n) / (f - n);
    res[2][3] = -1.f;

    res[3][2] = -2.f * (f * n) / (f - n);
    res[3][0] = res[3][1] = res[3][3] = 0.f;
}

static inline void mat4x4_ortho(mat4x4_type res, const float l, const float r, const float b, const float t, const float n,
                                const float f) {
    res[0][0] = 2.f / (r - l);
    res[0][1] = res[0][2] = res[0][3] = 0.f;

    res[1][1] = 2.f / (t - b);
    res[1][0] = res[1][2] = res[1][3] = 0.f;

    res[2][2] = -2.f / (f - n);
    res[2][0] = res[2][1] = res[2][3] = 0.f;

    res[3][0] = -(r + l) / (r - l);
    res[3][1] = -(t + b) / (t - b);
    res[3][2] = -(f + n) / (f - n);
    res[3][3] = 1.f;
}

static inline void mat4x4_perspective(mat4x4_type m, const float y_fov, const float aspect, const float n, const float f) {
    /* NOTE: Degrees are an unhandy unit to work with.
     * linmath.h uses radians for everything! */
    float const a = 1.f / tanf(y_fov / 2.f);

    m[0][0] = a / aspect;
    m[0][1] = 0.f;
    m[0][2] = 0.f;
    m[0][3] = 0.f;

    m[1][0] = 0.f;
    m[1][1] = a;
    m[1][2] = 0.f;
    m[1][3] = 0.f;

    m[2][0] = 0.f;
    m[2][1] = 0.f;
    m[2][2] = -((f + n) / (f - n));
    m[2][3] = -1.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = -((2.f * f * n) / (f - n));
    m[3][3] = 0.f;
}

static inline void mat4x4_look_at(mat4x4_type m, const vec3_type eye, const vec3_type center, const vec3_type up) {
    /* Adapted from Android's OpenGL Matrix.java.                        */
    /* See the OpenGL GLUT documentation for gluLookAt for a description */
    /* of the algorithm. We implement it in a straightforward way:       */

    /* TODO: The negation of of can be spared by swapping the order of
     *       operands in the following cross products in the right way. */
    vec3_type f;
    vec3_sub(f, center, eye);
    vec3_norm(f, f);

    vec3_type s;
    vec3_cross(s, f, up);
    vec3_norm(s, s);

    vec3_type t;
    vec3_cross(t, s, f);

    m[0][0] = s[0];
    m[0][1] = t[0];
    m[0][2] = -f[0];
    m[0][3] = 0.f;

    m[1][0] = s[1];
    m[1][1] = t[1];
    m[1][2] = -f[1];
    m[1][3] = 0.f;

    m[2][0] = s[2];
    m[2][1] = t[2];
    m[2][2] = -f[2];
    m[2][3] = 0.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = 0.f;
    m[3][3] = 1.f;

    mat4x4_translate_in_place(m, -eye[0], -eye[1], -eye[2]);
}

static inline void mat4x4_arcball(mat4x4_type res, const mat4x4_type M, const vec2_type _a, const vec2_type _b, const float s) {
    vec2_type a;
    memcpy(a, _a, sizeof(a));
    vec2_type b;
    memcpy(b, _b, sizeof(b));

    float z_a = 0.;
    float z_b = 0.;

    if (vec2_length(a) < 1.f) {
        z_a = sqrtf(1.f - vec2_dot(a, a));
    } else {
        vec2_norm(a, a);
    }

    if (vec2_length(b) < 1.f) {
        z_b = sqrtf(1.f - vec2_dot(b, b));
    } else {
        vec2_norm(b, b);
    }

    vec3_type a_ = {a[0], a[1], z_a};
    vec3_type b_ = {b[0], b[1], z_b};

    vec3_type c_;
    vec3_cross(c_, a_, b_);

    float const angle_rad = acosf(vec3_dot(a_, b_)) * s;
    mat4x4_rotate(res, M, c_[0], c_[1], c_[2], angle_rad);
}

static inline void mat4x4_extract_planes_from_projmat(const mat4x4_type mvp, vec4_type left, vec4_type right, vec4_type bottom,
                                                      vec4_type top, vec4_type near, vec4_type far) {
    // Gribb/Hartmann method:
    // https://stackoverflow.com/a/34960913

    for (int i = 4; i--;) {
        left[i] = mvp[i][3] + mvp[i][0];
    }
    for (int i = 4; i--;) {
        right[i] = mvp[i][3] - mvp[i][0];
    }
    for (int i = 4; i--;) {
        bottom[i] = mvp[i][3] + mvp[i][1];
    }
    for (int i = 4; i--;) {
        top[i] = mvp[i][3] - mvp[i][1];
    }
    for (int i = 4; i--;) {
        near[i] = mvp[i][3] + mvp[i][2];
    }
    for (int i = 4; i--;) {
        far[i] = mvp[i][3] - mvp[i][2];
    }
}
