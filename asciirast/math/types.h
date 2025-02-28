/**
 * @file types.h
 * @brief File with definitions for the standard math types
 */

#pragma once

#include "types/Mat.h"
#include "types/Rot.h"
#include "types/Transform.h"
#include "types/Vec.h"

namespace asciirast::math {

using Vec2Int = Vec<2, int>; ///< 2D int math vector
using Vec3Int = Vec<3, int>; ///< 3D int math vector

using Vec2 = Vec<2, float>; ///< 2D float math vector
using Vec3 = Vec<3, float>; ///< 3D float math vector
using Vec4 = Vec<4, float>; ///< 4D float math vector

using Mat2 = Mat<2, 2, float, true>; ///< 2x2 float column-major matrix
using Mat3 = Mat<3, 3, float, true>; ///< 3x3 float column-major matrix
using Mat4 = Mat<4, 4, float, true>; ///< 4x4 float column-major matrix

using Rot2 = Rot<2, float, true>; ///< 2D rotation abstraction
using Rot3 = Rot<3, float, true>; ///< 3D rotation abstraction

using Transform2 = Transform<2, float, true>; ///< 2D transform abstraction
using Transform3 = Transform<3, float, true>; ///< 3D transform abstraction

} // namespace asciirast::math
