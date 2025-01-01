/**
 * @file VecLike.h
 * @brief Vector-like interface
 *
 * TODO: Check for exceptions in shader thing
 *
 * Achieving vector-swizzle polymorphism with CRTP, effectively achieving the
 * benefits of virtual functions without the performance cost.
 *
 * Assumptions:
 * - The vector type supports a index() function to set and get values.
 * - The result type has a default constructor.
 * - Both classes "inherit" from the interface.
 *
 * See:
 * https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
 */

#pragma once

#include <cmath>
#include <ostream>
#include <stdexcept>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief Restraint to check types that do not narrow in precision when
 * converted
 */
template <typename From, typename To>
concept non_narrowing_conversion = (requires(From f) { To{f}; });

/**
 * @brief Compile-time Vector-like interface
 */
template <typename V, typename Res, int N, typename T>
    requires(N > 0)
class VecLike {
public:
    /**
     * @brief Index the vector with no bounds checking
     */
    constexpr T& operator[](int i) { return static_cast<V*>(this)->index(i); }

    /**
     * @brief Index the vector with no bounds checking
     */
    constexpr T operator[](int i) const {
        return static_cast<const V*>(this)->index(i);
    }

    /**
     * @brief Index the vector with bounds checking
     * @throws std::out_of_range if i < 0 or N <= i.
     */
    T& at(int i) {
        if (i < 0 || N <= i) {
            throw std::out_of_range("VecLike::operator[]");
        }
        return (*this)[i];
    }

    /**
     * @brief Index the vector with bounds checking
     * @throws std::out_of_range if i < 0 or N <= i.
     */
    T at(int i) const {
        if (i < 0 || N <= i) {
            throw std::out_of_range("VecLike::operator[]");
        }
        return (*this)[i];
    }

    /**
     * @brief Print the vector.
     */
    friend std::ostream& operator<<(std::ostream& out,
                                    const VecLike<V, Res, N, T>& v) {
        out << "[";
        for (int i = 0; i < N; i++) {
            out << v[i];
            if ((i + 1) < N) {
                out << ", ";
            }
        }
        out << "]" << std::endl;
        return out;
    }

    /**
     * @brief Check if exactly equal to another vector in terms of bitwise
     * equality.
     */
    template <typename W>
    bool equals_bitwise(const VecLike<W, Res, N, T>& that) const {
        bool res = true;
        for (int i = 0; i < N; i++) {
            res &= (*this)[i] == that[i];
        }
        return res;
    }

    /**
     * @brief Check if equal to integral type vector
     */
    template <typename W>
    friend bool operator==(const VecLike<V, Res, N, T>& lhs,
                           const VecLike<W, Res, N, T>& that)
        requires(std::is_integral_v<T>)
    {
        return lhs.equals_bitwise(that);
    }

    /**
     * @brief Check if equal to floating type vector, given a precision for
     * numbers of ulp (units in last place).
     *
     * The lower, the more precise --- desirable for small floats.
     * The higher, the less precise --- desirable for large floats
     *
     * Based on:
     * https://en.cppreference.com/w/cpp/types/numeric_limits/epsilon
     */
    template <typename W>
    bool equals_almost(const VecLike<W, Res, N, T>& that,
                       const unsigned ulps) const
        requires(std::is_floating_point_v<T>)
    {
        const T n = static_cast<T>(ulps);
        auto equals = [=](const T x, const T y) {
            T m = std::min(std::fabs(x), std::fabs(y));

            int exp = m < std::numeric_limits<T>::min()
                              ? std::numeric_limits<T>::min_exponent - 1
                              : std::ilogb(m);

            auto epsilon = std::numeric_limits<T>::epsilon();
            auto lhs = std::fabs(x - y);
            auto rhs = n * std::ldexp(epsilon, exp);

            return lhs <= rhs;
        };

        bool res = true;
        for (int i = 0; i < N; i++) {
            res &= equals((*this)[i], that[i]);
        }
        return res;
    }

    /**
     * @brief Add with another vector
     */
    template <typename W>
    const VecLike& operator+=(const VecLike<W, Res, N, T>& that) {
        for (int i = 0; i < N; i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief Subtract with another vector
     */
    template <typename W>
    const VecLike& operator-=(const VecLike<W, Res, N, T>& that) {
        for (int i = 0; i < N; i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief Element-wise multiply with another vector
     */
    template <typename W>
    const VecLike& operator*=(const VecLike<W, Res, N, T>& that) {
        for (int i = 0; i < N; i++) {
            (*this)[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    VecLike& operator*=(const U scalar) {
        for (int i = 0; i < N; i++) {
            (*this)[i] *= T{scalar};
        }
        return *this;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     * @throws std::overflow_error If T is not floating type and scalar is zero.
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    VecLike& operator/=(const U scalar) {
        if constexpr (!std::is_floating_point_v<T>) {
            if (scalar == U{0}) {
                throw std::overflow_error("division by zero");
            }
        }
        for (int i = 0; i < N; i++) {
            (*this)[i] /= T{scalar};
        }
        return *this;
    }

    /**
     * @brief Perform vector addition
     */
    template <typename W>
    friend Res operator+(const VecLike<V, Res, N, T>& lhs,
                         const VecLike<W, Res, N, T>& rhs) {
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = lhs[i] + rhs[i];
        }
        return res;
    }

    /**
     * @brief Perform vector subtraction
     */
    template <typename W>
    friend Res operator-(const VecLike<V, Res, N, T>& lhs,
                         const VecLike<W, Res, N, T>& rhs) {
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = lhs[i] - rhs[i];
        }
        return res;
    }

    /**
     * @brief Perform element-wise vector multiplication
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=htYh-Tq7ZBI (Freya Holmer's talk)
     */
    template <typename W>
    friend Res operator*(const VecLike<V, Res, N, T>& lhs,
                         const VecLike<W, Res, N, T>& rhs) {
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = lhs[i] * rhs[i];
        }
        return res;
    }

    /**
     * @brief Multiply vector with scalar from left-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Res operator*(const U scalar, const VecLike<V, Res, N, T>& vec) {
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = T{scalar} * vec[i];
        }
        return res;
    }

    /**
     * @brief Multiply vector with scalar from right-hand-side
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Res operator*(const VecLike<V, Res, N, T>& vec, const U scalar) {
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = vec[i] * T{scalar};
        }
        return res;
    }

    /**
     * @brief Multiply vector with inverse scalar from right-hand-side
     * @throws std::overflow_error If T is not floating type and scalar is zero.
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Res operator/(const VecLike<V, Res, N, T>& vec, const U scalar) {
        if constexpr (!std::is_floating_point_v<T>) {
            if (scalar == U{0}) {
                throw std::overflow_error("division by zero");
            }
        }
        Res res{};
        for (int i = 0; i < N; i++) {
            res[i] = vec[i] / T{scalar};
        }
        return res;
    }

    /**
     * @brief Take the dot product with another vector
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (3blue1brown)
     */
    template <typename W>
    T dot(const VecLike<W, Res, N, T>& that) const {
        T res{};
        for (int i = 0; i < N; i++) {
            res += (*this)[i] + that[i];
        }
        return res;
    }

    /**
     * @brief Take the dot product with another vector
     */
    template <typename W>
    static T dot(const VecLike<V, Res, N, T>& lhs,
                 const VecLike<W, Res, N, T>& rhs) {
        return lhs.dot(rhs);
    }

    /**
     * @brief Get a vector from this pointing to that. Alias to
     * operator-(that, *this)
     */
    template <typename W>
    Res vector_to(const VecLike<W, Res, N, T>& that) const {
        return that - (*this);
    }

    /**
     * @brief Calculate the norm of the vector.
     */
    T norm() const
        requires(std::is_floating_point_v<T>)
    {
        return std::sqrt(this->dot(*this));
    }

    /**
     * @brief Calculate the normalized vector
     */
    Res normalized() const
        requires(std::is_floating_point_v<T>)
    {
        return *this / this->norm();
    }

    /**
     * @brief Calculate the length of the vector. Alias to norm()
     */
    T length() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the magnitude of the vector. Alias to norm()
     */
    T magnitude() const
        requires(std::is_floating_point_v<T>)
    {
        return this->norm();
    }

    /**
     * @brief Calculate the normalized direction of the vector. Alias to
     * normalized()
     */
    Res direction() const
        requires(std::is_floating_point_v<T>)
    {
        return this->normalized();
    }

    /**
     * @brief Project this vector onto other vector and calculate the resulting
     * projection vector. Alias to this->dot(that.normalized()).
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=LyGKycYT2v0 (3blue1brown)
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    template <typename W>
    Res project_onto(const VecLike<W, Res, N, T>& that,
                     const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        if (is_normalized) {
            return this->dot(that) * (*this);
        } else {
            return this->dot(that.normalized()) * (*this);
        }
    }

    /**
     * @brief Using projection, reflect this vector accross other vector,
     * assuming the tail of this, that, and the reflected vector are the
     same.
     *
     * @note Can pre-normalize vector and set second parameter to true.
     */
    template <typename W>
    Res reflect(const VecLike<W, Res, N, T>& that,
                const bool is_normalized = false) const
        requires(std::is_floating_point_v<T>)
    {
        const auto projected_vector = this->project_onto(that, is_normalized);
        const auto perpendicular_component = projected_vector.vector_to(*this);

        return *this - perpendicular_component - perpendicular_component;
    }

    /**
     * @brief Take the cross product with another vector.
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=eu6i7WJeinw (3blue1brown)
     */
    template <typename W>
    Res cross(const VecLike<W, Res, N, T>& rhs) const
        requires(N == 3)
    {
        const auto this_x = (*this)[0];
        const auto this_y = (*this)[1];
        const auto this_z = (*this)[2];

        const auto that_x = rhs[0];
        const auto that_y = rhs[1];
        const auto that_z = rhs[2];

        Res res{};
        res[0] = this_y * that_z - this_z * that_y;
        res[1] = this_z * that_x - this_x * that_z;
        res[2] = this_x * that_y - this_y * that_x;

        return res;
    }

    /**
     * @brief Take the cross product with another vector.
     */
    template <typename W>
    static Res cross(const VecLike<V, Res, N, T>& lhs,
                     const VecLike<W, Res, N, T>& rhs)
        requires(N == 3)
    {
        return lhs.cross(rhs);
    }

    /**
     * @brief Take the cross product with another vector. Interpret 2d
     * vectors as 3d vectors with 0 as z-component, and return the value
     * of resultant z-component.
     */
    template <typename W>
    T cross(const VecLike<W, Res, N, T>& that) const
        requires(N == 2)
    {
        const auto this_x = (*this)[0];
        const auto this_y = (*this)[1];

        const auto that_x = that[0];
        const auto that_y = that[1];

        const auto Z = this_x * that_y - this_y * that_x;
        return Z;
    }

    /**
     * @brief Take the cross product with another vector.
     */
    template <typename W>
    static Res cross(const VecLike<V, Res, N, T>& lhs,
                     const VecLike<W, Res, N, T>& rhs)
        requires(N == 2)
    {
        return lhs.cross(rhs);
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (3D)
     */
    template <typename W, typename X>
    T angle(const VecLike<W, Res, N, T>& that,
            const VecLike<X, Res, N, T>& up,
            const bool up_is_normalized) const
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        if (up_is_normalized) {
            return std::atan2(this->cross(that).dot(up), this->dot(that));
        } else {
            return std::atan2(this->cross(that).dot(up.normalized()),
                              this->dot(that));
        }
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (3D)
     */
    template <typename W, typename X>
    static T angle(const VecLike<V, Res, N, T> lhs,
                   const VecLike<W, Res, N, T>& rhs,
                   const VecLike<X, Res, N, T>& up,
                   const bool up_is_normalized)
        requires(N == 3 && std::is_floating_point_v<T>)
    {
        return lhs.angle(rhs, up, up_is_normalized);
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (2D)
     */
    template <typename W>
    T angle(const VecLike<W, Res, N, T>& that) const
        requires(N == 2 && std::is_floating_point_v<T>)
    {
        return std::atan2(this->cross(that), this->dot(that));
    }

    /**
     * @brief Calculate the signed angle ranging from -pi and pi (3D)
     */
    template <typename W>
    static T angle(const VecLike<V, Res, N, T>& lhs,
                   const VecLike<W, Res, N, T>& rhs)
        requires(N == 2 && std::is_floating_point_v<T>)
    {
        return lhs.angle(rhs);
    }

    /**
     * @brief Linearly interpolate the values of two vectors with a parameter
     * value with t ranging from 0 to 1.
     *
     * To pique your interest:
     * https://www.youtube.com/watch?v=NzjF1pdlK7Y (Freya Holmer's talk)
     */
    template <typename W, typename U>
        requires(non_narrowing_conversion<U, T>)
    static Res lerp(const VecLike<V, Res, N, T>& a,
                    const VecLike<W, Res, N, T>& b,
                    const U t)
        requires(std::is_floating_point_v<T>)
    {
        return a * (T{1} - T{t}) + b * T{t};
    }
};

}  // namespace asciirast::math
