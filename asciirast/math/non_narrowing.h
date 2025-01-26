/**
 * @file non_narrowing.h
 * @brief File with definition of the non-narrowing trait.
 *
 * Taken from:
 * https://stackoverflow.com/a/67603594
 */

#pragma once

template <typename From, typename To>
concept non_narrowing = requires(From f) { To{f}; };
