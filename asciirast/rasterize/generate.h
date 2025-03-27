#pragma once

#include <cfloat>
#include <generator>

#include "../math.h"
#include "../program.h"

namespace asciirast::rasterize {

static_assert(std::numeric_limits<math::F>::min() >= std::numeric_limits<std::int64_t>::min());
static_assert(std::numeric_limits<math::F>::max() >= std::numeric_limits<std::int64_t>::max());

template<VaryingType Varying>
static std::generator<std::tuple<math::Vec2, math::F, Varying>>
generate_line_fragments(const ProjectedFragment<Varying>& proj0, const ProjectedFragment<Varying>& proj1)
{
    // DDA Line algorithm:
    // - https://www.redblobgames.com/grids/line-drawing/#more
    // - https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto pos0   = proj0.pos;
    const auto depth0 = proj0.depth;
    const auto attrs0 = proj0.attrs;

    const auto pos1   = proj1.pos;
    const auto depth1 = proj1.depth;
    const auto attrs1 = proj1.attrs;

    const auto perspective_corrected_t = [=](const math::F t) { return t * depth0 / std::lerp(depth0, depth1, t); };

    const auto delta   = pos1 - pos0;
    const auto size    = math::abs(delta);
    const auto len     = std::max<math::F>(size.x, size.y);
    const auto len_inv = 1 / len; // division by zero let through

    const auto inc_t     = len_inv;
    const auto inc_pos   = (pos1 - pos0) * len_inv;
    const auto inc_depth = (depth1 - depth0) * len_inv;
    const auto inc_attrs = (attrs1 + attrs0 * -1) * len_inv;

    auto acc_t     = math::F{ 0 };
    auto acc_pos   = pos0;
    auto acc_depth = depth0;
    auto acc_attrs = attrs0;

    if (std::isfinite(depth0) && std::isfinite(depth1)) {
        (void)(acc_attrs, inc_attrs);

        for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
            co_yield std::make_tuple(math::floor(acc_pos),
                                     acc_depth,
                                     math::lerp_varying(proj0.attrs, proj1.attrs, perspective_corrected_t(acc_t)));

            acc_t += inc_t;
            acc_pos += inc_pos;
            acc_depth += inc_depth;
        }
    } else {
        (void)(acc_t, inc_t);

        for (std::int64_t i = 0; i < static_cast<std::int64_t>(len); i++) {
            co_yield std::make_tuple(math::floor(acc_pos), acc_depth, acc_attrs);

            acc_pos += inc_pos;
            acc_depth += inc_depth;
            acc_attrs = std::move(acc_attrs + inc_attrs);
        }
    }
}

} // namespace asciirast::rasterize
