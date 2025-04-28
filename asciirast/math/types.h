/**
 * @file types.h
 * @brief File to import default math types
 */

#pragma once

#include "./AABB.h"
#include "./Mat.h"
#include "./Rot.h"
#include "./Transform.h"
#include "./Vec.h"

#include <cstdint>

namespace asciirast::math {

using Int = int;     ///< default integral type
using Float = float; ///< default floating point type

static constexpr bool uses_col_major = true; ///< Default matrix order

using Vec2Int = Vec<2, Int>; ///< 2D int math vector
using Vec3Int = Vec<3, Int>; ///< 3D int math vector
using Vec4Int = Vec<4, Int>; ///< 4D int math vector

using RGBA_8bit = Vec<4, std::uint8_t>; ///< RGBA 8 bit encoding

using Vec2 = Vec<2, Float>; ///< 2D math vector
using Vec3 = Vec<3, Float>; ///< 3D math vector
using Vec4 = Vec<4, Float>; ///< 4D math vector

using Mat2 = Mat<2, 2, Float, uses_col_major>; ///< 2x2 matrix
using Mat3 = Mat<3, 3, Float, uses_col_major>; ///< 3x3 matrix
using Mat4 = Mat<4, 4, Float, uses_col_major>; ///< 4x4 matrix

using Rot2D = Rot2DType<Float, uses_col_major>; ///< 2D rotation abstraction
using Rot3D = Rot3DType<Float, uses_col_major>; ///< 3D rotation abstraction

using Transform2D = Transform2DType<Float, uses_col_major>; ///< 2D Transformation abstraction
using Transform3D = Transform3DType<Float, uses_col_major>; ///< 3D Transformation abstraction

using AABB2D = AABB<2, Float, uses_col_major>; ///< 2D Axis-Aligned Bounding Box
using AABB3D = AABB<3, Float, uses_col_major>; ///< 3D Axis-Aligned Bounding Box

}; // namespace asciirast::math
