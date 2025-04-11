/**
 * @file Mat.h
 * @brief File with definition of a (basic) matrix class
 * @todo determinant and inverse function
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

namespace detail {

/**
 * @brief Trait to check if matrix can constructed from arguments.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the matrix is in column major
 * @tparam Args             Arguments consisting of vectors and smaller
 *                          matrices.
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major, typename... Args>
struct mat_constructible_from;

/**
 * @brief Helper class to initialize matrix from arguments.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_initializer;

/**
 * @brief Helper class to print matrix.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_col_major     Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_printer;

} // namespace detail

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

protected:
    std::array<T, M_y * N_x> m_elements{}; ///< 1D array of elements

public:
    /**
     * @brief Map a 2d matrix index to a 1d array index.
     */
    static constexpr std::size_t map_index(const std::size_t y, const std::size_t x)
    {
        if constexpr (is_col_major) {
            return M_y * x + y;
        } else {
            return N_x * y + x;
        }
    }

    /**
     * @brief Generate the identity matrix. Only for squared matricies.
     */
    static constexpr Mat identity()
        requires(N_x == M_y)
    {
        return Mat{ T{ 1 } };
    }

    /**
     * @brief Create matrix from columns
     */
    template<typename... Args>
        requires(constructible_from_cols_v<Args...>)
    static constexpr Mat from_cols(Args&&... args)
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
     * @brief Create matrix from rows
     */
    template<typename... Args>
        requires(constructible_from_rows_v<Args...>)
    static constexpr Mat from_rows(Args&&... args)
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
     * @brief Default constructor. Set all values to zero.
     */
    constexpr Mat() {};

    /**
     * @brief Construct matrix from a mix smaller of matrices and vectors,
     * padding the rest of the elements with zeroes.
     */
    template<typename... Args>
        requires(sizeof...(Args) > 0)
    constexpr Mat(Args&&... args)
        requires(constructible_from_args_v<Args...>)
    {
        using initializer = detail::mat_initializer<M_y, N_x, T, is_col_major>;

        initializer::init_from(*this, std::forward<Args>(args)...);
    };

    /**
     * @brief Initiate diagonal elements to some value
     */
    template<typename U>
        requires(std::is_convertible_v<U, T>)
    explicit constexpr Mat(const U diagonal_element)
    {
        if constexpr (is_col_major) {
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    (*this)[y, x] = (y == x) ? static_cast<T>(diagonal_element) : T{ 0 };
                }
            }
        } else {
            for (std::size_t y = 0; y < M_y; y++) {
                for (std::size_t x = 0; x < N_x; x++) {
                    (*this)[y, x] = (y == x) ? static_cast<T>(diagonal_element) : T{ 0 };
                }
            }
        }
    }

public:
    /**
     * @brief Get number of rows
     */
    static constexpr std::size_t row_count() { return M_y; }

    /**
     * @brief Get number of columns
     */
    static constexpr std::size_t col_count() { return N_x; }

    /**
     * @brief Get number of elements
     */
    static constexpr std::size_t size() { return M_y * N_x; }

    /**
     * @brief Get pointer over underlying data
     */
    constexpr T* data() { return &m_elements[0]; }

    /**
     * @brief Get pointer over underlying data
     */
    constexpr const T* data() const { return &m_elements[0]; }

    /**
     * @brief Get underlying array
     */
    constexpr std::array<T, M_y * N_x>& array() { return m_elements; }

    /**
     * @brief Get underlying array
     */
    constexpr const std::array<T, M_y * N_x>& array() const { return m_elements; }

    /**
     * @brief Index the underlying array
     */
    constexpr T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the underlying array
     */
    constexpr T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the matrix
     */
    constexpr T& operator[](const std::size_t y, const std::size_t x)
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Index the matrix
     */
    constexpr T operator[](const std::size_t y, const std::size_t x) const
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Print the matrix
     */
    friend std::ostream& operator<<(std::ostream& out, const Mat& m)
    {
        using mat_printer = detail::mat_printer<M_y, N_x, T, is_col_major>;

        return mat_printer::print(out, m);
    }

    /**
     * @brief Get the transpose of the matrix
     */
    constexpr Mat<N_x, M_y, T, is_col_major> transposed() const
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
     * @brief Get y'th row
     */
    constexpr Vec<N_x, T> row_get(const std::size_t y) const
    {
        assert(y < M_y && "index is inside bounds");

        Vec<N_x, T> res{};
        for (std::size_t x = 0; x < N_x; x++) {
            res[x] = (*this)[y, x];
        }
        return res;
    }

    /**
     * @brief Set y'th row
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
     * @brief Get x'th column
     */
    constexpr Vec<M_y, T> col_get(const std::size_t x) const
    {
        assert(x < N_x && "index is inside bounds");

        Vec<M_y, T> res{};
        for (std::size_t y = 0; y < M_y; y++) {
            res[y] = (*this)[y, x];
        }
        return res;
    }

    /**
     * @brief Set x'th column
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
     * @brief Check if equal to other matrix in terms of bitwise equality
     */
    friend constexpr bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T>)
    {
        bool res = true;
        for (std::size_t i = 0; i < size(); i++) {
            res &= lhs[i] == rhs[i];
        }
        return res;
    }

    /**
     * @brief Check if approximately equal to other matrix
     */
    friend constexpr bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        bool res = true;
        for (std::size_t i = 0; i < size(); i++) {
            res &= almost_equal(lhs[i], rhs[i]);
        }
        return res;
    }

    /**
     * @brief Check if approximately not equal to other matrix
     */
    friend constexpr bool operator!=(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

public:
    /**
     * @brief In-place component-wise addition with matrix
     */
    constexpr Mat& operator+=(const Mat& that)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with matrix
     */
    constexpr Mat& operator-=(const Mat& that)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with matrix
     */
    constexpr Mat& operator*=(const Mat& that)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= that[i];
        }
        return *this;
    }

    /**
     * @brief In-place matrix-scalar multiplication
     */
    constexpr Mat& operator*=(const T scalar)
    {
        for (std::size_t i = 0; i < size(); i++) {
            (*this)[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place matrix-scalar division
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
     * @brief Unary minus matrix operator
     */
    constexpr Mat operator-() const
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = -(*this)[i];
        }
        return res;
    }

    /**
     * @brief Matrix-matrix component-wise addition
     */
    friend constexpr Mat operator+(const Mat& lhs, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] + rhs[i];
        }
        return res;
    }

    /**
     * @brief Matrix-matrix component-wise subtraction
     */
    friend constexpr Mat operator-(const Mat& lhs, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] - rhs[i];
        }
        return res;
    }

    /**
     * @brief Scalar-matrix multiplication
     */
    friend constexpr Mat operator*(const T scalar, const Mat& rhs)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = scalar * rhs[i];
        }
        return res;
    }

    /**
     * @brief Matrix-scalar multiplication
     */
    friend constexpr Mat operator*(const Mat& lhs, const T scalar)
    {
        Mat res{};
        for (std::size_t i = 0; i < size(); i++) {
            res[i] = lhs[i] * scalar;
        }
        return res;
    }

    /**
     * @brief Matrix-scalar division
     */
    friend constexpr Mat operator/(const Mat& lhs, const T scalar)
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
     * @brief Perform matrix-matrix multiplication.
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. But the transposed matrix does take extra time to
     * create.
     */
    friend constexpr Mat<M_y, M_y, T, is_col_major> operator*(const Mat<M_y, N_x, T, is_col_major>& lhs,
                                                              const Mat<N_x, M_y, T, is_col_major>& rhs)
    {
        Mat<M_y, M_y, T, is_col_major> res{};

        if constexpr (is_col_major) {
            const auto lhs_T = lhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < rhs.col_count(); i++) {
                for (std::size_t j = 0; j < lhs_T.col_count(); j++) {
                    auto lb = &rhs.data()[M_y * i];
                    auto le = &rhs.data()[M_y * (i + 1)];
                    auto rb = &lhs_T.data()[M_y * j];

                    res[idx++] = std::inner_product(lb, le, rb, T{});
                }
            }
        } else {
            const auto rhs_T = rhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < lhs.row_count(); i++) {
                for (std::size_t j = 0; j < rhs_T.row_count(); j++) {
                    auto lb = &lhs.data()[N_x * i];
                    auto le = &lhs.data()[N_x * (i + 1)];
                    auto rb = &rhs_T.data()[N_x * j];

                    res[idx++] = std::inner_product(lb, le, rb, T{});
                }
            }
        }
        return res;
    }

    /**
     * @brief Perform matrix-vector multiplication
     *
     * @todo simd'ify col_major matrix-vec multiplication in c++26
     */
    friend constexpr Vec<M_y, T> operator*(const Mat<M_y, N_x, T, is_col_major>& mat, const Vec<N_x, T>& vec)
    {
        Vec<M_y, T> res{};

        if constexpr (is_col_major) {
            std::size_t idx = 0;
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    res[y] += mat[idx++] * vec[x];
                }
            }
        } else {
            for (std::size_t y = 0; y < M_y; y++) {
                auto lb = &mat.data()[N_x * y];
                auto le = &mat.data()[N_x * (y + 1)];
                auto rb = vec.array().begin();

                res[y] = std::inner_product(lb, le, rb, T{});
            }
        }
        return res;
    }
};

namespace detail {

template<typename TT>
struct mat_info_impl
{
    using value_type = void;                 ///< default value type
    static constexpr bool value = false;     ///< default type
    static constexpr std::size_t height = 0; ///< default height
    static constexpr std::size_t width = 0;  ///< default width
};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_info_impl<Mat<M_y, N_x, T, is_col_major>>
{
    using value_type = T;                      ///< value type
    static constexpr bool value = true;        ///< whether the type is a matrix
    static constexpr std::size_t height = M_y; ///< matrix height
    static constexpr std::size_t width = N_x;  ///< matrix width
};

/**
 * @brief Matrix information trait
 */
template<typename TT>
struct mat_info : mat_info_impl<std::remove_cvref_t<TT>>
{};

/**
 * @brief Trait to check if matrix can constructed from arguments.
 */
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
    /**
     * @brief Initialize matrix from arguments
     *
     * @param out   The matrix to initialize
     * @param args  The arguments
     */
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
    /**
     * @brief Print row-major matrix
     */
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
    /**
     * @brief Print column-major matrix
     */
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

} // namespace detail

} // namespace asciirast::math
