#include "Transform.h"

namespace asciirast::math {

template class Mat<2, 2, float, true>;
template class Mat<3, 3, float, true>;
template class Mat<4, 4, float, true>;

template class Vec<2, float>;
template class Vec<3, float>;
template class Vec<4, float>;

Transform2D::Transform2D()
        : m_mat{Mat3x3f::identity()}, m_mat_inv{Mat3x3f::identity()} {}

Transform3D::Transform3D()
        : m_mat{Mat4x4f::identity()}, m_mat_inv{Mat4x4f::identity()} {}

Vec2f Transform2D::apply(const Vec2f& v) const {
    const auto v0 = Vec3f{v, 1.f};
    const auto v1 = m_mat * v0;
    const auto vr = v1.xy.vec();
    return vr;
}

Vec3f Transform3D::apply(const Vec3f& v) const {
    const auto v0 = Vec4f{v, 1.f};
    const auto v1 = m_mat * v0;
    const auto vr = v1.xyz.vec();
    return vr;
}

Vec2f Transform2D::invert(const Vec2f& v) const {
    const auto v0 = Vec3f{v, 1.f};
    const auto v1 = m_mat_inv * v0;
    const auto vr = v1.xy.vec();
    return vr;
}

Vec3f Transform3D::invert(const Vec3f& v) const {
    const auto v0 = Vec4f{v, 1.f};
    const auto v1 = m_mat_inv * v0;
    const auto vr = v1.xyz.vec();
    return vr;
}

Transform2D Transform2D::stack(const Mat3x3f& mat, const Mat3x3f& inv_mat) {
    m_mat = mat * m_mat;
    m_mat_inv = m_mat_inv * inv_mat;
    return *this;
}

Transform3D Transform3D::stack(const Mat4x4f& mat, const Mat4x4f& inv_mat) {
    m_mat = mat * m_mat;
    m_mat_inv = m_mat_inv * inv_mat;
    return *this;
}

Transform2D Transform2D::stack(const Transform2D& that) {
    return this->stack(that.m_mat, that.m_mat_inv);
}

Transform3D Transform3D::stack(const Transform3D& that) {
    return this->stack(that.m_mat, that.m_mat_inv);
}

Transform2D Transform2D::translate(float delta_x, float delta_y) {
    const auto v = Vec3f{delta_x, delta_y, 1.f};
    const auto mr = Mat3x3f::identity().column_set(2, v);

    const auto vi = Vec3f{-delta_x, -delta_y, 1.f};
    const auto mi = Mat3x3f::identity().column_set(2, vi);

    return this->stack(mr, mi);
}

Transform3D Transform3D::translate(float delta_x,
                                   float delta_y,
                                   float delta_z) {
    const auto v = Vec4f{delta_x, delta_y, delta_z, 1.f};
    const auto mr = Mat4x4f::identity().column_set(3, v);

    const auto vi = Vec4f{-delta_x, -delta_y, -delta_z, 1.f};
    const auto mi = Mat4x4f::identity().column_set(3, vi);

    return this->stack(mr, mi);
}

Transform2D Transform2D::translate(const Vec2f& delta) {
    return this->translate(delta.x, delta.y);
}

Transform3D Transform3D::translate(const Vec3f& delta) {
    return this->translate(delta.x, delta.y, delta.z);
}

Transform2D Transform2D::rotate(const Vec2f& right_, bool is_normalized) {
    const auto right = is_normalized ? right_ : right_.normalized();

    const auto up = Vec2f{-right.y, right.x};
    const auto m0 = Mat2x2f::from_rows(right, up);
    const auto mr = Mat3x3f{m0};

    const auto mi = mr.transposed();

    return this->stack(mr, mi);
}

Transform3D Transform3D::rotate(const Vec3f& forward_,
                                const Vec3f& up_dir_,
                                bool is_normalized) {
    const auto up_dir = is_normalized ? up_dir_ : up_dir_.normalized();
    const auto forward = is_normalized ? forward_ : forward_.normalized();

    const auto right = cross(up_dir, forward);
    const auto up = cross(forward, right);

    const auto m0 = Mat3x3f::from_rows(right, up, forward);
    const auto mr = Mat4x4f{m0};

    const auto mi = mr.transposed();

    return this->stack(mr, mi);
}

Transform2D Transform2D::rotate_clockwise(float angle_x) {
    const auto x = std::cos(angle_x);
    const auto y = std::sin(angle_x);

    return this->rotate(Vec2f{x, y}, true);
}

Transform2D Transform2D::rotate_counterclockwise(float angle_x) {
    return this->rotate_clockwise(-angle_x);
}

Transform3D Transform3D::rotateX(float angle_x) {
    const auto up_dir_y = std::cos(angle_x);
    const auto up_dir_z = std::sin(angle_x);
    const auto forward_y = -up_dir_z;
    const auto forward_z = up_dir_y;

    return this->rotate(Vec3f{0.f, forward_y, forward_z},
                        Vec3f{0.f, up_dir_y, up_dir_z}, true);
}

Transform3D Transform3D::rotateY(float angle_y) {
    const auto up_dir = UP;
    const auto forward_x = std::cos(angle_y);
    const auto forward_z = std::sin(angle_y);

    return this->rotate(Vec3f{forward_x, 0.f, forward_z}, up_dir, true);
}

Transform3D Transform3D::rotateZ(float angle_z) {
    const auto forward = FORWARD;
    const auto up_dir_x = std::cos(angle_z);
    const auto up_dir_y = std::sin(angle_z);

    return this->rotate(forward, Vec3f{up_dir_x, up_dir_y, 0.f}, true);
}

Transform2D Transform2D::scale(float scale_x, float scale_y) {
    assert(scale_x != 0.f);
    assert(scale_y != 0.f);

    Mat3x3f mr{}, mi{};

    mr(0, 0) = scale_x;
    mr(1, 1) = scale_y;
    mr(2, 2) = 1.f;

    mi(0, 0) = 1.f / scale_x;
    mi(1, 1) = 1.f / scale_y;
    mi(2, 2) = 1.f;

    return this->stack(mr, mi);
}

Transform2D Transform2D::scale(const Vec2f& scale) {
    return this->scale(scale.x, scale.y);
}

Transform2D Transform2D::shearX(float sh_x) {
    const auto a = Vec3f{1.0f, sh_x, 0.0f};
    const auto b = Vec3f{0.0f, 1.0f, 0.0f};
    const auto c = Vec3f{0.0f, 0.0f, 1.0f};
    const auto mr = Mat3x3f::from_rows(a, b, c);

    auto mi = Mat3x3f{mr};
    mi(0, 1) = -sh_x;

    return this->stack(mr, mi);
}

Transform2D Transform2D::shearY(float sh_y) {
    const auto a = Vec3f{1.0f, 0.f, 0.f};
    const auto b = Vec3f{sh_y, 1.f, 0.f};
    const auto c = Vec3f{0.0f, 0.f, 1.f};
    const auto mr = Mat3x3f::from_rows(a, b, c);

    auto mi = Mat3x3f{mr};
    mi(1, 0) = -sh_y;

    return this->stack(mr, mi);
}

}  // namespace asciirast::math
