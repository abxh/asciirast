/*  non_narrowing_conv.h
 *
 *  Copyright (C) 2023 abxh
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *  See the file LICENSE included with this distribution for more
 *  information. */

/**
 * @file non_narrowing_conv.h
 * @brief Non narrowing conversion type restraint (c++ concept).
 *
 * based on:
 * https://stackoverflow.com/a/67603594
 */

#pragma once

namespace asciirast::math::utils {

template <typename To, typename... From>
concept non_narrowing_conv = (requires(From f) { To{f}; } && ...);

} // namespace asciirast::math::utils
