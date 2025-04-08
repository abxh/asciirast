/**
 * @file types.h
 * @brief File to import default math types
 */

#pragma once

#include "./math/AABB.h"
#include "./math/Mat.h"
#include "./math/Rot.h"
#include "./math/Transform.h"
#include "./math/Vec.h"

namespace asciirast::math {

using I = int;   ///@< default integral type
using F = float; ///@< default floating point type

static constexpr bool uses_col_major = true; ///@< Default matrix order

using Vec2Int = Vec<2, I>; ///@< 2D int math vector
using Vec3Int = Vec<3, I>; ///@< 3D int math vector
using Vec4Int = Vec<4, I>; ///@< 4D int math vector

using Vec2 = Vec<2, F>; ///@< 2D math vector
using Vec3 = Vec<3, F>; ///@< 3D math vector
using Vec4 = Vec<4, F>; ///@< 4D math vector

using Mat2Int = Mat<2, 2, I, uses_col_major>; ///@< 2x2 int matrix
using Mat3Int = Mat<3, 3, I, uses_col_major>; ///@< 3x3 int matrix
using Mat4Int = Mat<4, 4, I, uses_col_major>; ///@< 4x4 int matrix

using Mat2 = Mat<2, 2, F, uses_col_major>; ///@< 2x2 matrix
using Mat3 = Mat<3, 3, F, uses_col_major>; ///@< 3x3 matrix
using Mat4 = Mat<4, 4, F, uses_col_major>; ///@< 4x4 matrix

using Rot2D = Rot<2, F, uses_col_major>; ///@< 2D rotation abstraction
using Rot3D = Rot<3, F, uses_col_major>; ///@< 3D rotation abstraction

using Transform2D = Transform<2, F, uses_col_major>; ///@< 2D Transformation abstraction
using Transform3D = Transform<3, F, uses_col_major>; ///@< 3D Transformation abstraction

using AABB2D = AABB<2, F, uses_col_major>; ///@< 2D Axis-Aligned Bounding Box
using AABB3D = AABB<3, F, uses_col_major>; ///@< 3D Axis-Aligned Bounding Box

}; // namespace asciirast::math
