/**
 * @file VecBase.h
 * @brief Vector base class. Here are the members defined.
 */
#pragma once

#include "Swizzled.h"
#include "utils/non_narrowing_conv.h"

#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief Base vector.
 */
template <template <std::size_t, typename> class Vec, std::size_t N, typename T>
    requires(N > 1)
class VecBase {
public:
    /**
     * @brief The value type.
     */
    using value_type = T;

    /**
     * @brief The size of the vector
     */
    consteval auto size() const
    {
        return N;
    }

    /**
     * @brief Default constructor. Fill all values in vector with default value
     * constructor.
     */
    VecBase()
    {
        std::fill(this->begin(), this->end(), value_type{});
    }

    /**
     * @brief Use initial value to fill the entire array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const U &intital_value)
    {
        std::fill(this->begin(), this->end(), value_type{intital_value});
    }

    /**
     * @brief Use N values to fill the N-sized array.
     */
    template <typename... Us>
        requires(utils::non_narrowing_conv<value_type, Us...>)
    VecBase(const Us &...values)
        requires(N == sizeof...(Us))
        : e{value_type{values}...} {};

    /**
     * @brief Use std::array to fill the array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::array<T, N> &array)
    {
        std::copy(array.begin(), array.end(), this->begin());
    };

    /**
     * @brief Use pointer to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::size_t n, const T *ptr)
        requires(n == N)
    {
        std::copy_n(ptr, n, this->begin());
    };

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Vector begin iterator. To be used with higher-level functions
     */
    auto begin()
    {
        return &this->e[0];
    }
    /**
     * @brief Vector begin const iterator. To be used with higher-level
     * functions
     */
    auto begin() const
    {
        return &this->e[0];
    }
    /**
     * @brief Vector end iterator. To be used with higher-level functions
     */
    auto end()
    {
        return &this->e[N];
    }
    /**
     * @brief Vector end const iterator. To be used with higher-level functions
     */
    auto end() const
    {
        return &this->e[N];
    }
    /**
     * @brief Get the value pointer
     */
    const T *get_value_ptr() const
    {
        return std::decay(this->e);
    }

public:
    /// @cond DO_NO_DOCUMENT
    T e[N];
    /// @endcond
};

/**
 * @brief Specialized 2-sized base vector. Swizzled combinations of {x, y} of
 * max size 2 can be accessed as a class member.
 */
template <template <std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 2, T> {
private:
    static constexpr std::size_t N = 2;

public:
    /**
     * @brief The value type.
     */
    using value_type = T;

    /**
     * @brief The size of the vector
     */
    consteval auto size() const
    {
        return N;
    }

    /**
     * @brief Default constructor. Fill all values in vector with default value
     * constructor.
     */
    VecBase()
    {
        std::fill(this->begin(), this->end(), value_type{});
    }

    /**
     * @brief Use initial value to fill the entire array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const U &intital_value)
    {
        std::fill(this->begin(), this->end(), value_type{intital_value});
    }

    /**
     * @brief Use N values to fill the N-sized array.
     */
    template <typename... Us>
        requires(utils::non_narrowing_conv<value_type, Us...>)
    VecBase(const Us &...values)
        requires(N == sizeof...(Us))
        : e{value_type{values}...} {};

    /**
     * @brief Use std::array to fill the array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::array<T, N> &array)
    {
        std::copy(array.begin(), array.end(), this->begin());
    };

    /**
     * @brief Use pointer to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::size_t n, const T *ptr)
        requires(n == N)
    {
        std::copy_n(ptr, n, this->begin());
    };

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Vector begin iterator. To be used with higher-level functions
     */
    auto begin()
    {
        return &this->e[0];
    }
    /**
     * @brief Vector begin const iterator. To be used with higher-level
     * functions
     */
    auto begin() const
    {
        return &this->e[0];
    }
    /**
     * @brief Vector end iterator. To be used with higher-level functions
     */
    auto end()
    {
        return &this->e[N];
    }
    /**
     * @brief Vector end const iterator. To be used with higher-level functions
     */
    auto end() const
    {
        return &this->e[N];
    }

    /**
     * @brief Get the value pointer
     */
    const T *get_value_ptr() const
    {
        return std::decay(this->e);
    }

public:
    /// @cond DO_NO_DOCUMENT
    union {
        T e[N];

        Swizzled<Vec, N, T, 0> x;
        Swizzled<Vec, N, T, 1> y;

        Swizzled<Vec, N, T, 0, 0> xx;
        Swizzled<Vec, N, T, 0, 1> xy;
        Swizzled<Vec, N, T, 1, 0> yx;
        Swizzled<Vec, N, T, 1, 1> yy;
    };
    /// @endcond
};

/**
 * @brief Specialized 3-sized base vector. Swizzled combinations of {x, y, z}
 * and {r, g, b} of max size 3 can be accessed as a class member.
 */
template <template <std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 3, T> {
private:
    static constexpr std::size_t N = 3;

public:
    /**
     * @brief The value type.
     */
    using value_type = T;

    /**
     * @brief The size of the vector
     */
    consteval auto size() const
    {
        return N;
    }

    /**
     * @brief Default constructor. Fill all values in vector with default value
     * constructor.
     */
    VecBase()
    {
        std::fill(this->begin(), this->end(), value_type{});
    }

    /**
     * @brief Use initial value to fill the entire array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const U &intital_value)
    {
        std::fill(this->begin(), this->end(), value_type{intital_value});
    }

    /**
     * @brief Use N values to fill the N-sized array.
     */
    template <typename... Us>
        requires(utils::non_narrowing_conv<value_type, Us...>)
    VecBase(const Us &...values)
        requires(N == sizeof...(Us))
        : e{value_type{values}...} {};

    /**
     * @brief Use std::array to fill the array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::array<T, N> &array)
    {
        std::copy(array.begin(), array.end(), this->begin());
    };

    /**
     * @brief Use pointer to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::size_t n, const T *ptr)
        requires(n == N)
    {
        std::copy_n(ptr, n, this->begin());
    };

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Vector begin iterator. To be used with higher-level functions
     */
    auto begin()
    {
        return &this->e[0];
    }
    /**
     * @brief Vector begin const iterator. To be used with higher-level
     * functions
     */
    auto begin() const
    {
        return &this->e[0];
    }
    /**
     * @brief Vector end iterator. To be used with higher-level functions
     */
    auto end()
    {
        return &this->e[N];
    }
    /**
     * @brief Vector end const iterator. To be used with higher-level functions
     */
    auto end() const
    {
        return &this->e[N];
    }

    /**
     * @brief Get the value pointer
     */
    const T *get_value_ptr() const
    {
        return std::decay(this->e);
    }

public:
    /// @cond DO_NO_DOCUMENT
    union {
        T e[N];

        Swizzled<Vec, N, T, 0> x;
        Swizzled<Vec, N, T, 1> y;
        Swizzled<Vec, N, T, 2> z;

        Swizzled<Vec, N, T, 0, 0> xx;
        Swizzled<Vec, N, T, 0, 1> xy;
        Swizzled<Vec, N, T, 0, 2> xz;
        Swizzled<Vec, N, T, 1, 0> yx;
        Swizzled<Vec, N, T, 1, 1> yy;
        Swizzled<Vec, N, T, 1, 2> yz;
        Swizzled<Vec, N, T, 2, 0> zx;
        Swizzled<Vec, N, T, 2, 1> zy;
        Swizzled<Vec, N, T, 2, 2> zz;

        Swizzled<Vec, N, T, 0, 0, 0> xxx;
        Swizzled<Vec, N, T, 0, 0, 1> xxy;
        Swizzled<Vec, N, T, 0, 0, 2> xxz;
        Swizzled<Vec, N, T, 0, 1, 0> xyx;
        Swizzled<Vec, N, T, 0, 1, 1> xyy;
        Swizzled<Vec, N, T, 0, 1, 2> xyz;
        Swizzled<Vec, N, T, 0, 2, 0> xzx;
        Swizzled<Vec, N, T, 0, 2, 1> xzy;
        Swizzled<Vec, N, T, 0, 2, 2> xzz;
        Swizzled<Vec, N, T, 1, 0, 0> yxx;
        Swizzled<Vec, N, T, 1, 0, 1> yxy;
        Swizzled<Vec, N, T, 1, 0, 2> yxz;
        Swizzled<Vec, N, T, 1, 1, 0> yyx;
        Swizzled<Vec, N, T, 1, 1, 1> yyy;
        Swizzled<Vec, N, T, 1, 1, 2> yyz;
        Swizzled<Vec, N, T, 1, 2, 0> yzx;
        Swizzled<Vec, N, T, 1, 2, 1> yzy;
        Swizzled<Vec, N, T, 1, 2, 2> yzz;
        Swizzled<Vec, N, T, 2, 0, 0> zxx;
        Swizzled<Vec, N, T, 2, 0, 1> zxy;
        Swizzled<Vec, N, T, 2, 0, 2> zxz;
        Swizzled<Vec, N, T, 2, 1, 0> zyx;
        Swizzled<Vec, N, T, 2, 1, 1> zyy;
        Swizzled<Vec, N, T, 2, 1, 2> zyz;
        Swizzled<Vec, N, T, 2, 2, 0> zzx;
        Swizzled<Vec, N, T, 2, 2, 1> zzy;
        Swizzled<Vec, N, T, 2, 2, 2> zzz;
    };
    /// @endcond
};

/**
 * @brief Specialized 4-sized base vector. Swizzled combinations of {x, y, z, w}
 * and {r, g, b, a} of max size 4 can be accessed as a class member.
 */
template <template <std::size_t, typename> class Vec, typename T>
class VecBase<Vec, 4, T> {
private:
    static constexpr std::size_t N = 4;

public:
    /**
     * @brief The value type.
     */
    using value_type = T;

    /**
     * @brief The size of the vector
     */
    consteval auto size() const
    {
        return N;
    }

    /**
     * @brief Default constructor. Fill all values in vector with default value
     * constructor.
     */
    VecBase()
    {
        std::fill(this->begin(), this->end(), value_type{});
    }

    /**
     * @brief Use initial value to fill the entire array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const U &intital_value)
    {
        std::fill(this->begin(), this->end(), value_type{intital_value});
    }

    /**
     * @brief Use N values to fill the N-sized array.
     */
    template <typename... Us>
        requires(utils::non_narrowing_conv<value_type, Us...>)
    VecBase(const Us &...values)
        requires(N == sizeof...(Us))
        : e{value_type{values}...} {};

    /**
     * @brief Use std::array to fill the array.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::array<T, N> &array)
    {
        std::copy(array.begin(), array.end(), this->begin());
    };

    /**
     * @brief Use pointer to fill the vector.
     */
    template <typename U>
        requires(utils::non_narrowing_conv<value_type, U>)
    VecBase(const std::size_t n, const T *ptr)
        requires(n == N)
    {
        std::copy_n(ptr, n, this->begin());
    };

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Index the vector. Can throw runtime_error.
     */
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::VecBase<>::operator[]");
        }
        return this->e[i];
    }

    /**
     * @brief Vector begin iterator. To be used with higher-level functions
     */
    auto begin()
    {
        return &this->e[0];
    }
    /**
     * @brief Vector begin const iterator. To be used with higher-level
     * functions
     */
    auto begin() const
    {
        return &this->e[0];
    }
    /**
     * @brief Vector end iterator. To be used with higher-level functions
     */
    auto end()
    {
        return &this->e[N];
    }
    /**
     * @brief Vector end const iterator. To be used with higher-level functions
     */
    auto end() const
    {
        return &this->e[N];
    }

    /**
     * @brief Get the value pointer
     */
    const T *get_value_ptr() const
    {
        return std::decay(this->e);
    }

public:
    /// @cond DO_NO_DOCUMENT
    union {
        T e[N];

        Swizzled<Vec, N, T, 0> x;
        Swizzled<Vec, N, T, 1> y;
        Swizzled<Vec, N, T, 2> z;
        Swizzled<Vec, N, T, 3> w;

        Swizzled<Vec, N, T, 0, 0> xx;
        Swizzled<Vec, N, T, 0, 1> xy;
        Swizzled<Vec, N, T, 0, 2> xz;
        Swizzled<Vec, N, T, 0, 3> xw;
        Swizzled<Vec, N, T, 1, 0> yx;
        Swizzled<Vec, N, T, 1, 1> yy;
        Swizzled<Vec, N, T, 1, 2> yz;
        Swizzled<Vec, N, T, 1, 3> yw;
        Swizzled<Vec, N, T, 2, 0> zx;
        Swizzled<Vec, N, T, 2, 1> zy;
        Swizzled<Vec, N, T, 2, 2> zz;
        Swizzled<Vec, N, T, 2, 3> zw;
        Swizzled<Vec, N, T, 3, 0> wx;
        Swizzled<Vec, N, T, 3, 1> wy;
        Swizzled<Vec, N, T, 3, 2> wz;
        Swizzled<Vec, N, T, 3, 3> ww;

        Swizzled<Vec, N, T, 0, 0, 0> xxx;
        Swizzled<Vec, N, T, 0, 0, 1> xxy;
        Swizzled<Vec, N, T, 0, 0, 2> xxz;
        Swizzled<Vec, N, T, 0, 0, 3> xxw;
        Swizzled<Vec, N, T, 0, 1, 0> xyx;
        Swizzled<Vec, N, T, 0, 1, 1> xyy;
        Swizzled<Vec, N, T, 0, 1, 2> xyz;
        Swizzled<Vec, N, T, 0, 1, 3> xyw;
        Swizzled<Vec, N, T, 0, 2, 0> xzx;
        Swizzled<Vec, N, T, 0, 2, 1> xzy;
        Swizzled<Vec, N, T, 0, 2, 2> xzz;
        Swizzled<Vec, N, T, 0, 2, 3> xzw;
        Swizzled<Vec, N, T, 0, 3, 0> xwx;
        Swizzled<Vec, N, T, 0, 3, 1> xwy;
        Swizzled<Vec, N, T, 0, 3, 2> xwz;
        Swizzled<Vec, N, T, 0, 3, 3> xww;
        Swizzled<Vec, N, T, 1, 0, 0> yxx;
        Swizzled<Vec, N, T, 1, 0, 1> yxy;
        Swizzled<Vec, N, T, 1, 0, 2> yxz;
        Swizzled<Vec, N, T, 1, 0, 3> yxw;
        Swizzled<Vec, N, T, 1, 1, 0> yyx;
        Swizzled<Vec, N, T, 1, 1, 1> yyy;
        Swizzled<Vec, N, T, 1, 1, 2> yyz;
        Swizzled<Vec, N, T, 1, 1, 3> yyw;
        Swizzled<Vec, N, T, 1, 2, 0> yzx;
        Swizzled<Vec, N, T, 1, 2, 1> yzy;
        Swizzled<Vec, N, T, 1, 2, 2> yzz;
        Swizzled<Vec, N, T, 1, 2, 3> yzw;
        Swizzled<Vec, N, T, 1, 3, 0> ywx;
        Swizzled<Vec, N, T, 1, 3, 1> ywy;
        Swizzled<Vec, N, T, 1, 3, 2> ywz;
        Swizzled<Vec, N, T, 1, 3, 3> yww;
        Swizzled<Vec, N, T, 2, 0, 0> zxx;
        Swizzled<Vec, N, T, 2, 0, 1> zxy;
        Swizzled<Vec, N, T, 2, 0, 2> zxz;
        Swizzled<Vec, N, T, 2, 0, 3> zxw;
        Swizzled<Vec, N, T, 2, 1, 0> zyx;
        Swizzled<Vec, N, T, 2, 1, 1> zyy;
        Swizzled<Vec, N, T, 2, 1, 2> zyz;
        Swizzled<Vec, N, T, 2, 1, 3> zyw;
        Swizzled<Vec, N, T, 2, 2, 0> zzx;
        Swizzled<Vec, N, T, 2, 2, 1> zzy;
        Swizzled<Vec, N, T, 2, 2, 2> zzz;
        Swizzled<Vec, N, T, 2, 2, 3> zzw;
        Swizzled<Vec, N, T, 2, 3, 0> zwx;
        Swizzled<Vec, N, T, 2, 3, 1> zwy;
        Swizzled<Vec, N, T, 2, 3, 2> zwz;
        Swizzled<Vec, N, T, 2, 3, 3> zww;
        Swizzled<Vec, N, T, 3, 0, 0> wxx;
        Swizzled<Vec, N, T, 3, 0, 1> wxy;
        Swizzled<Vec, N, T, 3, 0, 2> wxz;
        Swizzled<Vec, N, T, 3, 0, 3> wxw;
        Swizzled<Vec, N, T, 3, 1, 0> wyx;
        Swizzled<Vec, N, T, 3, 1, 1> wyy;
        Swizzled<Vec, N, T, 3, 1, 2> wyz;
        Swizzled<Vec, N, T, 3, 1, 3> wyw;
        Swizzled<Vec, N, T, 3, 2, 0> wzx;
        Swizzled<Vec, N, T, 3, 2, 1> wzy;
        Swizzled<Vec, N, T, 3, 2, 2> wzz;
        Swizzled<Vec, N, T, 3, 2, 3> wzw;
        Swizzled<Vec, N, T, 3, 3, 0> wwx;
        Swizzled<Vec, N, T, 3, 3, 1> wwy;
        Swizzled<Vec, N, T, 3, 3, 2> wwz;
        Swizzled<Vec, N, T, 3, 3, 3> www;

        Swizzled<Vec, N, T, 0, 0, 0, 0> xxxx;
        Swizzled<Vec, N, T, 0, 0, 0, 1> xxxy;
        Swizzled<Vec, N, T, 0, 0, 0, 2> xxxz;
        Swizzled<Vec, N, T, 0, 0, 0, 3> xxxw;
        Swizzled<Vec, N, T, 0, 0, 1, 0> xxyx;
        Swizzled<Vec, N, T, 0, 0, 1, 1> xxyy;
        Swizzled<Vec, N, T, 0, 0, 1, 2> xxyz;
        Swizzled<Vec, N, T, 0, 0, 1, 3> xxyw;
        Swizzled<Vec, N, T, 0, 0, 2, 0> xxzx;
        Swizzled<Vec, N, T, 0, 0, 2, 1> xxzy;
        Swizzled<Vec, N, T, 0, 0, 2, 2> xxzz;
        Swizzled<Vec, N, T, 0, 0, 2, 3> xxzw;
        Swizzled<Vec, N, T, 0, 0, 3, 0> xxwx;
        Swizzled<Vec, N, T, 0, 0, 3, 1> xxwy;
        Swizzled<Vec, N, T, 0, 0, 3, 2> xxwz;
        Swizzled<Vec, N, T, 0, 0, 3, 3> xxww;
        Swizzled<Vec, N, T, 0, 1, 0, 0> xyxx;
        Swizzled<Vec, N, T, 0, 1, 0, 1> xyxy;
        Swizzled<Vec, N, T, 0, 1, 0, 2> xyxz;
        Swizzled<Vec, N, T, 0, 1, 0, 3> xyxw;
        Swizzled<Vec, N, T, 0, 1, 1, 0> xyyx;
        Swizzled<Vec, N, T, 0, 1, 1, 1> xyyy;
        Swizzled<Vec, N, T, 0, 1, 1, 2> xyyz;
        Swizzled<Vec, N, T, 0, 1, 1, 3> xyyw;
        Swizzled<Vec, N, T, 0, 1, 2, 0> xyzx;
        Swizzled<Vec, N, T, 0, 1, 2, 1> xyzy;
        Swizzled<Vec, N, T, 0, 1, 2, 2> xyzz;
        Swizzled<Vec, N, T, 0, 1, 2, 3> xyzw;
        Swizzled<Vec, N, T, 0, 1, 3, 0> xywx;
        Swizzled<Vec, N, T, 0, 1, 3, 1> xywy;
        Swizzled<Vec, N, T, 0, 1, 3, 2> xywz;
        Swizzled<Vec, N, T, 0, 1, 3, 3> xyww;
        Swizzled<Vec, N, T, 0, 2, 0, 0> xzxx;
        Swizzled<Vec, N, T, 0, 2, 0, 1> xzxy;
        Swizzled<Vec, N, T, 0, 2, 0, 2> xzxz;
        Swizzled<Vec, N, T, 0, 2, 0, 3> xzxw;
        Swizzled<Vec, N, T, 0, 2, 1, 0> xzyx;
        Swizzled<Vec, N, T, 0, 2, 1, 1> xzyy;
        Swizzled<Vec, N, T, 0, 2, 1, 2> xzyz;
        Swizzled<Vec, N, T, 0, 2, 1, 3> xzyw;
        Swizzled<Vec, N, T, 0, 2, 2, 0> xzzx;
        Swizzled<Vec, N, T, 0, 2, 2, 1> xzzy;
        Swizzled<Vec, N, T, 0, 2, 2, 2> xzzz;
        Swizzled<Vec, N, T, 0, 2, 2, 3> xzzw;
        Swizzled<Vec, N, T, 0, 2, 3, 0> xzwx;
        Swizzled<Vec, N, T, 0, 2, 3, 1> xzwy;
        Swizzled<Vec, N, T, 0, 2, 3, 2> xzwz;
        Swizzled<Vec, N, T, 0, 2, 3, 3> xzww;
        Swizzled<Vec, N, T, 0, 3, 0, 0> xwxx;
        Swizzled<Vec, N, T, 0, 3, 0, 1> xwxy;
        Swizzled<Vec, N, T, 0, 3, 0, 2> xwxz;
        Swizzled<Vec, N, T, 0, 3, 0, 3> xwxw;
        Swizzled<Vec, N, T, 0, 3, 1, 0> xwyx;
        Swizzled<Vec, N, T, 0, 3, 1, 1> xwyy;
        Swizzled<Vec, N, T, 0, 3, 1, 2> xwyz;
        Swizzled<Vec, N, T, 0, 3, 1, 3> xwyw;
        Swizzled<Vec, N, T, 0, 3, 2, 0> xwzx;
        Swizzled<Vec, N, T, 0, 3, 2, 1> xwzy;
        Swizzled<Vec, N, T, 0, 3, 2, 2> xwzz;
        Swizzled<Vec, N, T, 0, 3, 2, 3> xwzw;
        Swizzled<Vec, N, T, 0, 3, 3, 0> xwwx;
        Swizzled<Vec, N, T, 0, 3, 3, 1> xwwy;
        Swizzled<Vec, N, T, 0, 3, 3, 2> xwwz;
        Swizzled<Vec, N, T, 0, 3, 3, 3> xwww;

        // copy of above:
        Swizzled<Vec, N, T, 1, 0, 0, 0> yxxx;
        Swizzled<Vec, N, T, 1, 0, 0, 1> yxxy;
        Swizzled<Vec, N, T, 1, 0, 0, 2> yxxz;
        Swizzled<Vec, N, T, 1, 0, 0, 3> yxxw;
        Swizzled<Vec, N, T, 1, 0, 1, 0> yxyx;
        Swizzled<Vec, N, T, 1, 0, 1, 1> yxyy;
        Swizzled<Vec, N, T, 1, 0, 1, 2> yxyz;
        Swizzled<Vec, N, T, 1, 0, 1, 3> yxyw;
        Swizzled<Vec, N, T, 1, 0, 2, 0> yxzx;
        Swizzled<Vec, N, T, 1, 0, 2, 1> yxzy;
        Swizzled<Vec, N, T, 1, 0, 2, 2> yxzz;
        Swizzled<Vec, N, T, 1, 0, 2, 3> yxzw;
        Swizzled<Vec, N, T, 1, 0, 3, 0> yxwx;
        Swizzled<Vec, N, T, 1, 0, 3, 1> yxwy;
        Swizzled<Vec, N, T, 1, 0, 3, 2> yxwz;
        Swizzled<Vec, N, T, 1, 0, 3, 3> yxww;
        Swizzled<Vec, N, T, 1, 1, 0, 0> yyxx;
        Swizzled<Vec, N, T, 1, 1, 0, 1> yyxy;
        Swizzled<Vec, N, T, 1, 1, 0, 2> yyxz;
        Swizzled<Vec, N, T, 1, 1, 0, 3> yyxw;
        Swizzled<Vec, N, T, 1, 1, 1, 0> yyyx;
        Swizzled<Vec, N, T, 1, 1, 1, 1> yyyy;
        Swizzled<Vec, N, T, 1, 1, 1, 2> yyyz;
        Swizzled<Vec, N, T, 1, 1, 1, 3> yyyw;
        Swizzled<Vec, N, T, 1, 1, 2, 0> yyzx;
        Swizzled<Vec, N, T, 1, 1, 2, 1> yyzy;
        Swizzled<Vec, N, T, 1, 1, 2, 2> yyzz;
        Swizzled<Vec, N, T, 1, 1, 2, 3> yyzw;
        Swizzled<Vec, N, T, 1, 1, 3, 0> yywx;
        Swizzled<Vec, N, T, 1, 1, 3, 1> yywy;
        Swizzled<Vec, N, T, 1, 1, 3, 2> yywz;
        Swizzled<Vec, N, T, 1, 1, 3, 3> yyww;
        Swizzled<Vec, N, T, 1, 2, 0, 0> yzxx;
        Swizzled<Vec, N, T, 1, 2, 0, 1> yzxy;
        Swizzled<Vec, N, T, 1, 2, 0, 2> yzxz;
        Swizzled<Vec, N, T, 1, 2, 0, 3> yzxw;
        Swizzled<Vec, N, T, 1, 2, 1, 0> yzyx;
        Swizzled<Vec, N, T, 1, 2, 1, 1> yzyy;
        Swizzled<Vec, N, T, 1, 2, 1, 2> yzyz;
        Swizzled<Vec, N, T, 1, 2, 1, 3> yzyw;
        Swizzled<Vec, N, T, 1, 2, 2, 0> yzzx;
        Swizzled<Vec, N, T, 1, 2, 2, 1> yzzy;
        Swizzled<Vec, N, T, 1, 2, 2, 2> yzzz;
        Swizzled<Vec, N, T, 1, 2, 2, 3> yzzw;
        Swizzled<Vec, N, T, 1, 2, 3, 0> yzwx;
        Swizzled<Vec, N, T, 1, 2, 3, 1> yzwy;
        Swizzled<Vec, N, T, 1, 2, 3, 2> yzwz;
        Swizzled<Vec, N, T, 1, 2, 3, 3> yzww;
        Swizzled<Vec, N, T, 1, 3, 0, 0> ywxx;
        Swizzled<Vec, N, T, 1, 3, 0, 1> ywxy;
        Swizzled<Vec, N, T, 1, 3, 0, 2> ywxz;
        Swizzled<Vec, N, T, 1, 3, 0, 3> ywxw;
        Swizzled<Vec, N, T, 1, 3, 1, 0> ywyx;
        Swizzled<Vec, N, T, 1, 3, 1, 1> ywyy;
        Swizzled<Vec, N, T, 1, 3, 1, 2> ywyz;
        Swizzled<Vec, N, T, 1, 3, 1, 3> ywyw;
        Swizzled<Vec, N, T, 1, 3, 2, 0> ywzx;
        Swizzled<Vec, N, T, 1, 3, 2, 1> ywzy;
        Swizzled<Vec, N, T, 1, 3, 2, 2> ywzz;
        Swizzled<Vec, N, T, 1, 3, 2, 3> ywzw;
        Swizzled<Vec, N, T, 1, 3, 3, 0> ywwx;
        Swizzled<Vec, N, T, 1, 3, 3, 1> ywwy;
        Swizzled<Vec, N, T, 1, 3, 3, 2> ywwz;
        Swizzled<Vec, N, T, 1, 3, 3, 3> ywww;

        // copy of above:
        Swizzled<Vec, N, T, 2, 0, 0, 0> zxxx;
        Swizzled<Vec, N, T, 2, 0, 0, 1> zxxy;
        Swizzled<Vec, N, T, 2, 0, 0, 2> zxxz;
        Swizzled<Vec, N, T, 2, 0, 0, 3> zxxw;
        Swizzled<Vec, N, T, 2, 0, 1, 0> zxyx;
        Swizzled<Vec, N, T, 2, 0, 1, 1> zxyy;
        Swizzled<Vec, N, T, 2, 0, 1, 2> zxyz;
        Swizzled<Vec, N, T, 2, 0, 1, 3> zxyw;
        Swizzled<Vec, N, T, 2, 0, 2, 0> zxzx;
        Swizzled<Vec, N, T, 2, 0, 2, 1> zxzy;
        Swizzled<Vec, N, T, 2, 0, 2, 2> zxzz;
        Swizzled<Vec, N, T, 2, 0, 2, 3> zxzw;
        Swizzled<Vec, N, T, 2, 0, 3, 0> zxwx;
        Swizzled<Vec, N, T, 2, 0, 3, 1> zxwy;
        Swizzled<Vec, N, T, 2, 0, 3, 2> zxwz;
        Swizzled<Vec, N, T, 2, 0, 3, 3> zxww;
        Swizzled<Vec, N, T, 2, 1, 0, 0> zyxx;
        Swizzled<Vec, N, T, 2, 1, 0, 1> zyxy;
        Swizzled<Vec, N, T, 2, 1, 0, 2> zyxz;
        Swizzled<Vec, N, T, 2, 1, 0, 3> zyxw;
        Swizzled<Vec, N, T, 2, 1, 1, 0> zyyx;
        Swizzled<Vec, N, T, 2, 1, 1, 1> zyyy;
        Swizzled<Vec, N, T, 2, 1, 1, 2> zyyz;
        Swizzled<Vec, N, T, 2, 1, 1, 3> zyyw;
        Swizzled<Vec, N, T, 2, 1, 2, 0> zyzx;
        Swizzled<Vec, N, T, 2, 1, 2, 1> zyzy;
        Swizzled<Vec, N, T, 2, 1, 2, 2> zyzz;
        Swizzled<Vec, N, T, 2, 1, 2, 3> zyzw;
        Swizzled<Vec, N, T, 2, 1, 3, 0> zywx;
        Swizzled<Vec, N, T, 2, 1, 3, 1> zywy;
        Swizzled<Vec, N, T, 2, 1, 3, 2> zywz;
        Swizzled<Vec, N, T, 2, 1, 3, 3> zyww;
        Swizzled<Vec, N, T, 2, 2, 0, 0> zzxx;
        Swizzled<Vec, N, T, 2, 2, 0, 1> zzxy;
        Swizzled<Vec, N, T, 2, 2, 0, 2> zzxz;
        Swizzled<Vec, N, T, 2, 2, 0, 3> zzxw;
        Swizzled<Vec, N, T, 2, 2, 1, 0> zzyx;
        Swizzled<Vec, N, T, 2, 2, 1, 1> zzyy;
        Swizzled<Vec, N, T, 2, 2, 1, 2> zzyz;
        Swizzled<Vec, N, T, 2, 2, 1, 3> zzyw;
        Swizzled<Vec, N, T, 2, 2, 2, 0> zzzx;
        Swizzled<Vec, N, T, 2, 2, 2, 1> zzzy;
        Swizzled<Vec, N, T, 2, 2, 2, 2> zzzz;
        Swizzled<Vec, N, T, 2, 2, 2, 3> zzzw;
        Swizzled<Vec, N, T, 2, 2, 3, 0> zzwx;
        Swizzled<Vec, N, T, 2, 2, 3, 1> zzwy;
        Swizzled<Vec, N, T, 2, 2, 3, 2> zzwz;
        Swizzled<Vec, N, T, 2, 2, 3, 3> zzww;
        Swizzled<Vec, N, T, 2, 3, 0, 0> zwxx;
        Swizzled<Vec, N, T, 2, 3, 0, 1> zwxy;
        Swizzled<Vec, N, T, 2, 3, 0, 2> zwxz;
        Swizzled<Vec, N, T, 2, 3, 0, 3> zwxw;
        Swizzled<Vec, N, T, 2, 3, 1, 0> zwyx;
        Swizzled<Vec, N, T, 2, 3, 1, 1> zwyy;
        Swizzled<Vec, N, T, 2, 3, 1, 2> zwyz;
        Swizzled<Vec, N, T, 2, 3, 1, 3> zwyw;
        Swizzled<Vec, N, T, 2, 3, 2, 0> zwzx;
        Swizzled<Vec, N, T, 2, 3, 2, 1> zwzy;
        Swizzled<Vec, N, T, 2, 3, 2, 2> zwzz;
        Swizzled<Vec, N, T, 2, 3, 2, 3> zwzw;
        Swizzled<Vec, N, T, 2, 3, 3, 0> zwwx;
        Swizzled<Vec, N, T, 2, 3, 3, 1> zwwy;
        Swizzled<Vec, N, T, 2, 3, 3, 2> zwwz;
        Swizzled<Vec, N, T, 2, 3, 3, 3> zwww;

        // copy of above:
        Swizzled<Vec, N, T, 3, 0, 0, 0> wxxx;
        Swizzled<Vec, N, T, 3, 0, 0, 1> wxxy;
        Swizzled<Vec, N, T, 3, 0, 0, 2> wxxz;
        Swizzled<Vec, N, T, 3, 0, 0, 3> wxxw;
        Swizzled<Vec, N, T, 3, 0, 1, 0> wxyx;
        Swizzled<Vec, N, T, 3, 0, 1, 1> wxyy;
        Swizzled<Vec, N, T, 3, 0, 1, 2> wxyz;
        Swizzled<Vec, N, T, 3, 0, 1, 3> wxyw;
        Swizzled<Vec, N, T, 3, 0, 2, 0> wxzx;
        Swizzled<Vec, N, T, 3, 0, 2, 1> wxzy;
        Swizzled<Vec, N, T, 3, 0, 2, 2> wxzz;
        Swizzled<Vec, N, T, 3, 0, 2, 3> wxzw;
        Swizzled<Vec, N, T, 3, 0, 3, 0> wxwx;
        Swizzled<Vec, N, T, 3, 0, 3, 1> wxwy;
        Swizzled<Vec, N, T, 3, 0, 3, 2> wxwz;
        Swizzled<Vec, N, T, 3, 0, 3, 3> wxww;
        Swizzled<Vec, N, T, 3, 1, 0, 0> wyxx;
        Swizzled<Vec, N, T, 3, 1, 0, 1> wyxy;
        Swizzled<Vec, N, T, 3, 1, 0, 2> wyxz;
        Swizzled<Vec, N, T, 3, 1, 0, 3> wyxw;
        Swizzled<Vec, N, T, 3, 1, 1, 0> wyyx;
        Swizzled<Vec, N, T, 3, 1, 1, 1> wyyy;
        Swizzled<Vec, N, T, 3, 1, 1, 2> wyyz;
        Swizzled<Vec, N, T, 3, 1, 1, 3> wyyw;
        Swizzled<Vec, N, T, 3, 1, 2, 0> wyzx;
        Swizzled<Vec, N, T, 3, 1, 2, 1> wyzy;
        Swizzled<Vec, N, T, 3, 1, 2, 2> wyzz;
        Swizzled<Vec, N, T, 3, 1, 2, 3> wyzw;
        Swizzled<Vec, N, T, 3, 1, 3, 0> wywx;
        Swizzled<Vec, N, T, 3, 1, 3, 1> wywy;
        Swizzled<Vec, N, T, 3, 1, 3, 2> wywz;
        Swizzled<Vec, N, T, 3, 1, 3, 3> wyww;
        Swizzled<Vec, N, T, 3, 2, 0, 0> wzxx;
        Swizzled<Vec, N, T, 3, 2, 0, 1> wzxy;
        Swizzled<Vec, N, T, 3, 2, 0, 2> wzxz;
        Swizzled<Vec, N, T, 3, 2, 0, 3> wzxw;
        Swizzled<Vec, N, T, 3, 2, 1, 0> wzyx;
        Swizzled<Vec, N, T, 3, 2, 1, 1> wzyy;
        Swizzled<Vec, N, T, 3, 2, 1, 2> wzyz;
        Swizzled<Vec, N, T, 3, 2, 1, 3> wzyw;
        Swizzled<Vec, N, T, 3, 2, 2, 0> wzzx;
        Swizzled<Vec, N, T, 3, 2, 2, 1> wzzy;
        Swizzled<Vec, N, T, 3, 2, 2, 2> wzzz;
        Swizzled<Vec, N, T, 3, 2, 2, 3> wzzw;
        Swizzled<Vec, N, T, 3, 2, 3, 0> wzwx;
        Swizzled<Vec, N, T, 3, 2, 3, 1> wzwy;
        Swizzled<Vec, N, T, 3, 2, 3, 2> wzwz;
        Swizzled<Vec, N, T, 3, 2, 3, 3> wzww;
        Swizzled<Vec, N, T, 3, 3, 0, 0> wwxx;
        Swizzled<Vec, N, T, 3, 3, 0, 1> wwxy;
        Swizzled<Vec, N, T, 3, 3, 0, 2> wwxz;
        Swizzled<Vec, N, T, 3, 3, 0, 3> wwxw;
        Swizzled<Vec, N, T, 3, 3, 1, 0> wwyx;
        Swizzled<Vec, N, T, 3, 3, 1, 1> wwyy;
        Swizzled<Vec, N, T, 3, 3, 1, 2> wwyz;
        Swizzled<Vec, N, T, 3, 3, 1, 3> wwyw;
        Swizzled<Vec, N, T, 3, 3, 2, 0> wwzx;
        Swizzled<Vec, N, T, 3, 3, 2, 1> wwzy;
        Swizzled<Vec, N, T, 3, 3, 2, 2> wwzz;
        Swizzled<Vec, N, T, 3, 3, 2, 3> wwzw;
        Swizzled<Vec, N, T, 3, 3, 3, 0> wwwx;
        Swizzled<Vec, N, T, 3, 3, 3, 1> wwwy;
        Swizzled<Vec, N, T, 3, 3, 3, 2> wwwz;
        Swizzled<Vec, N, T, 3, 3, 3, 3> wwww;
    };
    /// @endcond
};

} // namespace asciirast::math
