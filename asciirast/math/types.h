/**
 * @file types.h
 * @brief File to import math types
 */

#pragma once

#include "./math/AABB.h"
#include "./math/Mat.h"
#include "./math/Rot.h"
#include "./math/Transform.h"
#include "./math/Vec.h"

namespace asciirast::math {

using I = int;   ///< default integral type
using F = float; ///< default floating point type

static constexpr bool is_col_major = true; ///< default matrix major order

using Vec2Int = Vec<2, I>; ///< 2D integral type math vector
using Vec3Int = Vec<3, I>; ///< 3D integral type math vector
using Vec4Int = Vec<4, I>; ///< 4D integral type math vector

using Vec2 = Vec<2, F>; ///< 2D math vector
using Vec3 = Vec<3, F>; ///< 3D math vector
using Vec4 = Vec<4, F>; ///< 4D math vector

using Mat2Int = Mat<2, 2, I, is_col_major>; ///< 2x2 integral type matrix
using Mat3Int = Mat<3, 3, I, is_col_major>; ///< 3x3 integral type matrix
using Mat4Int = Mat<4, 4, I, is_col_major>; ///< 4x4 integral type matrix

using Mat2 = Mat<2, 2, F, is_col_major>; ///< 2x2 matrix
using Mat3 = Mat<3, 3, F, is_col_major>; ///< 3x3 matrix
using Mat4 = Mat<4, 4, F, is_col_major>; ///< 4x4 matrix

using Rot2D = Rot<2, F, is_col_major>; ///< 2D rotation abstraction
using Rot3D = Rot<3, F, is_col_major>; ///< 3D rotation abstraction

using Transform2D = Transform<2, F, is_col_major>; ///< 2D transform abstraction
using Transform3D = Transform<3, F, is_col_major>; ///< 3D transform abstraction

using AABB2D = AABB<2, F, is_col_major>; ///< 2D Axis-Aligned Bounding Box
using AABB3D = AABB<3, F, is_col_major>; ///< 3D Axis-Aligned Bounding Box

};
