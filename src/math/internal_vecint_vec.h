#include "float.h"
#include "int.h"
#include "stddef.h"

#ifndef DIM
#error "define DIM. defaulting to 2"
#define DIM 2
#endif

#define PASTE(a, b) a##b                   // Paste two tokens together
#define CONCAT(a, b) PASTE(a, b)           // First expand, then paste two tokens together
#define JOIN(a, b) CONCAT(a, CONCAT(_, b)) // First expand, then paste two tokens toegher with a _ in between

#define VEC_NAME(DIM) CONCAT(vec, JOIN(DIM, type))
#define VEC_METHOD(DIM, NAME) CONCAT(vec, JOIN(DIM, NAME))
#define VECINT_NAME(DIM) CONCAT(vec, CONCAT(DIM, int_type))
#define VECINT_METHOD(DIM, NAME) CONCAT(vec, CONCAT(DIM, JOIN(int, NAME)))

typedef float VEC_NAME(DIM)[DIM];
typedef int VECINT_NAME(DIM)[DIM];

static inline void VECINT_METHOD(DIM, CONCAT(to_vec, DIM))(VEC_NAME(DIM) res, const VECINT_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = int_to_float(v0[i]);
    }
}

static inline void VEC_METHOD(DIM, CONCAT(truncated_to_vec, CONCAT(DIM, int)))(VECINT_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_truncated_to_int(v0[i]);
    }
}

static inline void VEC_METHOD(DIM, CONCAT(rounded_to_vec, CONCAT(DIM, int)))(VECINT_NAME(DIM) res, const VEC_NAME(DIM) v0) {
    for (size_t i = 0; i < DIM; i++) {
        res[i] = float_rounded_to_int(v0[i]);
    }
}

#undef PASTE
#undef CONCAT
#undef JOIN
#undef VEC_NAME
#undef VEC_METHOD
#undef VECINT_NAME
#undef VECINT_METHOD
