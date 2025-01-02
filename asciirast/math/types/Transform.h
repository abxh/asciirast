/**
 * @file Transform.h
 * @brief Class for stacking primitive transformations
 *
 * Using OpenGL conventions for the coordinate systems. Just multiply
 * by the UP / RIGHT / FORWARD basis vectors to be coordinate system agnostic.
 */

#pragma once

#include <cassert>
#include "Mat.h"

namespace asciirast::math {

using Mat2x2f = Mat<2, 2, float>;
using Mat3x3f = Mat<3, 3, float>;
using Mat4x4f = Mat<4, 4, float>;

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

/**
 * @brief 2D transformation class
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
    constexpr Transform2D() : mat{Mat3x3f::identity()} {}

    /**
     * Apply to 2D Vector
     */
    Vec2f apply(const Vec2f& v) const {
        auto res = mat * Vec3f{v, 1.f};

        return res.xy / res.z;
    }

    /**
     * Stack a new transformation matrix on top
     */
    constexpr Transform2D stack(const Mat3x3f& that) {
        mat = that * mat;
        return *this;
    }

    /**
     * Stack another Transform2D on top of this
     */
    constexpr Transform2D stack(const Transform2D& that) {
        return this->stack(that.mat);
    }

    /**
     * Translate by (delta_x, delta_y)
     */
    constexpr Transform2D translate(float delta_x, float delta_y) {
        auto v = Vec3f{delta_x, delta_y, 1.f};
        auto res = Mat3x3f::identity().column_set(2, v);

        return this->stack(res);
    }

    /**
     * Translate by (-delta_x, -delta_y)
     */
    constexpr Transform2D translate_inv(float delta_x, float delta_y) {
        return translate(-delta_x, -delta_y);
    }

    /**
     * Rotate such that the `right` vector becomes the new right basis vector.
     */
    constexpr Transform2D rotate(Vec2f right, bool is_normalized = false) {
        if (!is_normalized) {
            right = right.normalized();
        }
        auto up = Vec2f{-right.y, right.x};  // 90 degrees counterclockwise
        auto res = Mat2x2f::from_columns(right, up) *
                   Mat2x2f::from_columns(RIGHT, UP).transposed();  // inverse

        // TODO: check if math works out

        return this->stack(Mat3x3f{res});
    }

    /**
     * Rotate counterclockwise by `angle_in_radians` with respect to the screen
     */
    constexpr Transform2D rotate_counterclockwise(float angle_in_radians) {
        return rotate(std::cos(angle_in_radians) * RIGHT +
                              std::sin(angle_in_radians) * UP,
                      true);
    }

    /**
     * Rotate clockwise by `angle_in_radians` with respect to the screen
     */
    constexpr Transform2D rotate_clockwise(float angle_in_radians) {
        return rotate_clockwise(-angle_in_radians);
    }

    /**
     * Scale by (scale_x, scale_y) in x- and y-axis.
     */
    constexpr Transform2D scale(float scale_x, float scale_y) {
        Mat3x3f res{};
        res(0, 0) = scale_x;
        res(1, 1) = scale_y;
        res(2, 2) = 1.f;

        return this->stack(res);
    }

    /**
     * Scale by (1.f / scale_x, 1.f / scale_y) in x- and y-axis.
     */
    constexpr Transform2D scale_inv(float scale_x, float scale_y) {
        assert(scale_x != 0.f);
        assert(scale_y != 0.f);
        return scale(1.f / scale_x, 1.f / scale_y);
    }

    /**
     * Sheer x-axis by `sh_x`
     */
    constexpr Transform2D shearX(float sh_x) {
        auto a = Vec3f{1.f, sh_x, 0.f};
        auto b = Vec3f{0.f, 1.0f, 0.f};
        auto c = Vec3f{0.f, 0.0f, 1.f};
        auto res = Mat3x3f::from_rows(a, b, c);

        return this->stack(res);
    }

    /**
     * Sheer x-axis by `-sh_x`
     */
    constexpr Transform2D shearX_inv(float sh_x) { return shearX(-sh_x); }

    /**
     * Sheer y-axis by `sh_y`
     */
    constexpr Transform2D shearY(float sh_y) {
        auto a = Vec3f{1.0f, 0.f, 0.f};
        auto b = Vec3f{sh_y, 1.f, 0.f};
        auto c = Vec3f{0.0f, 0.f, 1.f};
        auto res = Mat3x3f::from_rows(a, b, c);

        return this->stack(res);
    }

    /**
     * Sheer y-axis by `-sh_y`
     */
    constexpr Transform2D shearY_inv(float sh_y) { return shearY(-sh_y); }
};

/**
 * @brief 3D transformation class
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
    constexpr Transform3D() : mat{Mat4x4f::identity()} {}

    /**
     * Apply to 3D Vector
     */
    Vec3f apply(const Vec3f& v) const {
        auto res = mat * Vec4f{v, 1.f};

        return res.xyz / res.w;
    }

    /**
     * Stack a new transformation matrix on top
     */
    constexpr Transform3D stack(const Mat4x4f& that) {
        mat = that * mat;
        return *this;
    }

    /**
     * Stack another Transform3D on top of this
     */
    constexpr Transform3D stack(const Transform3D& that) {
        return this->stack(that.mat);
    }

    /**
     * Translate by (delta_x, delta_y, delta_z)
     */
    constexpr Transform3D translate(float delta_x,
                                    float delta_y,
                                    float delta_z) {
        auto v = Vec<4, float>{delta_x, delta_y, delta_z, 1.0f};
        auto res = Mat4x4f::identity().column_set(3, v);

        return this->stack(res);
    }

    /**
     * Translate by (-delta_x, -delta_y, -delta_z)
     */
    constexpr Transform3D translate_inv(float delta_x,
                                        float delta_y,
                                        float delta_z) {
        return translate(-delta_x, -delta_y, -delta_z);
    }

    /**
     * Rotate such that the given vectors becomes the new basis vectors
     */
    constexpr Transform3D rotate(Vec3f right,
                                 Vec3f up,
                                 Vec3f forward,
                                 bool is_normalized = false) {
        if (!is_normalized) {
            right = right.normalized();
            up = up.normalized();
            forward = forward.normalized();
            // static_assert(right, up, forward) is linearly independent
        }
        auto res = Mat3x3f::from_rows(right, up, forward) *
                   Mat3x3f::from_rows(RIGHT, UP, FORWARD).transposed();

        // TODO: check if math works out

        return this->stack(Mat4x4f{res});
    }
};

}  // namespace asciirast::math
