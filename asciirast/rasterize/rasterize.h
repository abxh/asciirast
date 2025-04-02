#pragma once

#include <cfloat>
#include <generator>

#include "../math/types.h"
#include "../program.h"
#include "./lerp.h"

namespace asciirast::rasterize {

template<VaryingInterface Varying>
static auto
rasterize_line(const ProjectedFragment<Varying>& proj0, const ProjectedFragment<Varying>& proj1)
        -> std::generator<std::tuple<math::Vec2, math::F, math::F, Varying>>
{
    // DDA Line algorithm:
    // - https://www.redblobgames.com/grids/line-drawing/#more
    // - https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)

    const auto [pos0, z_inv0, w_inv0, attrs0] = proj0;
    const auto [pos1, z_inv1, w_inv1, attrs1] = proj1;

    const auto delta   = pos1 - pos0;
    const auto size    = math::abs(delta);
    const auto len     = std::max<math::F>(size.x, size.y);
    const auto len_inv = 1 / len; // division by zero let through

    const auto inc_t     = len_inv;
    const auto inc_pos   = (pos1 - pos0) * len_inv;
    const auto inc_z_inv = (z_inv1 - z_inv0) * len_inv;
    const auto inc_w_inv = (w_inv1 - w_inv0) * len_inv;
    const auto inc_attrs = (attrs1 + attrs0 * -1) * len_inv;

    auto acc_t     = math::F{ 0 };
    auto acc_pos   = pos0;
    auto acc_z_inv = z_inv0;
    auto acc_w_inv = w_inv0;
    auto acc_attrs = attrs0;

    if (std::isfinite(z_inv0) && std::isfinite(z_inv1)) {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            co_yield std::make_tuple(
                    math::floor(acc_pos),
                    acc_z_inv,
                    acc_w_inv,
                    lerp_varying(proj0.attrs, proj1.attrs, perspective_corrected_t_value(z_inv0, z_inv1, acc_t)));

            acc_t += inc_t;
            acc_pos += inc_pos;
            acc_z_inv += inc_z_inv;
            acc_w_inv += inc_w_inv;
        }
    } else {
        for (std::size_t i = 0; i < static_cast<std::size_t>(len); i++) {
            co_yield std::make_tuple(math::floor(acc_pos), acc_z_inv, acc_w_inv, acc_attrs);

            acc_pos += inc_pos;
            acc_z_inv += inc_z_inv;
            acc_w_inv += inc_w_inv;
            acc_attrs = std::move(acc_attrs + inc_attrs);
        }
    }
}

} // namespace asciirast::rasterize
