#pragma once

#include <cstddef>
#include <numbers>

#include "types/Mat.h"
#include "types/Vec.h"

namespace asciirast::math {

using size_t = std::size_t;

using Vec2f = Vec<2, float>;
using Vec3f = Vec<3, float>;
using Vec4f = Vec<4, float>;

using Mat3x3f = Mat<3, 3, float>;
using Mat4x4f = Mat<4, 4, float>;

static inline const float PI_f = std::numbers::pi_v<float>;

};  // namespace asciirast::math
