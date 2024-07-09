// taken from - and further modified:
// https://github.com/datenwolf/linmath.h/blob/master/linmath.h
//
// Originally under public domain.
//
// Column-major matrix
//
// Modifications may have introduced errors. The modifications are not tested. Use the original if in doubt.

#pragma once

#include "math/quat.h"
#include "math/vec.h"

typedef float mat4x4_type[4][4];
typedef float quat_type[4];

static inline void mat4x4_identity(mat4x4_type res) {
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            res[i][j] = i == j ? 1.f : 0.f;
        }
    }
}

static inline void mat4x4_copy(mat4x4_type res, const mat4x4_type N) {
    for (size_t i = 0; i < 4; ++i) {
        vec4_to_array(res[i], vec4_copy(vec4_from_array(N[i])));
    }
}

static inline vec4_type mat4x4_row(const mat4x4_type M, const size_t i) {
    vec4_type res;
    for (size_t k = 0; k < 4; ++k) {
        res.array[k] = M[k][i];
    }
    return res;
}

static inline vec4_type mat4x4_col(const mat4x4_type M, const size_t i) {
    vec4_type res;
    for (size_t k = 0; k < 4; ++k) {
        res.array[k] = M[i][k];
    }
    return res;
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
        vec4_to_array(res[i], vec4_add(vec4_from_array(a[i]), vec4_from_array(b[i])));
    }
}

static inline void mat4x4_sub(mat4x4_type res, const mat4x4_type a, const mat4x4_type b) {
    for (size_t i = 0; i < 4; ++i) {
        vec4_to_array(res[i], vec4_sub(vec4_from_array(a[i]), vec4_from_array(b[i])));
    }
}

static inline void mat4x4_scale(mat4x4_type res, const mat4x4_type a, const float k) {
    for (size_t i = 0; i < 4; ++i)
        vec4_to_array(res[i], vec4_scale(vec4_from_array(a[i]), k));
}

static inline void mat4x4_scale_aniso(mat4x4_type res, const mat4x4_type a, const float x, const float y, const float z) {
    vec4_to_array(res[0], vec4_scale(vec4_from_array(a[0]), x));
    vec4_to_array(res[1], vec4_scale(vec4_from_array(a[1]), y));
    vec4_to_array(res[1], vec4_scale(vec4_from_array(a[2]), z));
    vec4_to_array(res[3], vec4_from_array(a[3]));
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
        res.array[j] = 0.f;
        for (size_t i = 0; i < 4; ++i) {
            res.array[j] += M[i][j] * v.array[i];
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
    const vec4_type t = {.array = {x, y, z, 0}};
    vec4_type r;
    for (size_t i = 0; i < 4; ++i) {
        r = mat4x4_row(res, i);
        res[3][i] += vec4_dot(r, t);
    }
}

static inline void mat4x4_from_vec3_mul_outer(mat4x4_type res, const vec3_type a, const vec3_type b) {
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = 0; j < 4; ++j) {
            res[i][j] = i < 3 && j < 3 ? a.array[i] * b.array[j] : 0.f;
        }
    }
}

static inline void mat4x4_rotate(mat4x4_type res, const mat4x4_type M, const float x, const float y, const float z,
                                 const float angle_rad) {
    float s = sinf(angle_rad);
    float c = cosf(angle_rad);
    vec3_type u = {.array = {x, y, z}};

    if (vec3_length(u) > (float)(1e-4)) {
        u = vec3_norm(u);
        mat4x4_type T;
        mat4x4_from_vec3_mul_outer(T, u, u);

        mat4x4_type S = {
            {0, u.array[2], -u.array[1], 0}, {-u.array[2], 0, u.array[0], 0}, {u.array[1], -u.array[0], 0, 0}, {0, 0, 0, 0}};
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

    vec3_to_array(res[2], vec3_norm(vec3_from_array(res[2])));

    s = vec3_dot(vec3_from_array(res[1]), vec3_from_array(res[2]));
    h = vec3_scale(vec3_from_array(res[2]), s);
    vec3_to_array(res[1], vec3_norm(vec3_sub(vec3_from_array(res[1]), h)));

    s = vec3_dot(vec3_from_array(res[0]), vec3_from_array(res[2]));
    h = vec3_scale(vec3_from_array(res[2]), s);
    vec3_to_array(res[0], vec3_sub(vec3_from_array(res[0]), h));

    s = vec3_dot(vec3_from_array(res[0]), vec3_from_array(res[1]));
    h = vec3_scale(vec3_from_array(res[1]), s);
    vec3_to_array(res[0], vec3_norm(vec3_sub(vec3_from_array(res[0]), h)));
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

    vec3_type f = vec3_norm(vec3_sub(center, eye));
    vec3_type s = vec3_norm(vec3_cross(f, up));
    vec3_type t = vec3_cross(s, f);

    m[0][0] = s.array[0];
    m[0][1] = t.array[0];
    m[0][2] = -f.array[0];
    m[0][3] = 0.f;

    m[1][0] = s.array[1];
    m[1][1] = t.array[1];
    m[1][2] = -f.array[1];
    m[1][3] = 0.f;

    m[2][0] = s.array[2];
    m[2][1] = t.array[2];
    m[2][2] = -f.array[2];
    m[2][3] = 0.f;

    m[3][0] = 0.f;
    m[3][1] = 0.f;
    m[3][2] = 0.f;
    m[3][3] = 1.f;

    mat4x4_translate_in_place(m, -eye.array[0], -eye.array[1], -eye.array[2]);
}

static inline void mat4x4_arcball(mat4x4_type res, const mat4x4_type M, const vec2_type _a, const vec2_type _b, const float s) {
    vec2_type a = _a;
    vec2_type b = _b;

    float z_a = 0.;
    float z_b = 0.;

    if (vec2_length(a) < 1.f) {
        z_a = sqrtf(1.f - vec2_dot(a, a));
    } else {
        a = vec2_norm(a);
    }

    if (vec2_length(b) < 1.f) {
        z_b = sqrtf(1.f - vec2_dot(b, b));
    } else {
        b = vec2_norm(b);
    }

    vec3_type a_ = {.array = {a.array[0], a.array[1], z_a}};
    vec3_type b_ = {.array = {b.array[0], b.array[1], z_b}};

    vec3_type c_ = vec3_cross(a_, b_);

    float const angle_rad = acosf(vec3_dot(a_, b_)) * s;
    mat4x4_rotate(res, M, c_.array[0], c_.array[1], c_.array[2], angle_rad);
}

static inline void mat4x4o_mul_quat(mat4x4_type res, const mat4x4_type M, const quat_type q) {
    /*  XXX: The way this is written only works for orthogonal matrices. */
    /* TODO: Take care of non-orthogonal case. */
    quat_mul_vec3_alt(res[0], q, M[0]);
    quat_mul_vec3_alt(res[1], q, M[1]);
    quat_mul_vec3_alt(res[2], q, M[2]);

    res[3][0] = res[3][1] = res[3][2] = 0.f;
    res[0][3] = M[0][3];
    res[1][3] = M[1][3];
    res[2][3] = M[2][3];
    res[3][3] = M[3][3]; // typically 1.0, but here we make it general
}

static inline void mat4x4_from_quat(mat4x4_type res, const quat_type q) {
    float a = q[3];
    float b = q[0];
    float c = q[1];
    float d = q[2];
    float a2 = a * a;
    float b2 = b * b;
    float c2 = c * c;
    float d2 = d * d;

    res[0][0] = a2 + b2 - c2 - d2;
    res[0][1] = 2.f * (b * c + a * d);
    res[0][2] = 2.f * (b * d - a * c);
    res[0][3] = 0.f;

    res[1][0] = 2 * (b * c - a * d);
    res[1][1] = a2 - b2 + c2 - d2;
    res[1][2] = 2.f * (c * d + a * b);
    res[1][3] = 0.f;

    res[2][0] = 2.f * (b * d + a * c);
    res[2][1] = 2.f * (c * d - a * b);
    res[2][2] = a2 - b2 - c2 + d2;
    res[2][3] = 0.f;

    res[3][0] = res[3][1] = res[3][2] = 0.f;
    res[3][3] = 1.f;
}
