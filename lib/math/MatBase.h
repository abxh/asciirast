/**
 * @file MatBase.h
 * @brief Matrix base class
 *
 * The matrix stores data using column-major order by default.
 *
 * For more information on row-major and column-major order, see:
 * https://en.wikipedia.org/wiki/row-_and_column-major_order
 */

#pragma once

#include <cmath>
#include <cstddef>
#include <ranges>
#include <type_traits>

#include "Vec.h"

namespace asciirast::math {

/**
 * @enum MatStorageOrder
 * @brief Enum for matrix storage order
 */
enum class MatStorageOrder { ColumnMajor, RowMajor };

/**
 * @constant MAT_STORAGE_ORDER
 * @brief The matrix storage order
 */
static constexpr auto MAT_STORAGE_ORDER = MatStorageOrder::ColumnMajor;

/**
 * @brief Compute the index of the element in the matrix based on the storage
 * order.
 *
 * @tparam M_y Number of rows in the matrix.
 * @tparam N_x Number of columns in the matrix.
 * @param y Row index.
 * @param x Column index.
 * @return The linear index into the matrix.
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
 * @brief Matrix base class for general-purpose matrix operations.
 *
 * @tparam M_y Number of rows in the matrix.
 * @tparam N_x Number of columns in the matrix.
 * @tparam T The type of elements in the matrix (must be arithmetic).
 */
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class MatBase {
public:
    T table[M_y * N_x]; /**< Array storing matrix elements */

public:
    /**
     * @brief Construct a matrix from column vectors.
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    static MatBase from_columns(const Vs&... column_vectors)
        requires(N_x == sizeof...(column_vectors));

    /**
     * @brief Construct a matrix from row vectors.
     */
    template <typename... Vs>
        requires(std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    static MatBase from_rows(const Vs&... row_vectors)
        requires(M_y == sizeof...(row_vectors));

    /**
     * @brief Get the total number of elements in the matrix.
     */
    constexpr auto size() const { return M_y * N_x; }

    /**
     * @brief Get the number of rows in the matrix.
     */
    constexpr auto row_count() const { return M_y; }

    /**
     * @brief Get the number of columns in the matrix.
     */
    constexpr auto column_count() const { return N_x; }

    /**
     * @brief Default constructor. Sets diagonal elements to 1 and others to 0.
     */
    MatBase();

    /**
     * @brief Constructor that initializes the matrix with a given value for
     * diagonal elements.
     */
    template <typename U>
        requires(non_narrowing_conversion<T, U>)
    explicit MatBase(const U& value);

    /**
     * @brief Construct a matrix from column vectors (if column-major) or row
     * vectors (if row-major).
     */
    template <typename... Vs>
        requires(MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor &&
                 std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
    MatBase(const Vs&... column_vectors)
        requires(N_x == sizeof...(column_vectors));

    /**
     * @brief Construct a matrix from column vectors (if column-major) or row
     * vectors (if row-major).
     */
    template <typename... Vs>
        requires(MAT_STORAGE_ORDER == MatStorageOrder::RowMajor &&
                 std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
    MatBase(const Vs&... row_vectors)
        requires(M_y == sizeof...(row_vectors));

    /**
     * @brief Index the matrix with bounds checking.
     *
     * @param y Row index.
     * @param x Column index.
     * @return A reference to the matrix element.
     * @throws std::runtime_error if out-of-bounds.
     */
    T& operator()(std::size_t y, std::size_t x);

    /**
     * @brief Index the matrix with bounds checking (const version).
     *
     * @param y Row index.
     * @param x Column index.
     * @return A const reference to the matrix element.
     * @throws std::runtime_error if out-of-bounds.
     */
    const T& operator()(std::size_t y, std::size_t x) const;

    /**
     * @brief Get the j'th column of the matrix.
     *
     * @param j The column index.
     * @return A vector representing the j'th column.
     * @throws std::runtime_error if column index is out-of-bounds.
     */
    Vec<M_y, T> column_get(std::size_t j) const;

    /**
     * @brief Get the i'th row of the matrix.
     *
     * @param i The row index.
     * @return A vector representing the i'th row.
     * @throws std::runtime_error if row index is out-of-bounds.
     */
    Vec<N_x, T> row_get(std::size_t i) const;

    /**
     * @brief Set the j'th column of the matrix.
     *
     * @param j The column index.
     * @param v The vector to set as the column.
     * @throws std::runtime_error if column index is out-of-bounds.
     */
    void column_set(std::size_t j, const Vec<M_y, T>& v);

    /**
     * @brief Set the i'th row of the matrix.
     *
     * @param i The row index.
     * @param v The vector to set as the row.
     * @throws std::runtime_error if row index is out-of-bounds.
     */
    void row_set(std::size_t i, const Vec<N_x, T>& v);

    /**
     * @brief Get all the row vectors of the matrix.
     *
     * @return A range of row vectors.
     */
    auto row_vectors() const;

    /**
     * @brief Get all the column vectors of the matrix.
     *
     * @return A range of column vectors.
     */
    auto column_vectors() const;
};

// Default Constructor
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
MatBase<M_y, N_x, T>::MatBase() {
    std::fill(&this->table[0], &this->table[size()], T{0});

    for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
        this->table[get_matrix_index<M_y, N_x>(i, i)] = T{1};
    }
}

// Constructor with initial value
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
template <typename U>
    requires(non_narrowing_conversion<T, U>)
MatBase<M_y, N_x, T>::MatBase(const U& value) {
    std::fill(&this->table[0], &this->table[size()], T{0});

    for (std::size_t i = 0; i < std::min(M_y, N_x); i++) {
        this->table[get_matrix_index<M_y, N_x>(i, i)] = value;
    }
}

// Constructor (column-major)
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
template <typename... Vs>
    requires(MAT_STORAGE_ORDER == MatStorageOrder::ColumnMajor &&
             std::same_as<std::common_type_t<Vs...>, Vec<M_y, T>>)
MatBase<M_y, N_x, T>::MatBase(const Vs&... column_vectors)
    requires(N_x == sizeof...(column_vectors))
{
    const auto v = {column_vectors...};

    for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
        this->column_set(idx, vec);
    }
}

// Constructor (row-major)
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
template <typename... Vs>
    requires(MAT_STORAGE_ORDER == MatStorageOrder::RowMajor &&
             std::same_as<std::common_type_t<Vs...>, Vec<N_x, T>>)
MatBase<M_y, N_x, T>::MatBase(const Vs&... row_vectors)
    requires(M_y == sizeof...(row_vectors))
{
    const auto v = {row_vectors...};

    for (auto [idx, vec] : std::views::zip(std::views::iota(0), v)) {
        this->row_set(idx, vec);
    }
}

// operator() (non-const)
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
T& MatBase<M_y, N_x, T>::operator()(std::size_t y, std::size_t x) {
    if (y >= M_y || x >= N_x) {
        throw std::runtime_error("asciirast::math::Mat<>::operator[]");
    }
    return this->table[get_matrix_index<M_y, N_x>(y, x)];
}

// operator() (const)
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
const T& MatBase<M_y, N_x, T>::operator()(std::size_t y, std::size_t x) const {
    if (y >= M_y || x >= N_x) {
        throw std::runtime_error("asciirast::math::Mat<>::operator[]");
    }
    return this->table[get_matrix_index<M_y, N_x>(y, x)];
}

// column_get
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
Vec<M_y, T> MatBase<M_y, N_x, T>::column_get(std::size_t j) const {
    if (j >= N_x) {
        throw std::runtime_error("asciirast::math::Mat::column_get");
    }
    Vec<M_y, T> v{};
    for (std::size_t i = 0; i < M_y; i++) {
        v[i] = this->table[get_matrix_index<M_y, N_x>(i, j)];
    }
    return v;
}

// row_get
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
Vec<N_x, T> MatBase<M_y, N_x, T>::row_get(std::size_t i) const {
    if (i >= M_y) {
        throw std::runtime_error("asciirast::math::Mat::row_get");
    }
    Vec<N_x, T> v{};
    for (std::size_t j = 0; j < N_x; j++) {
        v[j] = this->table[get_matrix_index<M_y, N_x>(i, j)];
    }
    return v;
}

// column_set
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
void MatBase<M_y, N_x, T>::column_set(std::size_t j, const Vec<M_y, T>& v) {
    if (j >= N_x) {
        throw std::runtime_error("asciirast::math::Mat::column_set");
    }
    for (std::size_t i = 0; i < M_y; i++) {
        this->table[get_matrix_index<M_y, N_x>(i, j)] = v[i];
    }
}

// row_set
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
void MatBase<M_y, N_x, T>::row_set(std::size_t i, const Vec<N_x, T>& v) {
    if (i >= M_y) {
        throw std::runtime_error("asciirast::math::Mat::row_set");
    }
    for (std::size_t j = 0; j < N_x; j++) {
        this->table[get_matrix_index<M_y, N_x>(i, j)] = v[j];
    }
}

// row_vectors
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
auto MatBase<M_y, N_x, T>::row_vectors() const {
    return std::ranges::iota_view(0U, M_y) |
           std::views::transform([this](auto idx) { return row_get(idx); });
}

// column_vectors
template <std::size_t M_y, std::size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
auto MatBase<M_y, N_x, T>::column_vectors() const {
    return std::ranges::iota_view(0U, M_y) |
           std::views::transform([this](auto idx) { return column_get(idx); });
}

}  // namespace asciirast::math
