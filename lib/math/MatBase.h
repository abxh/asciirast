/**
 * @file MatBase.h
 * @brief Matrix base class
 *
 * using column-major order. see:
 * https://en.wikipedia.org/wiki/row-_and_column-major_order
 */

#pragma once

#include "Vec.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <ranges>
#include <stdexcept>
#include <type_traits>

namespace asciirast::math {

enum class MatMajor { ColumnMajor, RowMajor };

static constexpr auto MAT_MAJOR = MatMajor::ColumnMajor;

template <std::size_t M_y, std::size_t N_x>
static inline constexpr std::size_t get_matrix_index(const std::size_t y,
                                                     const std::size_t x)
{
    if constexpr (MAT_MAJOR == MatMajor::ColumnMajor) {
        return M_y * x + y;
    }
    else {
        return N_x * y + x;
    }
}

template <std::size_t M_y, std::size_t N_x>
static constexpr auto generate_indicies_column_major()
{
    std::array<std::size_t, M_y * N_x> res;
    size_t idx = 0;
    for (std::size_t x = 0; x < N_x; x++) {
        for (std::size_t y = 0; y < M_y; y++) {
            res[idx++] = get_matrix_index<M_y, N_x>(y, x);
        }
    }
    return res;
}

template <std::size_t M_y, std::size_t N_x>
static constexpr auto generate_indicies_row_major()
{
    std::array<std::size_t, M_y * N_x> res;
    size_t idx = 0;
    for (std::size_t y = 0; y < M_y; y++) {
        for (std::size_t x = 0; x < N_x; x++) {
            res[idx++] = get_matrix_index<M_y, N_x>(y, x);
        }
    }
    return res;
}

template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class MatBase {
private:
    T table[M_y * N_x];

private:
    static constexpr auto column_major_indicies =
        generate_indicies_column_major<M_y, N_x>();
    static constexpr auto row_major_indicies =
        generate_indicies_row_major<M_y, N_x>();

    /**
     * @brief Zero identity of type
     */
    static constexpr T zero()
    {
        return T{0};
    }

    /**
     * @brief One identity of type
     */
    static constexpr T one()
    {
        return T{1};
    }

public:
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    static MatBase from_columns(const Vs &...column_vectors)
        requires(N_x == sizeof...(column_vectors))
    {
        MatBase m{};
        const auto v = {column_vectors...};
        for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
            m.column_set(idx, vec);
        }
        return m;
    }

    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    static MatBase from_rows(const Vs &...row_vectors)
        requires(M_y == sizeof...(row_vectors))
    {
        MatBase m{};
        const auto v = {row_vectors...};
        for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
            m.row_set(idx, vec);
        }
        return m;
    }

public:
    /**
     * @brief The number of elements
     */
    constexpr auto size() const
    {
        return M_y * N_x;
    }

    /**
     * @brief The number of rows
     */
    constexpr auto row_count() const
    {
        return M_y;
    }

    /**
     * @brief The number of columns
     */
    constexpr auto column_count() const
    {
        return N_x;
    }

    /**
     * @brief Default constructor. Set diagonal elements to 1
     */
    MatBase()
    {
        std::fill(&this->table[0], &this->table[size()], zero());
        for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
            this->table[get_matrix_index<M_y, N_x>(i, i)] = one();
        }
    }

    /**
     * @brief Fill diagonal with initial value
     */
    template <typename U>
        requires(utils::non_narrowing_conv<T, U>)
    explicit MatBase(const U &initial_diagonal_value)
    {
        std::fill(&this->table[0], &this->table[size()], zero());
        for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
            this->table[get_matrix_index<M_y, N_x>(i, i)] =
                initial_diagonal_value;
        }
    }

    /**
     * @brief Initate matrix based on MAT_MAJOR.
     * - If column major, copy vectors as column vectors.
     * - If row major, copy vectors as row vectors.
     */
    template <typename... Vs>
        requires(MAT_MAJOR == MatMajor::ColumnMajor
                 && std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    MatBase(const Vs &...column_vectors)
        requires(N_x == sizeof...(column_vectors))
    {
        const auto v = {column_vectors...};
        for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
            this->column_set(idx, vec);
        }
    }

    /**
     * @brief Initate matrix based on MAT_MAJOR.
     * - If column major, copy vectors as column vectors.
     * - If row major, copy vectors as row vectors.
     */
    template <typename... Vs>
        requires(MAT_MAJOR == MatMajor::RowMajor
                 && std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    MatBase(const Vs &...row_vectors)
        requires(M_y == sizeof...(row_vectors))
    {
        const auto v = {row_vectors...};
        for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
            this->row_set(idx, vec);
        }
    }

    /**
     * @brief Print matrix.
     */
    friend std::ostream &operator<<(std::ostream &out,
                                    const MatBase<M_y, N_x, T> &m)
    {
        auto vs = m.row_vectors();
        out << "[";
        Vec<N_x, T>::print(out, vs[0], 10, ' ', "", "");
        if (m.row_count() > 1) {
            out << std::endl;
        }
        for (auto v :
             vs | std::views::drop(1) | std::views::take(m.row_count() - 2)) {
            out << " ";
            Vec<N_x, T>::print(out, v, 10, ' ', "", "");
            out << std::endl;
        }
        if (m.row_count() > 1) {
            out << " ";
            Vec<N_x, T>::print(out, vs[m.row_count() - 1], 10, ' ', "", "");
        }
        out << "]" << std::endl;
        return out;
    }

    /**
     * @brief Index the matrix. With no OOB checking.
     */
    T &index(std::size_t y, std::size_t x) noexcept
    {
        return this->table[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Index the matrix. With no OOB checking.
     */
    const T &index(std::size_t y, std::size_t x) const noexcept
    {
        return this->table[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Index the matrix () operator. Can throw runtime_error.
     */
    T &operator()(std::size_t y, std::size_t x)
    {
        if (y >= M_y || x >= N_x) {
            throw std::runtime_error("asciirast::math::Mat<>::operator[]");
        }
        return this->table[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Index the matrix with () operator. Can throw runtime_error.
     */
    const T &operator()(std::size_t y, std::size_t x) const
    {
        if (y >= M_y || x >= N_x) {
            throw std::runtime_error("asciirast::math::Mat<>::operator[]");
        }
        return this->table[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Get j'th column
     */
    Vec<M_y, T> column_get(std::size_t j) const
    {
        if (j >= N_x) {
            throw std::runtime_error("asciirast::math::Mat<>::column_get()");
        }
        if constexpr (MAT_MAJOR == MatMajor::ColumnMajor) {
            return Vec<M_y, T>(&this->table[M_y * j]);
        }
        else {
            auto values = column_major_indicies | std::views::drop(M_y * j)
                        | std::views::take(M_y)
                        | std::views::transform([&](const auto idx) {
                              return this->table[idx];
                          });
            return Vec<M_y, T>(values.begin());
        }
    }

    /**
     * @brief Get i'th row
     */
    Vec<N_x, T> row_get(std::size_t i) const
    {
        if (i >= M_y) {
            throw std::runtime_error("asciirast::math::Mat<>::row_get()");
        }
        if constexpr (MAT_MAJOR == MatMajor::RowMajor) {
            return Vec<N_x, T>(&this->table[N_x * i]);
        }
        else {
            auto values = row_major_indicies | std::views::drop(N_x * i)
                        | std::views::take(N_x)
                        | std::views::transform([&](const auto idx) {
                              return this->table[idx];
                          });
            return Vec<N_x, T>(values.begin());
        }
    }

    /**
     * @brief Set j'th column
     */
    void column_set(std::size_t j, const Vec<M_y, T> &v)
    {
        if (j >= N_x) {
            throw std::runtime_error("asciirast::math::Mat<>::column_set()");
        }
        if constexpr (MAT_MAJOR == MatMajor::ColumnMajor) {
            std::copy(v.begin(), v.end(), &this->table[M_y * j]);
        }
        else {
            auto indicies = column_major_indicies | std::views::drop(M_y * j)
                          | std::views::take(M_y);

            for (auto [idx, value] : std::views::zip(indicies, v)) {
                this->table[idx] = value;
            }
        }
    }

    /**
     * @brief Set i'th row
     */
    void row_set(std::size_t i, const Vec<N_x, T> &v)
    {
        if (i >= M_y) {
            throw std::runtime_error("asciirast::math::Mat<>::row_set()");
        }
        if constexpr (MAT_MAJOR == MatMajor::RowMajor) {
            std::copy(v.begin(), v.end(), &this->table[N_x * i]);
        }
        else {
            auto indicies = row_major_indicies | std::views::drop(N_x * i)
                          | std::views::take(N_x);

            for (auto [idx, value] : std::views::zip(indicies, v)) {
                this->table[idx] = value;
            }
        }
    }

    /**
     * @brief Get row vectors
     */
    auto row_vectors() const
    {
        return row_major_indicies | std::views::transform([&](const auto idx) {
                   return this->table[idx];
               })
             | std::views::chunk(N_x)
             | std::views::transform([](const auto &chunk) {
                   return Vec<N_x, T>{chunk.begin()};
               });
    }

    /**
     * @brief Get column vectors
     */
    auto column_vectors() const
    {
        return column_major_indicies
             | std::views::transform([&](const auto idx) {
                   return this->table[idx];
               })
             | std::views::chunk(M_y)
             | std::views::transform([](const auto &chunk) {
                   return Vec<M_y, T>{chunk.begin()};
               });
    }
};

} // namespace asciirast::math
