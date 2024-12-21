#pragma once

#include <array>
#include <cstddef>
#include <stdexcept>

namespace asciirast::math {

template <std::size_t size, typename T, std::size_t... Indicies>
class Swizzle;

template <std::size_t N, typename T>
    requires(N > 1 && (std::is_integral_v<T> || std::is_floating_point_v<T>))
class VecData {
public:
    VecData()
    {
        std::fill(e.begin(), e.end(), T{});
    };
    VecData(const T &value)
    {
        std::fill(e.begin(), e.end(), value);
    };
    VecData(const std::array<T, N> &array)
        : e{array} {};
    template <class... Ts>
        requires(std::is_same_v<T, Ts> && ...)
    VecData(const Ts &...args)
        requires(sizeof...(Ts) == N)
        : e{args...} {};
    auto begin()
    {
        return e.begin();
    }
    auto end()
    {
        return e.end();
    }
    auto begin() const
    {
        return e.begin();
    }
    auto end() const
    {
        return e.end();
    }
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }

public:
    std::array<T, N> e;
};

template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
class VecData<2, T> {
private:
    static constexpr std::size_t N = 2;

public:
    VecData()
    {
        std::fill(e.begin(), e.end(), T{});
    };
    VecData(const T &value)
    {
        std::fill(e.begin(), e.end(), value);
    };
    VecData(const std::array<T, N> &array)
        : e{array} {};
    template <class... Ts>
        requires(std::is_same_v<T, Ts> && ...)
    VecData(const Ts &...args)
        requires(sizeof...(Ts) == N)
        : e{args...} {};
    auto begin()
    {
        return e.begin();
    }
    auto end()
    {
        return e.end();
    }
    auto begin() const
    {
        return e.begin();
    }
    auto end() const
    {
        return e.end();
    }
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }

public:
    union {
        std::array<T, N> e;

        Swizzle<N, T, 0> x;
        Swizzle<N, T, 1> y;

        Swizzle<N, T, 0, 0> xx;
        Swizzle<N, T, 0, 1> xy;

        Swizzle<N, T, 1, 0> yx;
        Swizzle<N, T, 1, 1> yy;
    };
};

template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
class VecData<3, T> {
private:
    static constexpr std::size_t N = 3;

public:
    VecData()
    {
        std::fill(e.begin(), e.end(), T{});
    };
    VecData(const T &value)
    {
        std::fill(e.begin(), e.end(), value);
    };
    VecData(const std::array<T, N> &array)
        : e{array} {};
    template <class... Ts>
        requires(std::is_same_v<T, Ts> && ...)
    VecData(const Ts &...args)
        requires(sizeof...(Ts) == N)
        : e{args...} {};
    auto begin()
    {
        return e.begin();
    }
    auto end()
    {
        return e.end();
    }
    auto begin() const
    {
        return e.begin();
    }
    auto end() const
    {
        return e.end();
    }
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }

public:
    union {
        std::array<T, N> e;

        Swizzle<N, T, 0> x;
        Swizzle<N, T, 1> y;
        Swizzle<N, T, 2> z;

        Swizzle<N, T, 0, 0> xx;
        Swizzle<N, T, 0, 1> xy;
        Swizzle<N, T, 0, 2> xz;

        Swizzle<N, T, 1, 0> yx;
        Swizzle<N, T, 1, 1> yy;
        Swizzle<N, T, 1, 2> yz;

        Swizzle<N, T, 2, 0> zx;
        Swizzle<N, T, 2, 1> zy;
        Swizzle<N, T, 2, 2> zz;

        Swizzle<N, T, 0, 0, 0> xxx;
        Swizzle<N, T, 0, 0, 1> xxy;
        Swizzle<N, T, 0, 0, 2> xxz;

        Swizzle<N, T, 0, 1, 0> xyx;
        Swizzle<N, T, 0, 1, 1> xyy;
        Swizzle<N, T, 0, 1, 2> xyz;

        Swizzle<N, T, 0, 2, 0> xzx;
        Swizzle<N, T, 0, 2, 1> xzy;
        Swizzle<N, T, 0, 2, 2> xzz;

        Swizzle<N, T, 1, 0, 0> yxx;
        Swizzle<N, T, 1, 0, 1> yxy;
        Swizzle<N, T, 1, 0, 2> yxz;

        Swizzle<N, T, 1, 1, 0> yyx;
        Swizzle<N, T, 1, 1, 1> yyy;
        Swizzle<N, T, 1, 1, 2> yyz;

        Swizzle<N, T, 1, 2, 0> yzx;
        Swizzle<N, T, 1, 2, 1> yzy;
        Swizzle<N, T, 1, 2, 2> yzz;

        Swizzle<N, T, 2, 0, 0> zxx;
        Swizzle<N, T, 2, 0, 1> zxy;
        Swizzle<N, T, 2, 0, 2> zxz;

        Swizzle<N, T, 2, 1, 0> zyx;
        Swizzle<N, T, 2, 1, 1> zyy;
        Swizzle<N, T, 2, 1, 2> zyz;

        Swizzle<N, T, 2, 2, 0> zzx;
        Swizzle<N, T, 2, 2, 1> zzy;
        Swizzle<N, T, 2, 2, 2> zzz;
    };
};

template <typename T>
    requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
class VecData<4, T> {
private:
    static constexpr std::size_t N = 4;

public:
    VecData()
    {
        std::fill(e.begin(), e.end(), T{});
    };
    VecData(const T &value)
    {
        std::fill(e.begin(), e.end(), value);
    };
    VecData(const std::array<T, N> &array)
        : e{array} {};
    template <class... Ts>
        requires(std::is_same_v<T, Ts> && ...)
    VecData(const Ts &...args)
        requires(sizeof...(Ts) == N)
        : e{args...} {};
    auto begin()
    {
        return e.begin();
    }
    auto end()
    {
        return e.end();
    }
    auto begin() const
    {
        return e.begin();
    }
    auto end() const
    {
        return e.end();
    }
    T &operator[](std::size_t i)
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }
    const T &operator[](std::size_t i) const
    {
        if (i >= N) {
            throw std::runtime_error("asciirast::math::Vec<N,T>::operator[]");
        }
        return e[i];
    }

private:
    static constexpr int elm_num = 0;

public:
    union {
        std::array<T, N> e;

        Swizzle<N, T, 0> x;
        Swizzle<N, T, 1> y;
        Swizzle<N, T, 2> z;
        Swizzle<N, T, 3> w;

        Swizzle<N, T, 0, 0> xx;
        Swizzle<N, T, 0, 1> xy;
        Swizzle<N, T, 0, 2> xz;
        Swizzle<N, T, 0, 3> xw;

        Swizzle<N, T, 1, 0> yx;
        Swizzle<N, T, 1, 1> yy;
        Swizzle<N, T, 1, 2> yz;
        Swizzle<N, T, 1, 3> yw;

        Swizzle<N, T, 2, 0> zx;
        Swizzle<N, T, 2, 1> zy;
        Swizzle<N, T, 2, 2> zz;
        Swizzle<N, T, 2, 3> zw;

        Swizzle<N, T, 3, 0> wx;
        Swizzle<N, T, 3, 1> wy;
        Swizzle<N, T, 3, 2> wz;
        Swizzle<N, T, 3, 3> ww;

        Swizzle<N, T, 0, 0, 0> xxx;
        Swizzle<N, T, 0, 0, 1> xxy;
        Swizzle<N, T, 0, 0, 2> xxz;
        Swizzle<N, T, 0, 0, 3> xxw;

        Swizzle<N, T, 0, 1, 0> xyx;
        Swizzle<N, T, 0, 1, 1> xyy;
        Swizzle<N, T, 0, 1, 2> xyz;
        Swizzle<N, T, 0, 1, 3> xyw;

        Swizzle<N, T, 0, 2, 0> xzx;
        Swizzle<N, T, 0, 2, 1> xzy;
        Swizzle<N, T, 0, 2, 2> xzz;
        Swizzle<N, T, 0, 2, 3> xzw;

        Swizzle<N, T, 0, 3, 0> xwx;
        Swizzle<N, T, 0, 3, 1> xwy;
        Swizzle<N, T, 0, 3, 2> xwz;
        Swizzle<N, T, 0, 3, 3> xww;

        Swizzle<N, T, 1, 0, 0> yxx;
        Swizzle<N, T, 1, 0, 1> yxy;
        Swizzle<N, T, 1, 0, 2> yxz;
        Swizzle<N, T, 1, 0, 3> yxw;

        Swizzle<N, T, 1, 1, 0> yyx;
        Swizzle<N, T, 1, 1, 1> yyy;
        Swizzle<N, T, 1, 1, 2> yyz;
        Swizzle<N, T, 1, 1, 3> yyw;

        Swizzle<N, T, 1, 2, 0> yzx;
        Swizzle<N, T, 1, 2, 1> yzy;
        Swizzle<N, T, 1, 2, 2> yzz;
        Swizzle<N, T, 1, 2, 3> yzw;

        Swizzle<N, T, 1, 3, 0> ywx;
        Swizzle<N, T, 1, 3, 1> ywy;
        Swizzle<N, T, 1, 3, 2> ywz;
        Swizzle<N, T, 1, 3, 3> yww;

        Swizzle<N, T, 2, 0, 0> zxx;
        Swizzle<N, T, 2, 0, 1> zxy;
        Swizzle<N, T, 2, 0, 2> zxz;
        Swizzle<N, T, 2, 0, 3> zxw;

        Swizzle<N, T, 2, 1, 0> zyx;
        Swizzle<N, T, 2, 1, 1> zyy;
        Swizzle<N, T, 2, 1, 2> zyz;
        Swizzle<N, T, 2, 1, 3> zyw;

        Swizzle<N, T, 2, 2, 0> zzx;
        Swizzle<N, T, 2, 2, 1> zzy;
        Swizzle<N, T, 2, 2, 2> zzz;
        Swizzle<N, T, 2, 2, 3> zzw;

        Swizzle<N, T, 2, 3, 0> zwx;
        Swizzle<N, T, 2, 3, 1> zwy;
        Swizzle<N, T, 2, 3, 2> zwz;
        Swizzle<N, T, 2, 3, 3> zww;

        Swizzle<N, T, 3, 0, 0> wxx;
        Swizzle<N, T, 3, 0, 1> wxy;
        Swizzle<N, T, 3, 0, 2> wxz;
        Swizzle<N, T, 3, 0, 3> wxw;

        Swizzle<N, T, 3, 1, 0> wyx;
        Swizzle<N, T, 3, 1, 1> wyy;
        Swizzle<N, T, 3, 1, 2> wyz;
        Swizzle<N, T, 3, 1, 3> wyw;

        Swizzle<N, T, 3, 2, 0> wzx;
        Swizzle<N, T, 3, 2, 1> wzy;
        Swizzle<N, T, 3, 2, 2> wzz;
        Swizzle<N, T, 3, 2, 3> wzw;

        Swizzle<N, T, 3, 3, 0> wwx;
        Swizzle<N, T, 3, 3, 1> wwy;
        Swizzle<N, T, 3, 3, 2> wwz;
        Swizzle<N, T, 3, 3, 3> www;

        Swizzle<N, T, 0, 0, 0, 0> xxxx;
        Swizzle<N, T, 0, 0, 0, 1> xxxy;
        Swizzle<N, T, 0, 0, 0, 2> xxxz;
        Swizzle<N, T, 0, 0, 0, 3> xxxw;

        Swizzle<N, T, 0, 0, 1, 0> xxyx;
        Swizzle<N, T, 0, 0, 1, 1> xxyy;
        Swizzle<N, T, 0, 0, 1, 2> xxyz;
        Swizzle<N, T, 0, 0, 1, 3> xxyw;

        Swizzle<N, T, 0, 0, 2, 0> xxzx;
        Swizzle<N, T, 0, 0, 2, 1> xxzy;
        Swizzle<N, T, 0, 0, 2, 2> xxzz;
        Swizzle<N, T, 0, 0, 2, 3> xxzw;

        Swizzle<N, T, 0, 0, 3, 0> xxwx;
        Swizzle<N, T, 0, 0, 3, 1> xxwy;
        Swizzle<N, T, 0, 0, 3, 2> xxwz;
        Swizzle<N, T, 0, 0, 3, 3> xxww;

        Swizzle<N, T, 0, 1, 0, 0> xyxx;
        Swizzle<N, T, 0, 1, 0, 1> xyxy;
        Swizzle<N, T, 0, 1, 0, 2> xyxz;
        Swizzle<N, T, 0, 1, 0, 3> xyxw;

        Swizzle<N, T, 0, 1, 1, 0> xyyx;
        Swizzle<N, T, 0, 1, 1, 1> xyyy;
        Swizzle<N, T, 0, 1, 1, 2> xyyz;
        Swizzle<N, T, 0, 1, 1, 3> xyyw;

        Swizzle<N, T, 0, 1, 2, 0> xyzx;
        Swizzle<N, T, 0, 1, 2, 1> xyzy;
        Swizzle<N, T, 0, 1, 2, 2> xyzz;
        Swizzle<N, T, 0, 1, 2, 3> xyzw;

        Swizzle<N, T, 0, 1, 3, 0> xywx;
        Swizzle<N, T, 0, 1, 3, 1> xywy;
        Swizzle<N, T, 0, 1, 3, 2> xywz;
        Swizzle<N, T, 0, 1, 3, 3> xyww;

        Swizzle<N, T, 0, 2, 2, 0> xzzx;
        Swizzle<N, T, 0, 2, 2, 1> xzzy;
        Swizzle<N, T, 0, 2, 2, 2> xzzz;
        Swizzle<N, T, 0, 2, 2, 3> xzzw;

        Swizzle<N, T, 0, 2, 3, 0> xzwx;
        Swizzle<N, T, 0, 2, 3, 1> xzwy;
        Swizzle<N, T, 0, 2, 3, 2> xzwz;
        Swizzle<N, T, 0, 2, 3, 3> xzww;

        Swizzle<N, T, 0, 3, 2, 0> xwzx;
        Swizzle<N, T, 0, 3, 2, 1> xwzy;
        Swizzle<N, T, 0, 3, 2, 2> xwzz;
        Swizzle<N, T, 0, 3, 2, 3> xwzw;

        Swizzle<N, T, 0, 3, 3, 0> xwwx;
        Swizzle<N, T, 0, 3, 3, 1> xwwy;
        Swizzle<N, T, 0, 3, 3, 2> xwwz;
        Swizzle<N, T, 0, 3, 3, 3> xwww;

        Swizzle<N, T, 1, 0, 0, 0> yxxx;
        Swizzle<N, T, 1, 0, 0, 1> yxxy;
        Swizzle<N, T, 1, 0, 0, 2> yxxz;
        Swizzle<N, T, 1, 0, 0, 3> yxxw;

        Swizzle<N, T, 1, 0, 1, 0> yxyx;
        Swizzle<N, T, 1, 0, 1, 1> yxyy;
        Swizzle<N, T, 1, 0, 1, 2> yxyz;
        Swizzle<N, T, 1, 0, 1, 3> yxyw;

        Swizzle<N, T, 1, 0, 2, 0> yxzx;
        Swizzle<N, T, 1, 0, 2, 1> yxzy;
        Swizzle<N, T, 1, 0, 2, 2> yxzz;
        Swizzle<N, T, 1, 0, 2, 3> yxzw;

        Swizzle<N, T, 1, 0, 3, 0> yxwx;
        Swizzle<N, T, 1, 0, 3, 1> yxwy;
        Swizzle<N, T, 1, 0, 3, 2> yxwz;
        Swizzle<N, T, 1, 0, 3, 3> yxww;

        Swizzle<N, T, 1, 1, 0, 0> yyxx;
        Swizzle<N, T, 1, 1, 0, 1> yyxy;
        Swizzle<N, T, 1, 1, 0, 2> yyxz;
        Swizzle<N, T, 1, 1, 0, 3> yyxw;

        Swizzle<N, T, 1, 1, 1, 0> yyyx;
        Swizzle<N, T, 1, 1, 1, 1> yyyy;
        Swizzle<N, T, 1, 1, 1, 2> yyyz;
        Swizzle<N, T, 1, 1, 1, 3> yyyw;

        Swizzle<N, T, 1, 1, 2, 0> yyzx;
        Swizzle<N, T, 1, 1, 2, 1> yyzy;
        Swizzle<N, T, 1, 1, 2, 2> yyzz;
        Swizzle<N, T, 1, 1, 2, 3> yyzw;

        Swizzle<N, T, 1, 1, 3, 0> yywx;
        Swizzle<N, T, 1, 1, 3, 1> yywy;
        Swizzle<N, T, 1, 1, 3, 2> yywz;
        Swizzle<N, T, 1, 1, 3, 3> yyww;

        Swizzle<N, T, 1, 2, 2, 0> yzzx;
        Swizzle<N, T, 1, 2, 2, 1> yzzy;
        Swizzle<N, T, 1, 2, 2, 2> yzzz;
        Swizzle<N, T, 1, 2, 2, 3> yzzw;

        Swizzle<N, T, 1, 2, 3, 0> yzwx;
        Swizzle<N, T, 1, 2, 3, 1> yzwy;
        Swizzle<N, T, 1, 2, 3, 2> yzwz;
        Swizzle<N, T, 1, 2, 3, 3> yzww;

        Swizzle<N, T, 1, 3, 2, 0> ywzx;
        Swizzle<N, T, 1, 3, 2, 1> ywzy;
        Swizzle<N, T, 1, 3, 2, 2> ywzz;
        Swizzle<N, T, 1, 3, 2, 3> ywzw;

        Swizzle<N, T, 1, 3, 3, 0> ywwx;
        Swizzle<N, T, 1, 3, 3, 1> ywwy;
        Swizzle<N, T, 1, 3, 3, 2> ywwz;
        Swizzle<N, T, 1, 3, 3, 3> ywww;

        Swizzle<N, T, 2, 0, 0, 0> zxxx;
        Swizzle<N, T, 2, 0, 0, 1> zxxy;
        Swizzle<N, T, 2, 0, 0, 2> zxxz;
        Swizzle<N, T, 2, 0, 0, 3> zxxw;

        Swizzle<N, T, 2, 0, 1, 0> zxyx;
        Swizzle<N, T, 2, 0, 1, 1> zxyy;
        Swizzle<N, T, 2, 0, 1, 2> zxyz;
        Swizzle<N, T, 2, 0, 1, 3> zxyw;

        Swizzle<N, T, 2, 0, 2, 0> zxzx;
        Swizzle<N, T, 2, 0, 2, 1> zxzy;
        Swizzle<N, T, 2, 0, 2, 2> zxzz;
        Swizzle<N, T, 2, 0, 2, 3> zxzw;

        Swizzle<N, T, 2, 0, 3, 0> zxwx;
        Swizzle<N, T, 2, 0, 3, 1> zxwy;
        Swizzle<N, T, 2, 0, 3, 2> zxwz;
        Swizzle<N, T, 2, 0, 3, 3> zxww;

        Swizzle<N, T, 2, 1, 0, 0> zyxx;
        Swizzle<N, T, 2, 1, 0, 1> zyxy;
        Swizzle<N, T, 2, 1, 0, 2> zyxz;
        Swizzle<N, T, 2, 1, 0, 3> zyxw;

        Swizzle<N, T, 2, 1, 1, 0> zyyx;
        Swizzle<N, T, 2, 1, 1, 1> zyyy;
        Swizzle<N, T, 2, 1, 1, 2> zyyz;
        Swizzle<N, T, 2, 1, 1, 3> zyyw;

        Swizzle<N, T, 2, 1, 2, 0> zyzx;
        Swizzle<N, T, 2, 1, 2, 1> zyzy;
        Swizzle<N, T, 2, 1, 2, 2> zyzz;
        Swizzle<N, T, 2, 1, 2, 3> zyzw;

        Swizzle<N, T, 2, 1, 3, 0> zywx;
        Swizzle<N, T, 2, 1, 3, 1> zywy;
        Swizzle<N, T, 2, 1, 3, 2> zywz;
        Swizzle<N, T, 2, 1, 3, 3> zyww;

        Swizzle<N, T, 2, 2, 2, 0> zzzx;
        Swizzle<N, T, 2, 2, 2, 1> zzzy;
        Swizzle<N, T, 2, 2, 2, 2> zzzz;
        Swizzle<N, T, 2, 2, 2, 3> zzzw;

        Swizzle<N, T, 2, 2, 3, 0> zzwx;
        Swizzle<N, T, 2, 2, 3, 1> zzwy;
        Swizzle<N, T, 2, 2, 3, 2> zzwz;
        Swizzle<N, T, 2, 2, 3, 3> zzww;

        Swizzle<N, T, 2, 3, 2, 0> zwzx;
        Swizzle<N, T, 2, 3, 2, 1> zwzy;
        Swizzle<N, T, 2, 3, 2, 2> zwzz;
        Swizzle<N, T, 2, 3, 2, 3> zwzw;

        Swizzle<N, T, 2, 3, 3, 0> zwwx;
        Swizzle<N, T, 2, 3, 3, 1> zwwy;
        Swizzle<N, T, 2, 3, 3, 2> zwwz;
        Swizzle<N, T, 2, 3, 3, 3> zwww;

        Swizzle<N, T, 3, 0, 0, 0> wxxx;
        Swizzle<N, T, 3, 0, 0, 1> wxxy;
        Swizzle<N, T, 3, 0, 0, 2> wxxz;
        Swizzle<N, T, 3, 0, 0, 3> wxxw;

        Swizzle<N, T, 3, 0, 1, 0> wxyx;
        Swizzle<N, T, 3, 0, 1, 1> wxyy;
        Swizzle<N, T, 3, 0, 1, 2> wxyz;
        Swizzle<N, T, 3, 0, 1, 3> wxyw;

        Swizzle<N, T, 3, 0, 2, 0> wxzx;
        Swizzle<N, T, 3, 0, 2, 1> wxzy;
        Swizzle<N, T, 3, 0, 2, 2> wxzz;
        Swizzle<N, T, 3, 0, 2, 3> wxzw;

        Swizzle<N, T, 3, 0, 3, 0> wxwx;
        Swizzle<N, T, 3, 0, 3, 1> wxwy;
        Swizzle<N, T, 3, 0, 3, 2> wxwz;
        Swizzle<N, T, 3, 0, 3, 3> wxww;

        Swizzle<N, T, 3, 1, 0, 0> wyxx;
        Swizzle<N, T, 3, 1, 0, 1> wyxy;
        Swizzle<N, T, 3, 1, 0, 2> wyxz;
        Swizzle<N, T, 3, 1, 0, 3> wyxw;

        Swizzle<N, T, 3, 1, 1, 0> wyyx;
        Swizzle<N, T, 3, 1, 1, 1> wyyy;
        Swizzle<N, T, 3, 1, 1, 2> wyyz;
        Swizzle<N, T, 3, 1, 1, 3> wyyw;

        Swizzle<N, T, 3, 1, 2, 0> wyzx;
        Swizzle<N, T, 3, 1, 2, 1> wyzy;
        Swizzle<N, T, 3, 1, 2, 2> wyzz;
        Swizzle<N, T, 3, 1, 2, 3> wyzw;
        Swizzle<N, T, 3, 1, 3, 0> wywx;

        Swizzle<N, T, 3, 1, 3, 1> wywy;
        Swizzle<N, T, 3, 1, 3, 2> wywz;
        Swizzle<N, T, 3, 1, 3, 3> wyww;

        Swizzle<N, T, 3, 2, 2, 0> wzzx;
        Swizzle<N, T, 3, 2, 2, 1> wzzy;
        Swizzle<N, T, 3, 2, 2, 2> wzzz;
        Swizzle<N, T, 3, 2, 2, 3> wzzw;

        Swizzle<N, T, 3, 2, 3, 0> wzwx;
        Swizzle<N, T, 3, 2, 3, 1> wzwy;
        Swizzle<N, T, 3, 2, 3, 2> wzwz;
        Swizzle<N, T, 3, 2, 3, 3> wzww;

        Swizzle<N, T, 3, 3, 2, 0> wwzx;
        Swizzle<N, T, 3, 3, 2, 1> wwzy;
        Swizzle<N, T, 3, 3, 2, 2> wwzz;
        Swizzle<N, T, 3, 3, 2, 3> wwzw;

        Swizzle<N, T, 3, 3, 3, 0> wwwx;
        Swizzle<N, T, 3, 3, 3, 1> wwwy;
        Swizzle<N, T, 3, 3, 3, 2> wwwz;
        Swizzle<N, T, 3, 3, 3, 3> wwww;
    };
};

} // namespace asciirast::math
