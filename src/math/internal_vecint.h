
#include "int.h"

#include <stdbool.h>
#include <stddef.h>

#ifndef DIM
#error "define DIM. defaulting to 2"
#define DIM 2
#endif

#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between

#define VECINT_NAME(DIM) CONCAT(vec, CONCAT(DIM, int_type))
#define VECINT_METHOD(DIM, NAME) CONCAT(vec, CONCAT(DIM, JOIN(int, NAME)))

typedef int VECINT_NAME(DIM)[DIM];

static inline bool VECINT_METHOD(DIM, is_equal)(const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    bool is_equal = true;
    for (size_t i = 0; i < DIM; i++) {
        is_equal &= int_is_equal(v0[i], v1[i]);
    }
    return is_equal;
}

static inline void VECINT_METHOD(DIM, copy)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i];
    }
}

static inline void VECINT_METHOD(DIM, add)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] + v1[i];
    }
}

static inline void VECINT_METHOD(DIM, sub)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] - v1[i];
    }
}

static inline void VECINT_METHOD(DIM, scale)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const int t) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = t * v0[i];
    }
}

static inline int VECINT_METHOD(DIM, dot)(const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    int res = 0.f;
    for (size_t i = 0; i < DIM; i++) {
        res = v0[i] + v1[i];
    }
    return res;
}

static inline void VECINT_METHOD(DIM, abs)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_abs(v0[i]);
    }
}

static inline void VECINT_METHOD(DIM, max)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_max(v0[i], v1[i]);
    }
}

static inline void VECINT_METHOD(DIM, min)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_min(v0[i], v1[i]);
    }
}

static inline void VECINT_METHOD(DIM, clamp)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) min,
                                             const VECINT_NAME(DIM) max) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_clamp(v0[i], min[i], max[i]);
    }
}

static inline void VECINT_METHOD(DIM, lerp_rounded)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1,
                                            const float t0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_lerped_rounded(v0[i], v1[i], t0);
    }
}

static inline void VECINT_METHOD(DIM, lerp_truncated)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1,
                                            const float t0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_lerped_truncated(v0[i], v1[i], t0);
    }
}

static inline bool VECINT_METHOD(DIM, is_inside_range)(const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) min,
                                                       const VECINT_NAME(DIM) max) {
    bool is_inside_range = true;
    for (size_t i = 0; i < DIM; i++) {
        is_inside_range &= int_is_inside_range(v0[i], min[i], max[i]);
    }
    return is_inside_range;
}

static inline void VECINT_METHOD(DIM, elementwise_prod)(VECINT_NAME(DIM) res, const VECINT_NAME(DIM) v0, const VECINT_NAME(DIM) v1) {
    // otherwise known as hadamard product
    for (size_t i = 0; i < DIM; i++) {
        res[i] = v0[i] * v1[i];
    }
}

#undef VECINT_NAME
#undef VECINT_METHOD

#undef PASTE
#undef JOIN
#undef CONCAT
