/**
 * @file MatBase.h
 * @brief Matrix base class
 *
 * Using column-major order. To learn more about the difference between
 * row-major and column major, see:
 * https://en.wikipedia.org/wiki/row-_and_column-major_order
 */

#pragma once

#include "Vec.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <ranges>
#include <type_traits>

namespace asciirast::math {

/**
 * @brief Possible matrix orders.
 */
enum class MatStorageOrder { ColumnMajor, RowMajor };

/**
 * @brief Matrix storage order.
 */
static constexpr auto MAT_STORAGE_ORDER = MatStorageOrder::ColumnMajor;

/**
 * @brief Map a 2d matrix index to a 1d array index.
 */
template <std::size_t M_y, std::size_t N_x>
static inline constexpr std::size_t get_matrix_index(const std::size_t y,
                                                     const std::size_t x) {
    if constexpr (MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor) {
        return M_y * x + y;
    } else {
        return N_x * y + x;
    }
}

/**
 * @brief MxN dimensional matrix base class.
 */
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class MatBase {
public:
    T m_elements[M_y * N_x];  ///< 1d-array storing the matrix elements.

public:
    /**
     * @brief Create matrix from columns
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    static MatBase from_columns(const Vs&... column_vectors)
        requires(N_x == sizeof...(column_vectors))
    {
        MatBase m{};
        auto list = {column_vectors...};

        for (auto [idx, vec] : std::views::zip(std::views::iota(0), list)) {
            m.column_set(idx, vec);
        }
        return m;
    }

    /**
     * @brief Create matrix from rows
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    static MatBase from_rows(const Vs&... row_vectors)
        requires(M_y == sizeof...(row_vectors))
    {
        MatBase m{};
        auto list = {row_vectors...};

        for (auto [idx, vec] : std::views::zip(std::views::iota(0), list)) {
            m.row_set(idx, vec);
        }
        return m;
    }

public:
    /**
     * @brief The number of elements
     */
    constexpr auto size() const { return M_y * N_x; }

    /**
     * @brief The number of rows
     */
    constexpr auto row_count() const { return M_y; }

    /**
     * @brief The number of columns
     */
    constexpr auto column_count() const { return N_x; }

    /**
     * @brief Default constructor. Set diagonal elements to 1
     */
    MatBase() {
        std::fill(&this->m_elements[0], &this->m_elements[size()], T{0});

        for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
            this->m_elements[get_matrix_index<M_y, N_x>(i, i)] = T{1};
        }
    }

    /**
     * @brief Fill diagonal with initial value
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    explicit MatBase(const U& value) {
        std::fill(&this->m_elements[0], &this->m_elements[size()], T{0});

        for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
            this->m_elements[get_matrix_index<M_y, N_x>(i, i)] = value;
        }
    }

    /**
     * @brief Create matrix based on MAT_STORAGE_ORDER:
     * - If column major, copy vectors as column vectors.
     * - If row major, copy vectors as row vectors.
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    MatBase(const Vs&... column_vectors)
        requires(N_x == sizeof...(column_vectors) &&
                 MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor)
    {
        auto list = {column_vectors...};

        for (auto [idx, vec] : std::views::zip(std::views::iota(0), list)) {
            this->column_set(idx, vec);
        }
    }

    /**
     * @brief Create matrix based on MAT_STORAGE_ORDER:
     * - If column major, copy vectors as column vectors.
     * - If row major, copy vectors as row vectors.
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    MatBase(const Vs&... row_vectors)
        requires(M_y == sizeof...(row_vectors) &&
                 MAT_STORAGE_ORDER == MatStorageOrder::RowMajor)
    {
        auto list = {row_vectors...};

        for (auto [idx, vec] : std::views::zip(std::views::iota(0), list)) {
            this->row_set(idx, vec);
        }
    }

    /**
     * @brief Index the matrix with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    T& operator()(std::size_t y, std::size_t x) {
        if (y >= M_y || x >= N_x) {
            throw std::runtime_error("asciirast::math::MatBase::operator[]");
        }
        return this->m_elements[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Index the matrix with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    const T& operator()(std::size_t y, std::size_t x) const {
        if (y >= M_y || x >= N_x) {
            throw std::runtime_error("asciirast::math::MatBase::operator[]");
        }
        return this->m_elements[get_matrix_index<M_y, N_x>(y, x)];
    }

    /**
     * @brief Get x'th column with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    Vec<M_y, T> column_get(std::size_t x) const {
        if (x >= N_x) {
            throw std::runtime_error("asciirast::math::MatBase::column_get()");
        }
        if constexpr (MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor) {
            return Vec<M_y, T>(&this->m_elements[M_y * x]);
        } else {
            auto view = std::ranges::iota_view(0U, M_y) |
                        std::views::transform([&](auto y) {
                            return this->m_elements[get_matrix_index(y, x)];
                        });
            return Vec<M_y, T>(view.begin());
        }
    }

    /**
     * @brief Get y'th row with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    Vec<N_x, T> row_get(std::size_t y) const {
        if (y >= M_y) {
            throw std::runtime_error("asciirast::math::MatBase::row_get()");
        }
        if constexpr (MAT_STORAGE_ORDER == MatStorageOrder::RowMajor) {
            return Vec<N_x, T>(&this->m_elements[N_x * y]);
        } else {
            auto view = std::ranges::iota_view(0U, M_y) |
                        std::views::transform([&](auto x) {
                            return this->m_elements[get_matrix_index(y, x)];
                        });
            return Vec<N_x, T>(view.begin());
        }
    }

    /**
     * @brief Set x'th column with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    void column_set(std::size_t x, const Vec<M_y, T>& v) {
        if (x >= N_x) {
            throw std::runtime_error("asciirast::math::MatBase::column_set()");
        }
        if constexpr (MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor) {
            std::copy(v.begin(), v.end(), &this->m_elements[M_y * x]);
        } else {
            for (size_t y = 0; y < M_y; y++) {
                this->m_elements[get_matrix_index<M_y, N_x>(y, x)] = v[y];
            }
        }
    }

    /**
     * @brief Set y'th row with bounds checking
     * @throws runtime_error if indicies are out of bounds.
     */
    void row_set(std::size_t y, const Vec<N_x, T>& v) {
        if (y >= M_y) {
            throw std::runtime_error("asciirast::math::MatBase::row_set()");
        }
        if constexpr (MAT_STORAGE_ORDER == MatStorageOrder::RowMajor) {
            std::copy(v.begin(), v.end(), &this->m_elements[N_x * y]);
        } else {
            for (size_t x = 0; x < N_x; x++) {
                this->m_elements[get_matrix_index<M_y, N_x>(y, x)] = v[x];
            }
        }
    }

    /**
     * @brief Get row vectors
     */
    auto row_vectors() const {
        return std::ranges::iota_view(0U, N_x) |
               std::views::transform([this](auto x) { return row_get(x); });
    }

    /**
     * @brief Get column vectors
     */
    auto column_vectors() const {
        return std::ranges::iota_view(0U, M_y) |
               std::views::transform([this](auto y) { return column_get(y); });
    }
};

}  // namespace asciirast::math
