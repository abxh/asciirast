/**
 * @file Mat.h
 * @brief Matrix class
 */

#pragma once

#include "MatBase.h"

namespace asciirast::math {

template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class Mat : MatBase<M_y, N_x, T>;

}  // namespace asciirast::math
