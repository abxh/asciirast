/**
 * @file Angle.h
 * @brief File with definition of the angle class
 */

#pragma once

#include <numbers>

template <typename T>
static const T PI = std::numbers::pi;

template <typename T>
static const T PI_2 = std::numbers::pi + std::numbers::pi;

template <typename T>
    requires(std::is_floating_point_v<T>)
class Angle {
public:
    static Angle from_deg(const T deg) { return Angle{PI_2<T> * deg / T{360}}; }
    static Angle from_rad(const T rad) { return Angle{rad}; }

public:
    T m_rad;

public:
    Angle(T rad) : m_rad{rad} {}
    operator T() const {return m_rad; }
    T as_rad() const { return m_rad; }
    T as_deg() const { return T{360} * m_rad / PI_2<T>; }
};
