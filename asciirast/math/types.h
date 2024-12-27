#pragma once

#include <cstddef>
#include <numbers>
#include <type_traits>

namespace asciirast::math {

template <size_t M_y, size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class Mat;

template <size_t N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec;

using size_t = std::size_t;

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

using Mat3x3f = Mat<3, 3, float>;
using Mat4x4f = Mat<4, 4, float>;

static inline const float PI_f = std::numbers::pi_v<float>;

};  // namespace asciirast::math
