/**
 * @file AABB.h
 * @brief File with definition of axis-aligned bounding box
 * @todo raycast, distanceSqToPoint support?
 */

#pragma once

#include <cassert>
#include <cmath>
#include <type_traits>

#include "./Transform.h"

namespace asciirast::math {

/**
 * @brief Axis-Aligned Bounding Box
 *
 * @tparam N             Number of dimensions being operated on.
 * @tparam T             Type of elements
 * @tparam is_col_major  Whether the transform uses column major
 */
template<std::size_t N, typename T, bool is_col_major>
    requires(std::is_floating_point_v<T>)
class AABB
{
private:
    Vec<N, T> m_center; ///< center of bounding box
    Vec<N, T> m_extent; ///< half of size

public:
    /**
     * @brief Construct AABB from min and max
     */
    static AABB from_min_max(const Vec<N, T>& min, const Vec<N, T>& max)
    {
        const auto& size = math::max(max - min, Vec<N, T>{ 0 });

        return AABB{ min + size / 2, size };
    }

    /**
     * @brief Construct AABB from center and size
     */
    AABB(const Vec<N, T>& center, const Vec<N, T>& size)
            : m_center{ center }
            , m_extent{ math::max(size / 2, Vec<N, T>{ 0 }) }
    {
    }

    /**
     * @brief Check if bounding box contains a vector
     */
    bool contains(const Vec<N, T>& v_) const
    {
        const auto v = v_ - m_center;

        return -m_extent <= v && v <= +m_extent;
    }

    /**
     * @brief Check if bounding box contains a AABB completely
     */
    bool contains(const AABB& that) const
    {
        return this->min_get() <= that.min_get() && that.max_get() <= this->max_get();
    }

    /**
     * @brief Convert to transform that takes a unit area to converts it to the bounding box area
     */
    Transform<N, T, is_col_major> to_transform2() const
        requires(N == 2)
    {
        return Transform<N, T, is_col_major>().scale(this->size_get()).translate(this->min_get());
    }

    /**
     * @brief Convert to transform that takes a unit volume to converts it to the bounding box volume
     */
    Transform<N, T, is_col_major> to_transform3() const
        requires(N == 3)
    {
        return Transform<N, T, is_col_major>().scale(this->size_get()).translate(this->min_get());
    }

public:
    /**
     * @brief Get minimum of bounding box
     */
    Vec<N, T> min_get() const { return m_center - m_extent; }

    /**
     * @brief Set minimum of bounding box
     */
    AABB min_set(const Vec<N, T>& min) { return *this = std::move(AABB::from_min_max(min, this->max_get())); }

    /**
     * @brief Get maximum of bounding box
     */
    Vec<N, T> max_get() const { return m_center + m_extent; }

    /**
     * @brief Set maximum of bounding box
     */
    AABB max_set(const Vec<N, T>& max) { return *this = std::move(AABB::from_min_max(this->min_get(), max)); }

    /**
     * @brief Get size of bounding box
     */
    Vec<N, T> center_get() const { return m_center; }

    /**
     * @brief Set maximum of bounding box
     */
    AABB center_set(const Vec<N, T>& center)
    {
        m_center = center;
        return *this;
    }

    /**
     * @brief Get size of bounding box
     */
    Vec<N, T> size_get() const { return m_extent + m_extent; }

    /**
     * @brief Get size of bounding box
     */
    Vec<N, T> size_set(const Vec<N, T>& size)
    {
        m_extent = math::max(size / 2, Vec<N, T>{ 0 });
        return *this;
    }
};

}
