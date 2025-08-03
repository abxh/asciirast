/**
 * @file AABB.h
 * @brief Axis-Aligned Bounding Box
 */

#pragma once

#include <type_traits>

#include "./Transform.h"

namespace asciirast::math {

/**
 * @brief Axis-Aligned Bounding Box
 *
 * @tparam N             Number of dimensions being operated on.
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the produced matricies uses column major
 */
template<std::size_t N, typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class AABB
{
public:
    using Transform2D = Transform2DType<T, is_col_major>;
    using Transform3D = Transform3DType<T, is_col_major>;

    /**
     * @brief Construct AABB from a minimum and maximum point
     *
     * @param min_ The minimum of the AABB as Vec
     * @param max_ The maximum of the AABB as Vec
     * @return The AABB that spans from the minimum to the maximum
     */
    [[nodiscard]] static constexpr AABB from_min_max(const Vec<N, T>& min_, const Vec<N, T>& max_) noexcept
    {
        const auto size = max(max_ - min_, Vec<N, T>::from_value(0));
        const auto center = min_ + size / 2;

        return AABB{ center, size };
    }

    /**
     * @brief Construct AABB from center and size
     *
     * @param center The center of the AABB
     * @param size The Vec spanning from the minimum to the maximum
     */
    constexpr explicit AABB(const Vec<N, T>& center, const Vec<N, T>& size) noexcept
            : m_center{ center }
            , m_extent{ max(size / 2, math::Vec<N, T>::from_value(0)) } {};

    /**
     * @brief Check if a point is contained within the AABB
     *
     * @param v_ The point at hand
     * @return Whether the point is contained in the AABB
     */
    [[nodiscard]] constexpr bool contains(const Vec<N, T>& v_) const
    {
        const auto v = v_ - m_center;

        return -m_extent <= v && v <= +m_extent;
    }

    /**
     * @brief Check if another AABB is a contained in this AABB
     *
     * @param that The another AABB
     * @return Whether another AABB is contained in this AABB
     */
    [[nodiscard]] constexpr bool contains(const AABB& that) const
    {
        return this->min_get() <= that.min_get() && that.max_get() <= this->max_get();
    }

    /**
     * @brief Convert this AABB to it's corresponding transform
     * object, that takes in a unit area and converts it to it's area
     *
     * @return The Transform object that converts a unit area to the AABB's area
     */
    [[nodiscard]] constexpr Transform2D to_transform() const
        requires(N == 2)
    {
        return Transform2D().scale(this->size_get()).translate(this->min_get());
    }

    /**
     * @brief Convert this AABB to it's corresponding transform
     * object, that takes in a unit volume and converts it to it's volume
     *
     * @return The Transform object tha converts a unit volume to the AABB's voume
     */
    [[nodiscard]] constexpr Transform3D to_transform() const
        requires(N == 3)
    {
        return Transform3D().scale(this->size_get()).translate(this->min_get());
    }

    /**
     * @brief Get the minimum of this AABB
     *
     * @return The minimum as a Vec
     */
    [[nodiscard]] constexpr Vec<N, T> min_get() const { return m_center - m_extent; }

    /**
     * @brief Set the minimum of this AABB
     *
     * @param min The new minimum point
     * @return This
     */
    constexpr AABB& min_set(const Vec<N, T>& min)
    {
        *this = std::move(AABB::from_min_max(min, this->max_get()));
        return *this;
    }

    /**
     * @brief Get the maximum of this AABB
     *
     * @return The maximum as a Vec
     */
    [[nodiscard]] constexpr Vec<N, T> max_get() const { return m_center + m_extent; }

    /**
     * @brief Set the maximum of this AABB
     *
     * @param max The new maximum point
     * @return This
     */
    constexpr AABB& max_set(const Vec<N, T>& max)
    {
        *this = std::move(AABB::from_min_max(this->min_get(), max));
        return *this;
    }

    /**
     * @brief Get the center of the AABB
     *
     * @return The center as Vec
     */
    [[nodiscard]] constexpr Vec<N, T> center_get() const { return m_center; }

    /**
     * @brief Set the center of the AABB
     *
     * @param center The new center
     * @return This
     */
    constexpr AABB& center_set(const Vec<N, T>& center)
    {
        *this = std::move(AABB{ center, this->size_get() });
        return *this;
    }

    /**
     * @brief Get the size of the AABB
     *
     * @return The size of the AABB as Vec
     */
    [[nodiscard]] constexpr Vec<N, T> size_get() const { return m_extent + m_extent; }

    /**
     * @brief Set the size of the AABB
     *
     * @param size The new size
     * @return This
     */
    constexpr AABB& size_set(const Vec<N, T>& size)
    {
        m_extent = max(size / 2, Vec<N, T>::from_value(0));
        return *this;
    }

private:
    Vec<N, T> m_center; /// center of bounding box
    Vec<N, T> m_extent; /// half of size
};

} // namespace asciirast::math
