#pragma once

#include <array>
#include <cstddef>

#include "swizzle.h"

// For swizzle class idea.
// - https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/

namespace asciirast::math {

template <std::size_t size, typename T, std::size_t... Indicies>
class Swizzle {
private:
    static constexpr std::size_t N = sizeof...(Indicies);
    std::array<T, size> e;

public:
    Swizzle &operator=(const Vec<N, T> &rhs)
    {
        std::size_t indicies[] = {Indicies...};
        for (std::size_t i = 0; i < N; i++) {
            e[indicies[i]] = rhs.e[i];
        }
        return *this;
    }
    operator Vec<N, T>() const
    {
        return Vec<N, T>(e[Indicies]...);
    }
};

template <std::size_t size, typename T, std::size_t index>
class Swizzle<size, T, index> {
private:
    std::array<T, size> e;

public:
    Swizzle &operator=(const T &rhs)
    {
        e[index] = rhs;
        return *this;
    }
    operator T() const
    {
        return e[index];
    }
};

} // namespace asciirast::math
