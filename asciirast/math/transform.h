/**
 * @file transform.h
 * @brief Basic linear transformations in homogeneous space.
 */

#pragma once

#include <cmath>
#include <stdexcept>

#include "types.h"

namespace asciirast::math {

static inline Mat3x3f translation(float delta_x, float delta_y) {
    const auto v = Vec<3, float>{delta_x, delta_y, 1.0f};

    if (Mat3x3f::is_column_major) {
        return Mat3x3f::identity().column_set(2, v);
    } else {
        return Mat3x3f::identity().row_set(2, v);
    }
}

static inline Mat3x3f translation_inv(float delta_x, float delta_y) {
    return translation(-delta_x, -delta_y);
}

static inline Mat4x4f translation(float delta_x, float delta_y, float delta_z) {
    const auto v = Vec<4, float>{delta_x, delta_y, delta_z, 1.0f};

    if (Mat4x4f::is_column_major) {
        return Mat4x4f::identity().column_set(3, v);
    } else {
        return Mat4x4f::identity().row_set(3, v);
    }
}

static inline Mat3x3f scaling(float scale_x, float scale_y) {
    Mat3x3f res{};
    res(0, 0) = scale_x;
    res(1, 1) = scale_y;
    res(2, 2) = 1.0f;
    return res;
}

static inline Mat3x3f scaling_inv(float scale_x, float scale_y) {
    if (scale_x == 0.f || scale_y == 0.f) {
        throw std::overflow_error("asciirast::math::mat3_scaling_inv()");
    }
    return scaling(1.f / scale_x, 1.f / scale_y);
}

static inline Mat4x4f scaling(float scale_x, float scale_y, float scale_z) {
    Mat4x4f res{};
    res(0, 0) = scale_x;
    res(1, 1) = scale_y;
    res(2, 2) = scale_z;
    res(3, 3) = 1.f;
    return res;
}

static inline Mat4x4f scaling_inv(float scale_x, float scale_y, float scale_z) {
    if (scale_x == 0.f || scale_y == 0.f || scale_z == 0.f) {
        throw std::overflow_error("asciirast::math::mat3_scaling_inv()");
    }
    return scaling(1.f / scale_x, 1.f / scale_y, 1.f / scale_z);
}

static inline Mat3x3f rotation(float angle) {
    auto cos_ = std::cos(angle);
    auto sin_ = std::sin(angle);

    auto a = Vec3f{+cos_, -sin_, 0.f};
    auto b = Vec3f{+sin_, +cos_, 0.f};
    auto c = Vec3f{0.00f, 0.00f, 1.f};

    return Mat3x3f::from_rows(a, b, c);
}

static inline Mat3x3f rotation_inv(float angle) {
    return rotation(-angle);
}

static inline Mat3x3f shearX(float sh_x) {
    auto a = Vec3f{1.f, sh_x, 0.f};
    auto b = Vec3f{0.f, 1.0f, 0.f};
    auto c = Vec3f{0.f, 0.0f, 1.f};

    return Mat3x3f::from_rows(a, b, c);
}

static inline Mat3x3f shearX_inv(float sh_x) {
    return shearX(-sh_x);
}

static inline Mat3x3f shearY(float sh_y) {
    auto a = Vec3f{1.0f, 0.f, 0.f};
    auto b = Vec3f{sh_y, 1.f, 0.f};
    auto c = Vec3f{0.0f, 0.f, 1.f};

    return Mat3x3f::from_rows(a, b, c);
}

static inline Mat3x3f shearY_inv(float sh_y) {
    return shearY(-sh_y);
}

static inline Mat4x4f shearXY(float sh_x, float sh_y) {
    auto a = Vec4f{1.f, 0.f, sh_x, 0.f};
    auto b = Vec4f{0.f, 1.f, sh_y, 0.f};
    auto c = Vec4f{0.f, 0.f, 1.0f, 0.f};
    auto d = Vec4f{0.f, 0.f, 0.0f, 1.f};

    return Mat4x4f::from_rows(a, b, c);
}

static inline Mat4x4f shearXY_inv(float sh_x, float sh_y) {
    return shearXY_inv(-sh_x, -sh_y);
}

static inline Mat4x4f shearXZ(float sh_x, float sh_z) {
    auto a = Vec4f{1.f, sh_x, 0.f, 0.f};
    auto b = Vec4f{0.f, 1.0f, 0.f, 0.f};
    auto c = Vec4f{0.f, sh_z, 1.f, 0.f};
    auto d = Vec4f{0.f, 0.0f, 0.f, 1.f};

    return Mat4x4f::from_rows(a, b, c);
}

static inline Mat4x4f shearXZ_inv(float sh_x, float sh_z) {
    return shearXZ_inv(-sh_x, -sh_z);
}

static inline Mat4x4f shearYZ(float sh_y, float sh_z) {
    auto a = Vec4f{1.0f, 0.f, 0.f, 0.f};
    auto b = Vec4f{sh_y, 1.f, 0.f, 0.f};
    auto c = Vec4f{sh_z, 0.f, 1.f, 0.f};
    auto d = Vec4f{0.0f, 0.f, 0.f, 1.f};

    return Mat4x4f::from_rows(a, b, c);
}

static inline Mat4x4f shearYZ_inv(float sh_y, float sh_z) {
    return shearYZ(-sh_y, -sh_z);
}

}  // namespace asciirast::math::transform
