/**
 * @file Vec.h
 * @brief Math vector class
 */

#pragma once

#include "VecBase.h"
#include "VecLike.h"

namespace asciirast::math {

template <int N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec;

/**
 * @brief Vector information trait
 */
template <typename TT>
struct vec_info {
    using value_type = void;
    static constexpr bool is_vec = false;
    static constexpr int size = 1;
};

/**
 * @brief Vector information trait
 */
template <int M, typename T>
struct vec_info<Vec<M, T>> {
    using value_type = T;
    static constexpr bool is_vec = true;
    static constexpr int size = M;
};

/**
 * @brief math vector class
 */
template <int N, typename T>
    requires(N > 0 && std::is_arithmetic_v<T>)
class Vec : public VecBase<Vec, N, T>,
            public VecLike<Vec<N, T>, Vec<N, T>, N, T> {
public:
    using VecBase<Vec, N, T>::m_components;

    /**
     * @brief Default constructor. Set all values to 0
     */
    constexpr Vec() {
        for (int i = 0; i < N; i++) {
            m_components[i] = T{0};
        }
    }

    /**
     * @brief Set all values to initial value
     */
    constexpr explicit Vec(const T initial_value) {
        for (int i = 0; i < N; i++) {
            m_components[i] = initial_value;
        }
    };

    /**
     * @brief Set vector from a mix of values and vectors. Rest of the vector is
     * filled with zero.
     */
    template <typename... Args>
        requires((std::is_convertible_v<Args, T> || vec_info<Args>::is_vec) && ...)
    constexpr explicit Vec(const Args&... args)
        requires((vec_info<Args>::size + ...) <= N)
    {
        int idx = 0;
        set_using(idx, *this, args...);
        for (int i = idx; i < N; i++) {
            m_components[i] = T{0};
        }
    };

    /**
     * @brief Truncate larger vector and make into into a smaller vector.
     */
    template <int M>
        requires(M > N)
    constexpr explicit Vec(const Vec<M, T>& larger_vec) {
        for (int i = 0; i < N; i++) {
            m_components[i] = larger_vec.m_components[i];
        }
    }

    /**
     * @name C++ iterator support
     * @{
     * with begin() and end()
     */
    using iterator = typename std::array<T, N>::iterator;
    using const_iterator = typename std::array<T, N>::const_iterator;

    iterator begin() { return m_components.begin(); }
    iterator end() { return m_components.end(); }
    const_iterator begin() const { return m_components.begin(); }
    const_iterator end() const { return m_components.end(); }
    /// @}

private:
    template <typename W, typename Res, int M, typename U>
        requires(M > 0 && std::is_arithmetic_v<U>)
    friend class VecLike;

    constexpr T& index(int i) { return m_components[i]; }
    constexpr T index(int i) const { return m_components[i]; }

    static void set_using(int& idx, Vec& out) {};

    template <typename... Args>
    static constexpr void set_using(int& idx,
                                    Vec& out,
                                    const T& val,
                                    const Args&... args) {
        out.index(idx++) = val;
        set_using(idx, out, args...);
    }

    template <int M, typename... Args>
    static constexpr void set_using(int& idx,
                                    Vec& out,
                                    const Vec<M, T>& vec,
                                    const Args&... args) {
        for (int i = 0; i < M; i++) {
            out.index(idx++) = vec[i];
        }
        set_using(idx, out, args...);
    }
};

}  // namespace asciirast::math
