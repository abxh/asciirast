/**
 * @file Mat.h
 * @brief Math matrix
 *
 * @todo determinant function
 * @todo inverse function
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <sstream>
#include <type_traits>

#include "./Vec.h"

namespace asciirast::math {

/// @cond DO_NOT_DOCUMENT
namespace detail {
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major, typename... Args>
struct mat_constructible_from;
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_initializer;
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_printer;
} // namespace detail
/// @endcond

/**
 * @brief Math matrix class
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
class Mat
{
    template<typename... Args>
    static constexpr bool constructible_from_cols_v =
            (detail::vec_info<Args>::value && ...) && (0 < sizeof...(Args) && sizeof...(Args) <= N_x) &&
            ((detail::vec_info<Args>::size <= M_y) && ...);

    template<typename... Args>
    static constexpr bool constructible_from_rows_v =
            (detail::vec_info<Args>::value && ...) && (0 < sizeof...(Args) && sizeof...(Args) <= M_y) &&
            ((detail::vec_info<Args>::size <= N_x) && ...);

    template<typename... Args>
    static constexpr bool constructible_from_args_v =
            (detail::not_a_single_convertible_value<T, Args...>::value &&
             detail::not_a_single_value<Mat, Args...>::value) &&
            (detail::mat_constructible_from<M_y, N_x, T, is_col_major, Args...>::value);

    std::array<T, M_y * N_x> m_elements = {}; ///< 1D array of elements

public:
    /**
     * @brief Map two-dimensional matrix indicies to a single dimensional array index
     *
     * @param y Zero-based row index
     * @param x Zero-based column index
     */
    [[nodiscard]] static constexpr std::size_t map_index(const std::size_t y, const std::size_t x)
    {
        if constexpr (is_col_major) {
            return M_y * x + y;
        } else {
            return N_x * y + x;
        }
    }

    /**
     * @brief Get the square identity matrix
     *
     * @return The identity matrix
     */
    [[nodiscard]] static constexpr Mat identity()
        requires(N_x == M_y)
    {
        Mat res{};
        if constexpr (is_col_major) {
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    res[y, x] = (y == x) ? 1 : 0;
                }
            }
        } else {
            for (std::size_t y = 0; y < M_y; y++) {
                for (std::size_t x = 0; x < N_x; x++) {
                    res[y, x] = (y == x) ? 1 : 0;
                }
            }
        }
        return res;
    }

    /**
     * @brief Create matrix from column vectors
     *
     * @param args The column vectors
     * @return The matrix consisting of the column vectors with the rest padded with zero
     */
    template<typename... Args>
        requires(constructible_from_cols_v<Args...>)
    [[nodiscard]] static constexpr Mat from_cols(Args&&... args) noexcept
    {
        if constexpr (is_col_major) {
            using MatType = Mat<M_y, N_x, T, is_col_major>;

            return MatType{ std::forward<Args>(args)... };
        } else {
            using MatType = Mat<N_x, M_y, T, is_col_major>;

            return MatType{ std::forward<Args>(args)... }.transposed();
        }
    }

    /**
     * @brief Create matrix from row vectors
     *
     * @param args The row vectors
     * @return The matrix consisting of the row vectors with the rest padded with zero
     */
    template<typename... Args>
        requires(constructible_from_rows_v<Args...>)
    [[nodiscard]] static constexpr Mat from_rows(Args&&... args) noexcept
    {
        if constexpr (is_col_major) {
            using MatType = Mat<N_x, M_y, T, is_col_major>;

            return MatType{ std::forward<Args>(args)... }.transposed();
        } else {
            using MatType = Mat<M_y, N_x, T, is_col_major>;

            return MatType{ std::forward<Args>(args)... };
        }
    }

public:
    /**
     * @brief Construct matrix with all zeroes
     */
    constexpr Mat() {};

    /**
     * @brief Construct matrix from a mix of smaller matricies and vectors
     *        with the rest padded with zeroes, initialized with the
     *        arguments in the same order as the matrix major order.
     *
     * @param args A mix of smaller matricies and vectors
     */
    template<typename... Args>
        requires(sizeof...(Args) > 0)
    constexpr Mat(Args&&... args) noexcept
        requires(constructible_from_args_v<Args...>)
    {
        using initializer = detail::mat_initializer<M_y, N_x, T, is_col_major>;

        initializer::init_from(*this, std::forward<Args>(args)...);
    };

public:
    /**
     * @brief Get number of rows in the matrix
     *
     * @return The row count as size_t
     */
    [[nodiscard]] static constexpr std::size_t row_count() { return M_y; }

    /**
     * @brief Get number of columns in the matrix
     *
     * @return The column count as size_t
     */
    [[nodiscard]] static constexpr std::size_t col_count() { return N_x; }

    /**
     * @brief Get the total size of the matrix
     *
     * @return The matrix size as size_t
     */
    [[nodiscard]] static constexpr std::size_t size() { return M_y * N_x; }

    /**
     * @brief Get pointer to the underlying data in the matrix major order
     *
     * @return The pointer to the first element of the underlying data
     */
    [[nodiscard]] constexpr T* data() { return &m_elements[0]; }

    /**
     * @brief Get const pointer to the underlying data in the matrix major order
     *
     * @return The const pointer to the first element of the underlying data
     */
    [[nodiscard]] constexpr const T* data() const { return &m_elements[0]; }

    /**
     * @brief Get underlying array in matrix major order
     *
     * @return A reference to the underlying array
     */
    [[nodiscard]] constexpr std::array<T, M_y * N_x>& array() { return m_elements; }

    /**
     * @brief Get underlying array in matrix major order
     *
     * @return A const reference to the underlying array
     */
    [[nodiscard]] constexpr const std::array<T, M_y * N_x>& array() const { return m_elements; }

    /**
     * @brief Index the underlying array
     *
     * @param i The index
     * @return Reference to the value at the index
     */
    [[nodiscard]] constexpr T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the underlying array
     *
     * @param i The index
     * @return The value at the index
     */
    [[nodiscard]] constexpr T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the matrix
     *
     * @param y The row index
     * @param x The column index
     * @return A reference to the value at the indicies
     */
    [[nodiscard]] constexpr T& operator[](const std::size_t y, const std::size_t x)
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Index the matrix
     *
     * @param y The row index
     * @param x The column index
     * @return The value at the indicies
     */
    [[nodiscard]] constexpr T operator[](const std::size_t y, const std::size_t x) const
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Print the matrix
     *
     * @param out The output stream
     * @param m The matrix at hand
     * @return A reference to the output stream modified
     */
    friend std::ostream& operator<<(std::ostream& out, const Mat& m)
    {
        using mat_printer = detail::mat_printer<M_y, N_x, T, is_col_major>;

        return mat_printer::print(out, m);
    }

    /**
     * @brief Get a copy of the transposed of the matrix
     *
     * @return A copy of the transposed to the matrix
     */
    [[nodiscard]] constexpr Mat<N_x, M_y, T, is_col_major> transposed() const
    {
        Mat<N_x, M_y, T, is_col_major> res{};
        if constexpr (is_col_major) {
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    res[x, y] = (*this)[y, x];
                }
            }
        } else {
            for (std::size_t y = 0; y < M_y; y++) {
                for (std::size_t x = 0; x < N_x; x++) {
                    res[x, y] = (*this)[y, x];
                }
            }
        }
        return res;
    }

    /**
     * @brief Get the y'th row vector
     *
     * @param y The row-index
     * @return The y'th row vector as Vec
     */
    [[nodiscard]] constexpr Vec<N_x, T> row_get(const std::size_t y) const
    {
        assert(y < M_y && "index is inside bounds");

        Vec<N_x, T> res{};
        for (std::size_t x = 0; x < N_x; x++) {
            res[x] = (*this)[y, x];
        }

        return res;
    }

    /**
     * @brief Set the y'th row vector
     *
     * @param y The row-index
     * @param v The new row vector
     * @return This
     */
    constexpr Mat& row_set(const std::size_t y, const Vec<N_x, T>& v)
    {
        assert(y < M_y && "index is inside bounds");

        for (std::size_t x = 0; x < N_x; x++) {
            (*this)[y, x] = v[x];
        }

        return *this;
    }

    /**
     * @brief Get the x'th column vector
     *
     * @param x The column-index
     * @return The x'th column vector as Vec
     */
    [[nodiscard]] constexpr Vec<M_y, T> col_get(const std::size_t x) const
    {
        assert(x < N_x && "index is inside bounds");

        Vec<M_y, T> res{};
        for (std::size_t y = 0; y < M_y; y++) {
            res[y] = (*this)[y, x];
        }

        return res;
    }

    /**
     * @brief Set the x'th column vector
     *
     * @param x The column-index
     * @param v The new column vector
     * @return This
     */
    constexpr Mat& col_set(const std::size_t x, const Vec<M_y, T>& v)
    {
        assert(x < M_y && "index is inside bounds");

        for (std::size_t y = 0; y < M_y; y++) {
            (*this)[y, x] = v[y];
        }

        return *this;
    }

public:
    /**
     * @brief Check if two matricies are (approximately) equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the matrices are (approximately) equal
     */
    [[nodiscard]] friend constexpr bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T>)
    {
        bool res = true;
        for (std::size_t i = 0; i < size(); i++) {
            res &= lhs[i] == rhs[i];
        }
        return res;
    }

    /**
     * @brief Check if two matricies are (approximately) equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the matrices are (approximately) equal
     */
    [[nodiscard]] friend constexpr bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        bool res = true;
        for (std::size_t i = 0; i < size(); i++) {
            res &= almost_equal(lhs[i], rhs[i]);
        }
        return res;
    }

    /**
     * @brief Check if two matricies are (approximately) not equal
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return Whether the matrices are (approximately) not equal
     */
    [[nodiscard]] friend constexpr bool operator!=(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

public:
    /**
     * @brief Perform in-place component-wise addition with other matrix
     *
     * @param that The other matrix at hand
     * @return This
     */
    constexpr Mat& operator+=(const Mat& that)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief Perform in-place component-wise subtraction with other matrix
     *
     * @param that The other matrix at hand
     * @return This
     */
    constexpr Mat& operator-=(const Mat& that)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief Perform in-place matrix-scalar multiplication
     *
     * @param scalar The scalar at hand
     * @return The matrix with it's element scaled by the given scalar
     */
    constexpr Mat& operator*=(const T scalar)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief Perform in-place matrix-scalar division
     *
     * @param scalar The scalar at hand
     * @return The matrix with it's element inversely scaled by the given scalar
     */
    constexpr Mat& operator/=(const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != 0 && "non-zero division");
        }
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] /= scalar;
        }
        return *this;
    }

public:
    /**
     * @brief Perform unary plus operator
     *
     * @return The copy of the matrix as-is
     */
    [[nodiscard]] constexpr Mat operator+() const { return *this; }

    /**
     * @brief Perform unary minus operator
     *
     * @return The matrix with it's elements sign-flipped
     */
    [[nodiscard]] constexpr Mat operator-() const
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = -(*this)[i];
        }
        return res;
    }

    /**
     * @brief Perform matrix-matrix component-wise addition
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat operator+(const Mat& lhs, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] + rhs[i];
        }
        return res;
    }

    /**
     * @brief Perform matrix-matrix component-wise subtraction
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat operator-(const Mat& lhs, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] - rhs[i];
        }
        return res;
    }

    /**
     * @brief Perform scalar-matrix component-wise multiplication
     *
     * @param scalar The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat operator*(const T scalar, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = scalar * rhs[i];
        }
        return res;
    }

    /**
     * @brief Perform matrix-scalar component-wise multiplication
     *
     * @param lhs The left hand side of the operand
     * @param scalar The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat operator*(const Mat& lhs, const T scalar)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] * scalar;
        }
        return res;
    }

    /**
     * @brief Perform matrix-scalar component-wise division
     *
     * @param lhs The left hand side of the operand
     * @param scalar The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat operator/(const Mat& lhs, const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != 0 && "non-zero division");
        }
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] / scalar;
        }
        return res;
    }

public:
    /**
     * @brief Perform in-place multiplication with other matrix
     *
     * @param that The other matrix at hand
     * @return This modified
     */
    constexpr Mat& operator*=(const Mat& that)
    {
        *this = std::move((*this) * that);
        return *this;
    }

    /**
     * @brief Perform matrix-matrix multiplication
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. But is not particularly optimised.
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Mat<M_y, M_y, T, is_col_major> operator*(const Mat<M_y, N_x, T, is_col_major>& lhs,
                                                                            const Mat<N_x, M_y, T, is_col_major>& rhs)
    {
        Mat<M_y, M_y, T, is_col_major> res{};

        if constexpr (is_col_major) {
            const auto lhs_T = lhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < rhs.col_count(); i++) {
                for (std::size_t j = 0; j < lhs_T.col_count(); j++) {
                    const T* lb = &rhs.data()[M_y * i];
                    const T* le = &rhs.data()[M_y * (i + 1)];
                    const T* rb = &lhs_T.data()[M_y * j];

                    res[idx++] = std::inner_product(lb, le, rb, T{});
                }
            }
        } else {
            const auto rhs_T = rhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < lhs.row_count(); i++) {
                for (std::size_t j = 0; j < rhs_T.row_count(); j++) {
                    const T* lb = &lhs.data()[N_x * i];
                    const T* le = &lhs.data()[N_x * (i + 1)];
                    const T* rb = &rhs_T.data()[N_x * j];

                    res[idx++] = std::inner_product(lb, le, rb, T{});
                }
            }
        }
        return res;
    }

    /**
     * @brief Perform matrix-matrix multiplication
     *
     * @todo simd'ify col_major matrix-vec multiplication in c++26
     *
     * @param lhs The left hand side of the operand
     * @param rhs The right hand side of the operand
     * @return The resulting matrix
     */
    [[nodiscard]] friend constexpr Vec<M_y, T> operator*(const Mat<M_y, N_x, T, is_col_major>& lhs,
                                                         const Vec<N_x, T>& rhs)
    {
        Vec<M_y, T> res{};

        if constexpr (is_col_major) {
            std::size_t idx = 0;
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    res[y] += lhs[idx++] * rhs[x];
                }
            }
        } else {
            for (std::size_t y = 0; y < M_y; y++) {
                auto lb = &lhs.data()[N_x * y];
                auto le = &lhs.data()[N_x * (y + 1)];
                auto rb = rhs.array().begin();

                res[y] = std::inner_product(lb, le, rb, T{});
            }
        }
        return res;
    }
};

namespace detail {

/// @cond DO_NOT_DOCUMENT

template<typename TT>
struct mat_info_impl
{
    using value_type = void;
    static constexpr bool value = false;
    static constexpr std::size_t height = 0;
    static constexpr std::size_t width = 0;
};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_info_impl<Mat<M_y, N_x, T, is_col_major>>
{
    using value_type = T;
    static constexpr bool value = true;
    static constexpr std::size_t height = M_y;
    static constexpr std::size_t width = N_x;
};

template<typename TT>
struct mat_info : mat_info_impl<std::remove_cvref_t<TT>>
{};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major, typename... Args>
struct mat_constructible_from_impl
{
private:
    static constexpr bool accepted_types = ((vec_info<Args>::value || mat_info<Args>::value) && ...);

    static constexpr std::size_t num_vecs = ((vec_info<Args>::value ? 1 : 0) + ...);

    static constexpr std::size_t vec_max_length = std::max({ vec_info<Args>::size... });

    static constexpr std::size_t mat_max_width = std::max({ mat_info<Args>::width... });

    static constexpr std::size_t mat_max_height = std::max({ mat_info<Args>::height... });

    static constexpr bool total_area_in_bounds_col_major =
            (std::max(vec_max_length, mat_max_height) <= M_y) && (num_vecs + (mat_info<Args>::width + ...) <= N_x);

    static constexpr bool total_area_in_bounds_row_major =
            (std::max(vec_max_length, mat_max_width) <= N_x) && (num_vecs + (mat_info<Args>::height + ...) <= M_y);

    static constexpr bool total_area_in_bounds =
            (is_col_major && total_area_in_bounds_col_major) || (!is_col_major && total_area_in_bounds_row_major);

public:
    static constexpr bool value = accepted_types && total_area_in_bounds;
};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major, typename... Args>
struct mat_constructible_from : mat_constructible_from_impl<M_y, N_x, T, is_col_major, std::remove_cvref_t<Args>...>
{};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_initializer
{
public:
    template<typename... Args>
    static constexpr void init_from(Mat<M_y, N_x, T, is_col_major>& out, Args&&... args)
        requires(mat_constructible_from<M_y, N_x, T, is_col_major, Args...>::value)
    {
        std::size_t idx = 0;
        init_from_inner(idx, out, std::forward<Args>(args)...);
    }

private:
    static constexpr void init_from_inner(std::size_t& idx, Mat<M_y, N_x, T, is_col_major>& out)
    {
        (void)(idx);
        (void)(out);
    }
    template<std::size_t M, typename U>
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_col_major>& out,
                                          const Vec<M, U>& arg,
                                          auto&&... rest)
    {
        if constexpr (is_col_major) {
            out.col_set(idx++, Vec<M_y, T>{ arg });
        } else {
            out.row_set(idx++, Vec<N_x, T>{ arg });
        }
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M, typename U, std::size_t... Is>
        requires(sizeof...(Is) > 1)
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_col_major>& out,
                                          const Swizzled<Vec<sizeof...(Is), T>, M, U, Is...>& arg,
                                          auto&&... rest)
    {
        if constexpr (is_col_major) {
            out.col_set(idx++, Vec<M_y, T>{ arg });
        } else {
            out.row_set(idx++, Vec<N_x, T>{ arg });
        }
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M1_y, std::size_t N1_x>
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_col_major>& out,
                                          const Mat<M1_y, N1_x, T, is_col_major>& arg,
                                          auto&&... rest)
    {
        if constexpr (is_col_major) {
            for (std::size_t i = 0; i < arg.col_count(); i++) {
                out.col_set(idx + i, Vec<M_y, T>{ arg.col_get(i) });
            }
            idx += arg.col_count();
        } else {
            for (std::size_t i = 0; i < arg.row_count(); i++) {
                out.row_set(idx + i, Vec<N_x, T>{ arg.row_get(i) });
            }
            idx += arg.row_count();
        }
        init_from_inner(idx, out, rest...);
    }
};

template<std::size_t M_y, std::size_t N_x, typename T>
struct mat_printer<M_y, N_x, T, false>
{
    static std::ostream& print(std::ostream& out, const Mat<M_y, N_x, T, false>& mat)
    {
        std::stringstream s{};
        auto count_chars = [&s](const T& value) -> std::size_t {
            s.str("");
            s << value;
            return s.str().size();
        };
        std::array<int, mat.size()> lengths;
        std::transform(mat.range().begin(), mat.range().end(), lengths.begin(), count_chars);
        const auto max_length = *std::max_element(lengths.begin(), lengths.end());

        out << "[";
        for (std::size_t y = 0; y < M_y; y++) {
            if (y != 0) {
                out << " ";
            }
            out << "[ ";
            for (std::size_t x = 0; x < N_x; x++) {
                const auto index = Mat<M_y, N_x, T, false>::map_index(y, x);
                const auto lpad = (max_length - lengths[index]) / 2;
                const auto rpad = max_length - (lpad + lengths[index]);

                out << std::setw(lpad) << "";
                out << mat[index];
                out << std::setw(rpad) << "";
                if (x + 1 != N_x) {
                    out << ", ";
                }
            }
            out << "]";
            if (y + 1 != M_y) {
                out << std::endl;
            }
        }
        out << "]" << std::endl;

        return out;
    }
};

template<std::size_t M_y, std::size_t N_x, typename T>
struct mat_printer<M_y, N_x, T, true>
{
    static std::ostream& print(std::ostream& out, const Mat<M_y, N_x, T, true>& mat)
    {
        std::stringstream s{};
        auto count_chars = [&s](const T& value) -> std::size_t {
            s.str("");
            s << value;
            return s.str().size();
        };
        std::array<int, mat.size()> lengths;
        std::transform(mat.range().begin(), mat.range().end(), lengths.begin(), count_chars);
        const auto max_length = *std::max_element(lengths.begin(), lengths.end());

        out << "[";
        for (std::size_t y = 0; y < M_y; y++) {
            if (y != 0) {
                out << " ";
            }
            for (std::size_t x = 0; x < N_x; x++) {
                if (y == 0) {
                    out << "[ ";
                } else {
                    out << "  ";
                }
                const auto index = Mat<M_y, N_x, T, true>::map_index(y, x);
                const auto lpad = (max_length - lengths[index]) / 2;
                const auto rpad = max_length - (lpad + lengths[index]);

                out << std::setw(lpad) << "";
                out << mat[index];
                out << std::setw(rpad) << "";
                if (y + 1 != M_y) {
                    out << ",  ";
                } else {
                    out << " ]";
                    if (x + 1 != N_x) {
                        out << ",";
                    }
                }
            }
            if (y + 1 != M_y) {
                out << std::endl;
            }
        }
        out << "]" << std::endl;

        return out;
    }
};

/// @endcond

} // namespace detail

} // namespace asciirast::math
