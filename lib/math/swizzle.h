#pragma once

#include <cstddef>

// For swizzle class idea.
// - https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/

namespace asciirast::math {

template <std::size_t size, typename T, std::size_t... Indicies>
class Swizzle;

template <std::size_t size, typename T, std::size_t index>
class Swizzle<size, T, index>;

} // namespace asciirast::math
