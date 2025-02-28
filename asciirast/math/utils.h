#pragma once

#include <numbers>
#include <type_traits>

namespace asciirast::math {

template<typename T>
static const T PI = std::numbers::pi; // PI constant

template<typename T>
static const T PI_2 = std::numbers::pi + std::numbers::pi; // PI_2 constant

template<typename T>
    requires(std::is_floating_point_v<T>)
T
angle_as_radians(const T degrees)
{
    return PI_2<T> * degrees / T{ 360 };
}

template<typename T>
    requires(std::is_floating_point_v<T>)
T
angle_as_degrees(const T radians)
{
    return T{ 360 } * radians / PI_2<T>;
}

} // namespace asciirast::math
