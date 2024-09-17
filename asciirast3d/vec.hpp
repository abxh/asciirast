#pragma once

#include <array>
#include <stdexcept>

namespace asciirast3d {

template <std::size_t N, typename T>
    requires(std::is_arithmetic_v<T>)
struct Vec;

typedef Vec<1, float> Vec1f;
typedef Vec<2, float> Vec2f;
typedef Vec<3, float> Vec3f;

typedef Vec<1, int> Vec1i;
typedef Vec<2, int> Vec2i;
typedef Vec<3, int> Vec3i;

template <template <std::size_t, typename> typename V, typename T, std::size_t... indicies>
struct VecSwizzle {
    // inspiration:
    // https://kiorisyshen.github.io/2018/08/27/Vector%20Swizzling%20and%20Parameter%20Pack%20in%20C++/

private:
    static constexpr auto N = (sizeof...(indicies));
    T v[N];

public:
    V<N, T>& operator=(const V<N, T>& rhs)
    {
        std::size_t indexes[] = {indicies...};
        for (std::size_t i = 0; i < N; i++) {
            v[indexes[i]] = rhs[i];
        }
        return *(V<N, T>*)this;
    }
    V<N, T> as_vec() const
    {
        return V<N, T>{v[indicies]...};
    }
    operator V<N, T>() const
    {
        return as_vec();
    }
};

template <std::size_t N, typename T>
    requires(std::is_arithmetic_v<T>)
struct Vec {
    typedef Vec<N, T> V;

public:
    std::array<T, N> e;

public:
    Vec()
        : e{{0}} {};
    Vec(const std::array<T, N> arr)
        : e{arr} {};
    template <typename... Ts>
        requires(std::conjunction_v<std::is_same<T, Ts>...>)
    Vec(const Ts... values)
        : e{values...} {};

    T operator[](size_t i) const
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
    T& operator[](size_t i)
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
};

template <typename T>
struct Vec<1, T> {
    static constexpr auto N = 1;
    typedef Vec<N, T> V;

public:
    union {
        std::array<T, N> e;
        T x;
    };

public:
    Vec()
        : e{{0}} {};
    Vec(const std::array<T, N> arr)
        : e{arr} {};
    template <typename... Ts>
        requires(std::conjunction_v<std::is_same<T, Ts>...>)
    Vec(const Ts... values)
        : e{values...} {};

    T operator[](size_t i) const
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
    T& operator[](size_t i)
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
};

template <typename T>
struct Vec<2, T> {
    static constexpr auto N = 2;
    typedef Vec<N, T> V;

public:
    union {
        std::array<T, N> e;
        struct {
            T x;
            T y;
        };
        VecSwizzle<Vec, T, 0, 0> xx;
        VecSwizzle<Vec, T, 0, 1> xy;
        VecSwizzle<Vec, T, 1, 0> yx;
        VecSwizzle<Vec, T, 1, 1> yy;
    };

public:
    Vec()
        : e{{0}} {};
    Vec(const std::array<T, N> arr)
        : e{arr} {};
    template <typename... Ts>
        requires(std::conjunction_v<std::is_same<T, Ts>...>)
    Vec(const Ts... values)
        : e{values...} {};

    T operator[](size_t i) const
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
    T& operator[](size_t i)
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
};

template <typename T>
struct Vec<3, T> {
    static constexpr auto N = 3;
    typedef Vec<N, T> V;

public:
    union {
        std::array<T, N> e;
        struct {
            T x;
            T y;
            T z;
        };
        VecSwizzle<Vec, T, 0, 0> xx;
        VecSwizzle<Vec, T, 0, 1> xy;
        VecSwizzle<Vec, T, 0, 2> xz;

        VecSwizzle<Vec, T, 1, 0> yx;
        VecSwizzle<Vec, T, 1, 1> yy;
        VecSwizzle<Vec, T, 1, 2> yz;

        VecSwizzle<Vec, T, 2, 0> zx;
        VecSwizzle<Vec, T, 2, 1> zy;
        VecSwizzle<Vec, T, 2, 2> zz;

        VecSwizzle<Vec, T, 0, 0, 0> xxx;
        VecSwizzle<Vec, T, 0, 0, 1> xxy;
        VecSwizzle<Vec, T, 0, 0, 2> xxz;
        VecSwizzle<Vec, T, 0, 1, 0> xyx;
        VecSwizzle<Vec, T, 0, 1, 1> xyy;
        VecSwizzle<Vec, T, 0, 1, 2> xyz;
        VecSwizzle<Vec, T, 0, 2, 0> xzx;
        VecSwizzle<Vec, T, 0, 2, 1> xzy;
        VecSwizzle<Vec, T, 0, 2, 2> xzz;

        VecSwizzle<Vec, T, 1, 0, 0> yxx;
        VecSwizzle<Vec, T, 1, 0, 1> yxy;
        VecSwizzle<Vec, T, 1, 0, 2> yxz;
        VecSwizzle<Vec, T, 1, 1, 0> yyx;
        VecSwizzle<Vec, T, 1, 1, 1> yyy;
        VecSwizzle<Vec, T, 1, 1, 2> yyz;
        VecSwizzle<Vec, T, 1, 2, 0> yzx;
        VecSwizzle<Vec, T, 1, 2, 1> yzy;
        VecSwizzle<Vec, T, 1, 2, 2> yzz;

        VecSwizzle<Vec, T, 2, 0, 0> zxx;
        VecSwizzle<Vec, T, 2, 0, 1> zxy;
        VecSwizzle<Vec, T, 2, 0, 2> zxz;
        VecSwizzle<Vec, T, 2, 1, 0> zyx;
        VecSwizzle<Vec, T, 2, 1, 1> zyy;
        VecSwizzle<Vec, T, 2, 1, 2> zyz;
        VecSwizzle<Vec, T, 2, 2, 0> zzx;
        VecSwizzle<Vec, T, 2, 2, 1> zzy;
        VecSwizzle<Vec, T, 2, 2, 2> zzz;
    };

public:
    Vec()
        : e{{0}} {};
    Vec(const std::array<T, N> arr)
        : e{arr} {};
    template <typename... Ts>
        requires(std::conjunction_v<std::is_same<T, Ts>...>)
    Vec(const Ts... values)
        : e{values...} {};

    T operator[](size_t i) const
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
    T& operator[](size_t i)
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
};

template <typename T>
struct Vec<4, T> {
    static constexpr auto N = 4;
    typedef Vec<N, T> V;

public:
    union {
        std::array<T, N> e;
        struct {
            T x;
            T y;
            T z;
            T w;
        };
        VecSwizzle<Vec, T, 0, 0> xx;
        VecSwizzle<Vec, T, 0, 1> xy;
        VecSwizzle<Vec, T, 0, 2> xz;
        VecSwizzle<Vec, T, 0, 3> xw;

        VecSwizzle<Vec, T, 1, 0> yx;
        VecSwizzle<Vec, T, 1, 1> yy;
        VecSwizzle<Vec, T, 1, 2> yz;
        VecSwizzle<Vec, T, 1, 3> yw;

        VecSwizzle<Vec, T, 2, 0> zx;
        VecSwizzle<Vec, T, 2, 1> zy;
        VecSwizzle<Vec, T, 2, 2> zz;
        VecSwizzle<Vec, T, 2, 3> zw;

        VecSwizzle<Vec, T, 3, 0> wx;
        VecSwizzle<Vec, T, 3, 1> wy;
        VecSwizzle<Vec, T, 3, 2> wz;
        VecSwizzle<Vec, T, 3, 3> ww;

        VecSwizzle<Vec, T, 0, 0, 0> xxx;
        VecSwizzle<Vec, T, 0, 0, 1> xxy;
        VecSwizzle<Vec, T, 0, 0, 2> xxz;
        VecSwizzle<Vec, T, 0, 0, 3> xxw;
        VecSwizzle<Vec, T, 0, 1, 0> xyx;
        VecSwizzle<Vec, T, 0, 1, 1> xyy;
        VecSwizzle<Vec, T, 0, 1, 2> xyz;
        VecSwizzle<Vec, T, 0, 1, 3> xyw;
        VecSwizzle<Vec, T, 0, 2, 0> xzx;
        VecSwizzle<Vec, T, 0, 2, 1> xzy;
        VecSwizzle<Vec, T, 0, 2, 2> xzz;
        VecSwizzle<Vec, T, 0, 2, 3> xzw;
        VecSwizzle<Vec, T, 0, 3, 0> xwx;
        VecSwizzle<Vec, T, 0, 3, 1> xwy;
        VecSwizzle<Vec, T, 0, 3, 2> xwz;
        VecSwizzle<Vec, T, 0, 3, 3> xww;

        VecSwizzle<Vec, T, 1, 0, 0> yxx;
        VecSwizzle<Vec, T, 1, 0, 1> yxy;
        VecSwizzle<Vec, T, 1, 0, 2> yxz;
        VecSwizzle<Vec, T, 1, 0, 3> yxw;
        VecSwizzle<Vec, T, 1, 1, 0> yyx;
        VecSwizzle<Vec, T, 1, 1, 1> yyy;
        VecSwizzle<Vec, T, 1, 1, 2> yyz;
        VecSwizzle<Vec, T, 1, 1, 3> yyw;
        VecSwizzle<Vec, T, 1, 2, 0> yzx;
        VecSwizzle<Vec, T, 1, 2, 1> yzy;
        VecSwizzle<Vec, T, 1, 2, 2> yzz;
        VecSwizzle<Vec, T, 1, 2, 3> yzw;
        VecSwizzle<Vec, T, 1, 3, 0> ywx;
        VecSwizzle<Vec, T, 1, 3, 1> ywy;
        VecSwizzle<Vec, T, 1, 3, 2> ywz;
        VecSwizzle<Vec, T, 1, 3, 3> yww;

        VecSwizzle<Vec, T, 2, 0, 0> zxx;
        VecSwizzle<Vec, T, 2, 0, 1> zxy;
        VecSwizzle<Vec, T, 2, 0, 2> zxz;
        VecSwizzle<Vec, T, 2, 0, 3> zxw;
        VecSwizzle<Vec, T, 2, 1, 0> zyx;
        VecSwizzle<Vec, T, 2, 1, 1> zyy;
        VecSwizzle<Vec, T, 2, 1, 2> zyz;
        VecSwizzle<Vec, T, 2, 1, 3> zyw;
        VecSwizzle<Vec, T, 2, 2, 0> zzx;
        VecSwizzle<Vec, T, 2, 2, 1> zzy;
        VecSwizzle<Vec, T, 2, 2, 2> zzz;
        VecSwizzle<Vec, T, 2, 2, 3> zzw;
        VecSwizzle<Vec, T, 2, 3, 0> zwx;
        VecSwizzle<Vec, T, 2, 3, 1> zwy;
        VecSwizzle<Vec, T, 2, 3, 2> zwz;
        VecSwizzle<Vec, T, 2, 3, 3> zww;

        VecSwizzle<Vec, T, 3, 0, 0> wxx;
        VecSwizzle<Vec, T, 3, 0, 1> wxy;
        VecSwizzle<Vec, T, 3, 0, 2> wxz;
        VecSwizzle<Vec, T, 3, 0, 3> wxw;
        VecSwizzle<Vec, T, 3, 1, 0> wyx;
        VecSwizzle<Vec, T, 3, 1, 1> wyy;
        VecSwizzle<Vec, T, 3, 1, 2> wyz;
        VecSwizzle<Vec, T, 3, 1, 3> wyw;
        VecSwizzle<Vec, T, 3, 2, 0> wzx;
        VecSwizzle<Vec, T, 3, 2, 1> wzy;
        VecSwizzle<Vec, T, 3, 2, 2> wzz;
        VecSwizzle<Vec, T, 3, 2, 3> wzw;
        VecSwizzle<Vec, T, 3, 3, 0> wwx;
        VecSwizzle<Vec, T, 3, 3, 1> wwy;
        VecSwizzle<Vec, T, 3, 3, 2> wwz;
        VecSwizzle<Vec, T, 3, 3, 3> www;
    };

public:
    Vec()
        : e{{0}} {};
    Vec(const std::array<T, N> arr)
        : e{arr} {};
    template <typename... Ts>
        requires(std::conjunction_v<std::is_same<T, Ts>...>)
    Vec(const Ts... values)
        : e{values...} {};

    T operator[](size_t i) const
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
    T& operator[](size_t i)
    {
        if (i >= N) {
            throw std::out_of_range("index out of bounds");
        }
        return e[i];
    }
};

template <std::size_t N, typename T>
inline Vec<N, T> operator+(Vec<N, T> lhs, Vec<N, T> rhs)
{
    Vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = lhs[i] + rhs[i];
    }
    return res;
}

template <std::size_t N, typename T>
inline Vec<N, T> operator-(Vec<N, T> lhs, Vec<N, T> rhs)
{
    Vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = lhs[i] - rhs[i];
    }
    return res;
}

template <std::size_t N, typename T>
inline Vec<N, T> operator*(T scalar, Vec<N, T> rhs)
{
    Vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = scalar * rhs[i];
    }
    return res;
}

template <std::size_t N, typename T>
inline Vec<N, T> operator*(Vec<N, T> lhs, T scalar)
{
    Vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = lhs[i] * scalar;
    }
    return res;
}

template <std::size_t N, typename T>
inline Vec<N, T> operator/(Vec<N, T> lhs, T scalar)
{
    Vec<N, T> res;
    for (size_t i = 0; i < N; i++) {
        res[i] = lhs[i] / scalar;
    }
    return res;
}

} // namespace asciirast3d
