/**
 * @file Transform.h
 * @brief Class for stacking primitive linear transformations
 */

#pragma once

#include "Mat.h"

namespace asciirast::math {

using Mat3x3f = Mat<3, 3, float>;
using Mat4x4f = Mat<4, 4, float>;

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

/**
 * @brief 2D (linear) transformation class
 */
class Transform2D {
public:
    static inline const Vec2f RIGHT = Vec2f{1.f, 0.f};
    static inline const Vec2f UP = Vec2f{0.f, -1.f};

public:
    Mat3x3f mat;  ///< underlying matrix

    /**
     * Create a new transform object
     */
    Transform2D() : mat{Mat3x3f::identity()} {}

    /**
     * Apply to 2D Vector
     */
    Vec2f apply(const Vec2f& v) {
        auto res = mat * Vec3f{v, 1.f};

        return res.xy / res.z;
    }

    /**
     * Translate by (delta_x, delta_y)
     */
    Transform2D translate(float delta_x, float delta_y) {
        auto v = Vec3f{delta_x, delta_y, 1.f};
        auto res = Mat3x3f::identity().column_set(2, v);

        mat = res * mat;
        return *this;
    }

    /**
     * Translate by (-delta_x, -delta_y)
     */
    Transform2D translate_inv(float delta_x, float delta_y) {
        return translate(-delta_x, -delta_y);
    }

    /**
     * Rotate such that the `right` vector becomes the right basis vector.
     */
    Transform2D rotate(Vec2f right, bool is_normalized = false) {
        if (!is_normalized) {
            right = right.normalized();
        }

        auto a = Vec3f{right.x, -right.y, 0.f};
        auto b = Vec3f{right.y, +right.x, 0.f};
        auto c = Vec3f{0.0000f, 0.00000f, 1.f};
        auto res = Mat3x3f::from_rows(a, b, c);

        mat = res * mat;
        return *this;
    }

    /**
     * Rotate counterclockwise by `angle_in_radians`
     */
    Transform2D rotate(float angle_in_radians) {
        return rotate(Vec2f{std::cos(angle_in_radians) * Transform2D::RIGHT +
                            std::sin(angle_in_radians) * Transform2D::UP},
                      true);
    }

    /**
     * Rotate clockwise by `angle_in_radians`
     */
    Transform2D rotate_inv(float angle_in_radins) {
        return rotate_inv(-angle_in_radins);
    }

    /**
     * Scale by (scale_x, scale_y) in x- and y-axis.
     */
    Transform2D scale(float scale_x, float scale_y) {
        Mat3x3f res{};
        res(0, 0) = scale_x;
        res(1, 1) = scale_y;
        res(2, 2) = 1.f;

        mat = res * mat;
        return *this;
    }

    /**
     * Scale by (1.f / scale_x, 1.f / scale_y) in x- and y-axis.
     */
    Transform2D scale_inv(float scale_x, float scale_y) {
        return scale(1.f / scale_x, 1.f / scale_y);
    }

    /**
     * Sheer x-axis by `sh_x`
     */
    Transform2D shearX(float sh_x) {
        auto a = Vec3f{1.f, sh_x, 0.f};
        auto b = Vec3f{0.f, 1.0f, 0.f};
        auto c = Vec3f{0.f, 0.0f, 1.f};
        auto res = Mat3x3f::from_rows(a, b, c);

        mat = res * mat;
        return *this;
    }

    /**
     * Sheer x-axis by `-sh_x`
     */
    Transform2D shearX_inv(float sh_x) { return shearX(-sh_x); }

    /**
     * Sheer y-axis by `sh_y`
     */
    Transform2D shearY(float sh_y) {
        auto a = Vec3f{1.0f, 0.f, 0.f};
        auto b = Vec3f{sh_y, 1.f, 0.f};
        auto c = Vec3f{0.0f, 0.f, 1.f};
        auto res = Mat3x3f::from_rows(a, b, c);

        mat = res * mat;
        return *this;
    }

    /**
     * Sheer y-axis by `-sh_y`
     */
    Transform2D shearY_inv(float sh_y) { return shearY(-sh_y); }
};

/**
 * @brief 3D (linear) transformation class
 */
class Transform3D {
public:
    static inline const Vec3f RIGHT = Vec3f{1.f, 0.f, 0.f};
    static inline const Vec3f UP = Vec3f{0.f, 1.f, 0.f};
    static inline const Vec3f FORWARD = Vec3f{0.f, 0.f, -1.f};

public:
    Mat4x4f mat;  ///< underlying matrix

    /**
     * Create a new transform object
     */
    Transform3D() : mat{Mat4x4f::identity()} {}

    /**
     * Apply to 3D Vector
     */
    Vec3f apply(const Vec3f& v) {
        auto res = mat * Vec4f{v, 1.f};

        return res.xyz / res.w;
    }
};

}  // namespace asciirast::math
