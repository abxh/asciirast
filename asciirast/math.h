/**
 * @file math.h
 * @brief File to import math module
 */

#pragma once

#include "math/AABB.h"
#include "math/Mat.h"
#include "math/Rot.h"
#include "math/Transform.h"
#include "math/Vec.h"
#include "math/utils.h"

namespace asciirast::math {

using IntegralType = int;                  ///< default integral type
using FloatingPointType = float;           ///< default floating point type
static constexpr bool is_col_major = true; ///< default matrix major order

static const auto PI = constants::PI<FloatingPointType>; ///< PI constant

using Vec2Int = Vec<2, IntegralType>; ///< 2D integral type math vector
using Vec3Int = Vec<3, IntegralType>; ///< 3D integral type math vector

using Vec2 = Vec<2, FloatingPointType>; ///< 2D math vector
using Vec3 = Vec<3, FloatingPointType>; ///< 3D math vector
using Vec4 = Vec<4, FloatingPointType>; ///< 4D math vector

using Mat2 = Mat<2, 2, FloatingPointType, is_col_major>; ///< 2x2 matrix
using Mat3 = Mat<3, 3, FloatingPointType, is_col_major>; ///< 3x3 matrix
using Mat4 = Mat<4, 4, FloatingPointType, is_col_major>; ///< 4x4 matrix

using Rot2 = Rot<2, FloatingPointType, is_col_major>; ///< 2D rotation abstraction
using Rot3 = Rot<3, FloatingPointType, is_col_major>; ///< 3D rotation abstraction

using Transform2 = Transform<2, FloatingPointType, is_col_major>; ///< 2D transform abstraction
using Transform3 = Transform<3, FloatingPointType, is_col_major>; ///< 3D transform abstraction

using AABB2 = AABB<2, FloatingPointType, is_col_major>; ///< 2D Axis-Aligned Bounding Box
using AABB3 = AABB<3, FloatingPointType, is_col_major>; ///< 3D Axis-Aligned Bounding Box

};
