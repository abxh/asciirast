/**
 * @file Swizzled.h
 * @brief To create swizzled vectors
 *
 * Original idea from:
 * https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/
 */

#pragma once

#include <cstddef>

namespace asciirast::math {

/*
*
 * @brief Class to create swizzled vectors.
 */
template <template <std::size_t, typename> class Vec, std::size_t NTotal,
          typename T, std::size_t... Indicies>
class Swizzled {
private:
    static constexpr std::size_t M = sizeof...(Indicies);
    T e[NTotal];

public:
    /**
     * @brief Explicitly convert to vector.
     */
    Vec<M, T> as_vector() const
    {
        return Vec<M, T>{this->e[Indicies]...};
    }
    /**
     * @brief Implicitly convert to vector when the context is clear.
     */
    operator Vec<M, T>() const
    {
        return this->as_vector();
    }
    /**
     * @brief Set using vector.
     */
    Swizzled &operator=(const Vec<M, T> &rhs)
    {
        std::size_t indicies[] = {Indicies...};
        for (std::size_t i = 0; i < M; i++) {
            this->e[indicies[i]] = rhs.e[i];
        }
        return *this;
    }
};

/**
 * @brief Class to create swizzled vectors. Specialized for selecting a single
 * element.
 */
template <template <std::size_t, typename> class Vec, std::size_t Ntotal,
          typename T, std::size_t index>
class Swizzled<Vec, Ntotal, T, index> {
private:
    T e[Ntotal];

public:
    /**
     * @brief Explicitly convert to number.
     */
    T as_num() const
    {
        return this->e[index];
    }
    /**
     * @brief Implicitly convert to number when the context is clear.
     */
    operator T() const
    {
        return this->as_num();
    }
    /**
     * @brief Set using value.
     */
    Swizzled &operator=(const T &rhs)
    {
        e[index] = rhs;
        return *this;
    }
};

} // namespace asciirast::math
