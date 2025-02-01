/**
 * @file Complex.h
 * @brief File with definition of the complex number class
 */

#pragma once

#include "Angle.h"
#include "Vec.h"

namespace asciirast::math {

template <typename T>
    requires(std::is_floating_point_v<T>)
class Complex {
public:
    static Complex from_angle(const Angle<T> angle) {
        return Complex{std::cos(angle.as_rad()), std::sin(angle.as_rad())};
    }

public:
    union {
        Vec<2, T> m_vec;
        struct {
            T real;
            T imag;
        };
    };

public:
    explicit Complex(const T real, const T imag) : m_vec{real, imag} {}
    explicit Complex(const Vec<2, T>& vec) : m_vec{vec} {}

    /**
     * @brief Print the complex number
     */
    friend std::ostream& operator<<(std::ostream& out, const Complex& v) {
        out << (v.real >= 0 ? " " : "") << v.real << " + "
            << (v.imag >= 0 ? " " : "") << v.imag << "i" << std::endl;
        return out;
    }

public:
    T length_squared() const { return dot(m_vec, m_vec); }

    T length() const { return m_vec.length(); }

    Complex conjugate() const { return Complex{real, -imag}; }

    Complex normalized() const { return Complex{m_vec.normalized()}; }

public:
    friend Complex operator+(const Complex& lhs, const Complex& rhs) {
        return Complex{lhs.m_vec + rhs.m_vec};
    }

    friend Complex operator-(const Complex& lhs, const Complex& rhs) {
        return Complex{lhs.m_vec - rhs.m_vec};
    }

    friend Complex operator*(const Complex& lhs, const Complex& rhs) {
        /*
            (a + b*i) * (c + d*i) = ac + ad*i + bc*i + bd*i^2
                                  = ac + ad*i + bc*i - bd
                                  = (ac - bd) + i(ad + bc)
        */
        const T real = lhs.real * rhs.real - lhs.imag * rhs.imag;
        const T imag = lhs.real * rhs.imag + lhs.imag * rhs.real;

        return Complex{real, imag};
    }

    friend Complex operator*(const T scalar, const Complex& rhs) {
        return Complex{scalar * rhs.m_vec};
    }

    friend Complex operator*(const Complex& lhs, const T scalar) {
        return Complex{lhs.m_vec * scalar};
    }

    friend Complex operator/(const Complex& lhs, const Complex& rhs) {
        /*
            lhs / rhs = lhs / rhs * rhs.conjugate() / rhs.conjugate()
                      = (a + b*i) / (c + d*i) * (c - d*i) / (c - d*i)
                      = (a + b*i) * (c - d*i) / (cc + dd + i(-cd + cd))
                      = (a + b*i) * (c - d*i) / (cc + dd)
        */
        return lhs * rhs.conjugate() / rhs.length_squared();
    }

    friend Complex operator/(const Complex& lhs, const T scalar) {
        return Complex{lhs.m_vec / scalar};
    }

public:
    Complex operator+=(const Complex& other) {
        m_vec += other.m_vec;
        return *this;
    }

    Complex operator-=(const Complex& other) {
        m_vec -= other.m_vec;
        return *this;
    }

    Complex operator*=(const T& scalar) {
        m_vec *= scalar;
        return *this;
    }

    Complex operator/=(const T& scalar) {
        m_vec /= scalar;
        return *this;
    }
};

}  // namespace asciirast::math
