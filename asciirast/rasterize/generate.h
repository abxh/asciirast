#pragma once

#include <cfloat>
#include <generator>

#include "../math.h"
#include "../program.h"

namespace asciirast::rasterize {

static_assert(std::numeric_limits<math::F>::min() >= std::numeric_limits<std::int64_t>::min());
static_assert(std::numeric_limits<math::F>::max() >= std::numeric_limits<std::int64_t>::max());

static std::generator<math::Vec2>
generate_line(const math::F& len, const math::F& len_inv, const math::Vec2& v0, const math::Vec2& v1)
{
    // DDA Line algorithm:
    // - https://www.redblobgames.com/grids/line-drawing/#more
    // - https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    auto acc = v0;
    auto inc = (v1 - v0) * len_inv;

    for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
        co_yield math::floor(acc);
        acc += inc;
    }
}

static std::generator<math::F>
generate_depth(const math::F& len, const math::F& len_inv, const math::F& depth0, const math::F& depth1)
{
    auto acc = depth0;
    auto inc = (depth1 - depth0) * len_inv;

    for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
        co_yield acc;
        acc += inc;
    }
}

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs_lerp(const math::F& len,
                    const math::F& len_inv,
                    const ProjectedFragment<Varying>& proj0,
                    const ProjectedFragment<Varying>& proj1)
{
    auto acc = proj0.attrs;
    auto inc = (proj1.attrs + proj0.attrs * -1) * len_inv;

    for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
        co_yield acc;
        acc = std::move(acc + inc);
    }
}

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs_perspective_corrected(const math::F& len,
                                     const math::F& len_inv,
                                     const ProjectedFragment<Varying>& proj0,
                                     const ProjectedFragment<Varying>& proj1)
{
    auto perspective_corrected = [&proj0, &proj1](const math::F t) {
        return t * proj0.depth / std::lerp(proj0.depth, proj1.depth, t);
    };

    math::F acc_t = 0;
    math::F inc_t = len_inv;

    for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
        co_yield math::lerp_varying(proj0.attrs, proj1.attrs, perspective_corrected(acc_t));
        acc_t += inc_t;
    }
}

template<VaryingType Varying>
static std::generator<Varying>
generate_attrs(const math::F& len,
               const math::F& len_inv,
               const ProjectedFragment<Varying>& proj0,
               const ProjectedFragment<Varying>& proj1)
{
    if (std::isfinite(proj0.depth) && std::isfinite(proj1.depth)) {
        return generate_attrs_perspective_corrected(len, len_inv, proj0, proj1);
    } else {
        return generate_attrs_lerp(len, len_inv, proj0, proj1);
    }
}

} // namespace asciirast::rasterize
