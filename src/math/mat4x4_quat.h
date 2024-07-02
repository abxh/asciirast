// taken from - and further modified:
// https://github.com/datenwolf/linmath.h/blob/master/linmath.h
//
// Under public domain.
//
// May calculate inplace operations incorrectly. 

#include "quat.h"
#include "vec.h"

typedef vec4_type mat4x4_type[4];
typedef float quat_type[4];

static inline void mat4x4o_mul_quat(mat4x4_type res, const mat4x4_type M, const quat_type q) {
    /*  XXX: The way this is written only works for orthogonal matrices. */
    /* TODO: Take care of non-orthogonal case. */
    quat_mul_vec3(res[0], q, M[0]);
    quat_mul_vec3(res[1], q, M[1]);
    quat_mul_vec3(res[2], q, M[2]);

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
