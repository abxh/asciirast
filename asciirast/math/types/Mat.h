/**
 * @file Mat.h
 * @brief File with definition of a (basic) matrix class
 */

#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <iomanip>
#include <ostream>
#include <ranges>
#include <sstream>
#include <type_traits>

#include "Vec.h"

namespace asciirast::math {

/**
 * @brief Trait to check if matrix can constructed from arguments.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_column_major  Whether the matrix is in column major
 * @tparam Args             Arguments consisting of vectors and smaller
 * matrices.
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major, typename... Args>
struct mat_constructible_from;

/**
 * @brief Helper class to initialize matrix from arguments.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_column_major  Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major>
struct mat_initializer;

/**
 * @brief Helper class to prstd::size_t matrix.
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_column_major  Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major>
struct mat_printer;

/**
 * @brief Math matrix class
 *
 * @tparam M_y              Number of rows
 * @tparam N_x              Number of columns
 * @tparam T                Type of elements
 * @tparam is_column_major  Whether the matrix is in column major
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
class Mat
{
protected:
    std::array<T, M_y * N_x> m_elements; ///< 1D array of elements

public:
    /**
     * @brief Map a 2d matrix index to a 1d array index.
     */
    static std::size_t map_2d_index(const std::size_t y, const std::size_t x)
    {
        if constexpr (is_column_major) {
            return M_y * x + y;
        } else {
            return N_x * y + x;
        }
    }

    /**
     * @brief Generate the identity matrix. Only for squared matricies.
     */
    static Mat identity()
        requires(N_x == M_y)
    {
        return Mat{ T{ 1 } };
    }

    /**
     * @brief Create matrix from columns
     */
    template<typename... Args>
        requires(vec_info<Args>::value && ...)
    static Mat from_columns(const Args&... args)
        requires((0 < sizeof...(Args) && sizeof...(Args) <= N_x) && ((vec_info<Args>::size <= M_y) && ...))
    {
        if constexpr (is_column_major) {
            return Mat<M_y, N_x, T, is_column_major>{ args... };
        } else {
            return Mat<N_x, M_y, T, is_column_major>{ args... }.transposed();
        }
    }

    /**
     * @brief Create matrix from rows
     */
    template<typename... Args>
        requires(vec_info<Args>::value && ...)
    static Mat from_rows(const Args&... args)
        requires((0 < sizeof...(Args) && sizeof...(Args) <= M_y) && ((vec_info<Args>::size <= N_x) && ...))
    {
        if constexpr (is_column_major) {
            return Mat<N_x, M_y, T, is_column_major>{ args... }.transposed();
        } else {
            return Mat<M_y, N_x, T, is_column_major>{ args... };
        }
    }

public:
    /**
     * @brief Default constructor. Set all values to zero.
     */
    constexpr Mat()
    {
        for (T& x : this->range()) {
            x = T{ 0 };
        }
    }

    /**
     * @brief Initiate diagonal elements to some value
     */
    explicit Mat(const T diagonal_element)
    {
        for (T& x : this->diagonal_range()) {
            x = diagonal_element;
        }
    }

    /**
     * @brief Construct vector from a mix of values and (smaller) vectors,
     * padding the rest of the vector with zeroes.
     */
    template<typename... Args>
        requires(not_single_value<T, Args...>::value)
    explicit Mat(const Args&... args)
        requires(mat_constructible_from<M_y, N_x, T, is_column_major, Args...>::value)
    {
        mat_initializer<M_y, N_x, T, is_column_major>::init_from(*this, args...);
    };

    /**
     * @brief Construct matrix from input range.
     */
    explicit Mat(std::ranges::input_range auto&& it)
    {
        assert(this->size() == std::ranges::distance(it) && "range size is same as matrix size");
        for (auto [x, y] : std::views::zip(this->range(), it)) {
            x = y;
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
    static constexpr std::size_t column_count() { return N_x; }

    /**
     * @brief Get number of elements
     */
    static constexpr std::size_t size() { return M_y * N_x; }

    /**
     * @brief Get pointer over underlying data
     */
    T* data() { return &m_elements[0]; }

    /**
     * @brief Index the underlying array
     */
    T& operator[](const std::size_t i)
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the underlying array
     */
    T operator[](const std::size_t i) const
    {
        assert(i < this->size() && "index is inside bounds");

        return m_elements[i];
    }

    /**
     * @brief Index the matrix
     */
    T& operator()(const std::size_t y, const std::size_t x)
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_2d_index(y, x)];
    }

    /**
     * @brief Index the matrix
     */
    T operator()(const std::size_t y, const std::size_t x) const
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_2d_index(y, x)];
    }

    /**
     * @brief Print the matrix
     */
    friend std::ostream& operator<<(std::ostream& out, const Mat& m)
    {
        return mat_printer<M_y, N_x, T, is_column_major>::print(out, m);
    }

    /**
     * @brief Get the transpose of the matrix
     */
    Mat<N_x, M_y, T, is_column_major> transposed() const
    {
        Mat<N_x, M_y, T, is_column_major> out{};

        auto indicies = std::views::cartesian_product(std::views::iota(0U, M_y), std::views::iota(0U, N_x));

        for (auto [y, x] : indicies) {
            out(x, y) = (*this)(y, x);
        }
        return out;
    }

    /**
     * @brief Get range over matrix elements in the current storage order.
     */
    std::ranges::view auto range() { return std::ranges::views::all(m_elements); }

    /**
     * @brief Get range over matrix elements in the current storage order.
     */
    std::ranges::view auto range() const { return std::ranges::views::all(m_elements); }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y)
        requires(is_column_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        auto func = [this, y](const std::size_t x) -> T& { return m_elements[map_2d_index(y, x)]; };

        return std::ranges::views::iota(0U, N_x) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y) const
        requires(is_column_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        auto func = [this, y](const std::size_t x) -> T { return m_elements[map_2d_index(y, x)]; };

        return std::ranges::views::iota(0U, N_x) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y)
        requires(!is_column_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), N_x * y) | std::ranges::views::take(N_x);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y) const
        requires(!is_column_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), N_x * y) | std::ranges::views::take(N_x);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto column_range(const std::size_t x)
        requires(!is_column_major)
    {
        assert(x < this->column_count() && "index is inside bounds");

        auto func = [this, x](const std::size_t y) -> T& { return m_elements[map_2d_index(y, x)]; };

        return std::ranges::views::iota(0U, M_y) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto column_range(const std::size_t x) const
        requires(!is_column_major)
    {
        assert(x < this->column_count() && "index is inside bounds");

        auto func = [this, x](const std::size_t y) -> T { return m_elements[map_2d_index(y, x)]; };

        return std::ranges::views::iota(0U, M_y) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto column_range(const std::size_t x)
        requires(is_column_major)
    {
        assert(x < this->column_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), M_y * x) | std::ranges::views::take(M_y);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto column_range(const std::size_t x) const
        requires(is_column_major)
    {
        assert(x < this->column_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), M_y * x) | std::ranges::views::take(M_y);
    }

    /**
     * @brief Get range over diagonal elements.
     * @todo Support other diagonals than the main diagonal?
     */
    std::ranges::view auto diagonal_range()
    {
        auto func = [this](const std::size_t i) -> T& { return m_elements[map_2d_index(i, i)]; };

        return std::ranges::views::iota(0U, std::min(N_x, M_y)) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over diagonal elements.
     */
    std::ranges::view auto diagonal_range() const
    {
        auto func = [this](const std::size_t i) -> T { return m_elements[map_2d_index(i, i)]; };

        return std::ranges::views::iota(0U, std::min(N_x, M_y)) | std::ranges::views::transform(func);
    }

public:
    /**
     * @brief Get y'th row
     */
    Vec<N_x, T> row_get(const std::size_t y) const
    {
        assert(y < M_y && "index is inside bounds");

        return Vec<N_x, T>{ this->row_range(y) };
    }

    /**
     * @brief Set y'th row
     */
    Mat row_set(const std::size_t y, const Vec<N_x, T>& v)
    {
        assert(y < M_y && "index is inside bounds");

        auto view = std::views::zip(this->row_range(y), v.range());

        for (auto [x, y] : view) {
            x = y;
        }
        return *this;
    }

    /**
     * @brief Get x'th column
     */
    Vec<M_y, T> column_get(const std::size_t x) const
    {
        assert(x < N_x && "index is inside bounds");

        return Vec<M_y, T>{ this->column_range(x) };
    }

    /**
     * @brief Set x'th column
     */
    Mat column_set(const std::size_t x, const Vec<M_y, T>& v)
    {
        assert(x < M_y && "index is inside bounds");

        auto view = std::views::zip(this->column_range(x), v.range());

        for (auto [x, y] : view) {
            x = y;
        }
        return *this;
    }

public:
    /**
     * @brief Check if equal to other matrix
     */
    friend bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T>)
    {
        return std::ranges::equal(lhs.range(), rhs.range());
    }

    /**
     * @brief Check if approximately equal to other matrix
     */
    friend bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_same_v<T, float>)
    {
        auto func = [](const T lhs, const T rhs) -> bool { return almost_equals(lhs, rhs, 9); };

        return std::ranges::equal(lhs.range(), rhs.range(), func);
    }

    /**
     * @brief Check if approximately equal to other matrix
     */
    friend bool operator==(const Mat& lhs, const Mat& rhs)
        requires(std::is_same_v<T, double>)
    {
        auto func = [](const T lhs, const T rhs) -> bool { return almost_equals(lhs, rhs, 17); };

        return std::ranges::equal(lhs.range(), rhs.range(), func);
    }

    /**
     * @brief Check if approximately not equal to other matrix
     */
    friend bool operator!=(const Mat& lhs, const Mat& rhs) { return !(lhs == rhs); }

public:
    /**
     * @brief In-place component-wise addition with matrix
     */
    Mat& operator+=(const Mat& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x += y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with matrix
     */
    Mat& operator-=(const Mat& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x -= y;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with matrix
     */
    Mat& operator*=(const Mat& that)
    {
        for (auto [x, y] : std::views::zip(this->range(), that.range())) {
            x *= y;
        }
        return *this;
    }

    /**
     * @brief In-place matrix-scalar multiplication
     */
    Mat& operator*=(const T scalar)
    {
        for (auto& x : this->range()) {
            x *= scalar;
        }
        return *this;
    }

    /**
     * @brief In-place matrix-scalar division
     */
    Mat& operator/=(const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != T{ 0 } && "non-zero division");
        }

        for (auto& x : this->range()) {
            x /= scalar;
        }
        return *this;
    }

public:
    /**
     * @brief Unary minus matrix operator
     */
    Mat operator-() const
    {
        auto func = [](const T x) -> T { return -x; };
        auto view = std::ranges::views::transform(this->range(), func);

        return Mat{ view };
    }

    /**
     * @brief Matrix-matrix component-wise addition
     */
    friend Mat operator+(const Mat& lhs, const Mat& rhs)
    {
        auto func = std::plus();
        auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Mat{ view };
    }

    /**
     * @brief Matrix-matrix component-wise subtraction
     */
    friend Mat operator-(const Mat& lhs, const Mat& rhs)
    {
        auto func = std::minus();
        auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Mat{ view };
    }

    /**
     * @brief Scalar-matrix multiplication
     */
    friend Mat operator*(const T scalar, const Mat& rhs)
    {
        auto func = [=](const T x) -> T { return scalar * x; };
        auto view = std::views::transform(rhs.range(), func);

        return Mat{ view };
    }

    /**
     * @brief Matrix-scalar multiplication
     */
    friend Mat operator*(const Mat& lhs, const T scalar)
    {
        auto func = [=](const T x) -> T { return x * scalar; };
        auto view = std::views::transform(lhs.range(), func);

        return Mat{ view };
    }

    /**
     * @brief Matrix-scalar division
     */
    friend Mat operator/(const Mat& lhs, const T scalar)
    {
        if constexpr (std::is_integral_v<T>) {
            assert(scalar != T{ 0 } && "non-zero division");
        }

        auto func = [=](const T x) -> T { return x / scalar; };
        auto view = std::views::transform(lhs.range(), func);

        return Mat{ view };
    }

public:
    /**
     * @brief Perform matrix-matrix multiplication.
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. But the transposed matrix does take extra time to create.
     */
    friend Mat<M_y, M_y, T, is_column_major> operator*(const Mat<M_y, N_x, T, is_column_major>& lhs,
                                                       const Mat<N_x, M_y, T, is_column_major>& rhs)
    {
        Mat<M_y, M_y, T, is_column_major> res{};

        if constexpr (is_column_major) {
            const auto lhs_T = lhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < rhs.column_count(); i++) {
                for (std::size_t j = 0; j < lhs_T.column_count(); j++) {
                    auto vl = rhs.column_range(i);
                    auto vr = lhs_T.column_range(j);
                    auto v = std::ranges::views::zip_transform(std::multiplies(), vl, vr);

                    res[idx++] = std::ranges::fold_left(v, T{ 0 }, std::plus());
                }
            }
        } else {
            const auto rhs_T = rhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < lhs.row_count(); i++) {
                for (std::size_t j = 0; j < rhs_T.row_count(); j++) {
                    auto vl = lhs.row_range(i);
                    auto vr = rhs_T.row_range(j);
                    auto v = std::ranges::views::zip_transform(std::multiplies(), vl, vr);

                    res[idx++] = std::ranges::fold_left(v, T{ 0 }, std::plus());
                }
            }
        }
        return res;
    }

    /**
     * @brief Perform matrix-vector multiplication
     */
    friend Vec<M_y, T> operator*(const Mat<M_y, N_x, T, is_column_major>& mat, const Vec<N_x, T>& vec)
    {
        Vec<M_y, T> res{};

        if constexpr (is_column_major) {
            std::size_t idx = 0;
            for (std::size_t x = 0; x < N_x; x++) {
                for (std::size_t y = 0; y < M_y; y++) {
                    res[y] += mat[idx++] * vec[x];
                }
            }
        } else {
            for (std::size_t y = 0; y < mat.row_count(); y++) {
                auto v = std::ranges::views::zip_transform(std::multiplies(), mat.row_range(y), vec.range());

                res[y] = std::ranges::fold_left(v, T{ 0 }, std::plus());
            }
        }
        return res;
    }
};

/**
 * @brief Matrix information trait
 */
template<typename TT>
struct mat_info
{
    using value_type = void;                 ///< value type
    static constexpr bool value = false;     ///< whether the type is a matrix
    static constexpr std::size_t height = 0; ///< matrix height
    static constexpr std::size_t width = 0;  ///< matrix width
};

/**
 * @brief Matrix information trait
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major>
struct mat_info<Mat<M_y, N_x, T, is_column_major>>
{
    using value_type = T;                      ///< value type
    static constexpr bool value = true;        ///< whether the type is a matrix
    static constexpr std::size_t height = M_y; ///< matrix height
    static constexpr std::size_t width = N_x;  ///< matrix width
};

/**
 * @brief Trait to check if matrix can constructed from arguments.
 */
template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major, typename... Args>
struct mat_constructible_from
{
private:
    static constexpr bool accepted_types = ((vec_info<Args>::value || mat_info<Args>::value) && ...);

    static constexpr std::size_t num_vecs = ((vec_info<Args>::value ? 1 : 0) + ...);

    static constexpr std::size_t vec_max_length = std::max({ vec_info<Args>::size... });

    static constexpr std::size_t mat_max_width = std::max({ mat_info<Args>::width... });

    static constexpr std::size_t mat_max_height = std::max({ mat_info<Args>::height... });

    static constexpr bool total_area_in_bounds =
            (is_column_major && (std::max(vec_max_length, mat_max_height) <= M_y) &&
             (num_vecs + (mat_info<Args>::width + ...) <= N_x)) ||
            (!is_column_major && (std::max(vec_max_length, mat_max_width) <= N_x) &&
             (num_vecs + (mat_info<Args>::height + ...) <= M_y));

public:
    static constexpr bool value = accepted_types && total_area_in_bounds; ///< whether the arguments are of acceptable
                                                                          ///< types and total area is inside bounds
};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_column_major>
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
    static constexpr void init_from(Mat<M_y, N_x, T, is_column_major>& out, const Args&... args)
        requires(mat_constructible_from<M_y, N_x, T, is_column_major, Args...>::value)
    {
        std::size_t idx = 0;
        init_from_inner(idx, out, args...);
        if constexpr (is_column_major) {
            for (const std::size_t i : std::views::iota(idx, out.column_count())) {
                out.column_set(i, Vec<M_y, T>{ T{ 0 } });
            }
        } else {
            for (const std::size_t i : std::views::iota(idx, out.row_count())) {
                out.row_set(i, Vec<N_x, T>{ T{ 0 } });
            }
        }
    }

private:
    static constexpr void init_from_inner(std::size_t& idx, Mat<M_y, N_x, T, is_column_major>& out)
    {
        (void)(idx);
        (void)(out);
    }
    template<std::size_t M, typename U>
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_column_major>& out,
                                          const Vec<M, U>& arg,
                                          const auto&... rest)
    {
        if constexpr (is_column_major) {
            out.column_set(idx++, Vec<M_y, T>{ arg });
        } else {
            out.row_set(idx++, Vec<N_x, T>{ arg });
        }
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M, typename U, std::size_t... Is>
        requires(sizeof...(Is) > 1)
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_column_major>& out,
                                          const Swizzled<Vec<sizeof...(Is), T>, M, U, Is...>& arg,
                                          const auto&... rest)
    {
        if constexpr (is_column_major) {
            out.column_set(idx++, Vec<M_y, T>{ arg });
        } else {
            out.row_set(idx++, Vec<N_x, T>{ arg });
        }
        init_from_inner(idx, out, rest...);
    }

    template<std::size_t M1_y, std::size_t N1_x>
    static constexpr void init_from_inner(std::size_t& idx,
                                          Mat<M_y, N_x, T, is_column_major>& out,
                                          const Mat<M1_y, N1_x, T, is_column_major>& arg,
                                          const auto&... rest)
    {
        if constexpr (is_column_major) {
            for (const std::size_t i : std::ranges::views::iota(0U, arg.column_count())) {
                out.column_set(idx + i, Vec<M_y, T>{ arg.column_get(i) });
            }
            idx += arg.column_count();
        } else {
            for (const std::size_t i : std::ranges::views::iota(0U, arg.row_count())) {
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
        auto count_chars = [&s](const T& value) -> auto {
            s.str("");
            s << value;
            return s.str().size();
        };
        std::array<int, mat.size()> lengths;
        std::transform(mat.range().begin(), mat.range().end(), lengths.begin(), count_chars);
        auto max_length = *std::max_element(lengths.begin(), lengths.end());

        out << "[";
        for (std::size_t y = 0; y < M_y; y++) {
            if (y != 0) {
                out << " ";
            }
            out << "[ ";
            for (std::size_t x = 0; x < N_x; x++) {
                auto index = Mat<M_y, N_x, T, false>::map_2d_index(y, x);
                auto lpad = (max_length - lengths[index]) / 2;
                auto rpad = max_length - (lpad + lengths[index]);

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
        auto count_chars = [&s](const T& value) -> auto {
            s.str("");
            s << value;
            return s.str().size();
        };
        std::array<int, mat.size()> lengths;
        std::transform(mat.range().begin(), mat.range().end(), lengths.begin(), count_chars);
        auto max_length = *std::max_element(lengths.begin(), lengths.end());

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
                auto index = Mat<M_y, N_x, T, true>::map_2d_index(y, x);
                auto lpad = (max_length - lengths[index]) / 2;
                auto rpad = max_length - (lpad + lengths[index]);

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

} // namespace asciirast::math
