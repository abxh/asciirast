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
#include <ostream>
#include <ranges>
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
 * matrices.
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
 * @brief Helper class to prstd::size_t matrix.
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
protected:
    std::array<T, M_y * N_x> m_elements; ///< 1D array of elements

public:
    /**
     * @brief Map a 2d matrix index to a 1d array index.
     */
    static std::size_t map_index(const std::size_t y, const std::size_t x)
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
    static Mat identity()
        requires(N_x == M_y)
    {
        return Mat{ T{ 1 } };
    }

    /**
     * @brief Create matrix from columns
     */
    template<typename... Args>
        requires(detail::vec_info<Args>::value && ...)
    static Mat from_cols(Args&&... args)
        requires((0 < sizeof...(Args) && sizeof...(Args) <= N_x) && ((detail::vec_info<Args>::size <= M_y) && ...))
    {
        if constexpr (is_col_major) {
            return Mat<M_y, N_x, T, is_col_major>{ std::forward<Args>(args)... };
        } else {
            return Mat<N_x, M_y, T, is_col_major>{ std::forward<Args>(args)... }.transposed();
        }
    }

    /**
     * @brief Create matrix from rows
     */
    template<typename... Args>
        requires(detail::vec_info<Args>::value && ...)
    static Mat from_rows(Args&&... args)
        requires((0 < sizeof...(Args) && sizeof...(Args) <= M_y) && ((detail::vec_info<Args>::size <= N_x) && ...))
    {
        if constexpr (is_col_major) {
            return Mat<N_x, M_y, T, is_col_major>{ std::forward<Args>(args)... }.transposed();
        } else {
            return Mat<M_y, N_x, T, is_col_major>{ std::forward<Args>(args)... };
        }
    }

public:
    /**
     * @brief Default constructor. Set all values to zero.
     */
    Mat()
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
        for (const auto [y, x] : std::views::cartesian_product(std::views::iota(0U, M_y), std::views::iota(0U, N_x))) {
            (*this)[y, x] = (y == x) ? diagonal_element : T{ 0 };
        }
    }

    /**
     * @brief Construct matrix from a mix smaller of matrices and vectors,
     * padding the rest of the elements with zeroes.
     */
    template<typename... Args>
        requires(sizeof...(Args) > 0 && detail::not_a_single_value<T, Args...>::value)
    explicit Mat(Args&&... args)
        requires(detail::mat_constructible_from<M_y, N_x, T, is_col_major, Args...>::value)
    {
        detail::mat_initializer<M_y, N_x, T, is_col_major>::init_from(*this, std::forward(args)...);
    };

    /**
     * @brief Construct matrix from input range.
     */
    explicit Mat(std::ranges::input_range auto&& it)
    {
        assert(this->size() == std::ranges::distance(it) && "range size is same as matrix size");

        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), it)) {
            auto [dest, src] = t;

            dest = src;
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
    T* data() { return &m_elements[0]; }

    /**
     * @brief Get pointer over underlying data
     */
    const T* data() const { return &m_elements[0]; }

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
    T& operator[](const std::size_t y, const std::size_t x)
    {
        assert(y < M_y && "index is inside bounds");
        assert(x < N_x && "index is inside bounds");

        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Index the matrix
     */
    T operator[](const std::size_t y, const std::size_t x) const
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
        return detail::mat_printer<M_y, N_x, T, is_col_major>::print(out, m);
    }

    /**
     * @brief Get the transpose of the matrix
     */
    Mat<N_x, M_y, T, is_col_major> transposed() const
    {
        Mat<N_x, M_y, T, is_col_major> out{};

        for (const auto [y, x] : std::views::cartesian_product(std::views::iota(0U, M_y), std::views::iota(0U, N_x))) {
            out[x, y] = (*this)[y, x];
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

        for (const std::tuple<T&, const T> t : std::views::zip(this->row_range(y), v.range())) {
            auto [dest, src] = t;

            dest = src;
        }
        return *this;
    }

    /**
     * @brief Get x'th column
     */
    Vec<M_y, T> col_get(const std::size_t x) const
    {
        assert(x < N_x && "index is inside bounds");

        return Vec<M_y, T>{ this->col_range(x) };
    }

    /**
     * @brief Set x'th column
     */
    Mat col_set(const std::size_t x, const Vec<M_y, T>& v)
    {
        assert(x < M_y && "index is inside bounds");

        for (const std::tuple<T&, const T> t : std::views::zip(this->col_range(x), v.range())) {
            auto [dest, src] = t;

            dest = src;
        }
        return *this;
    }

public:
    /**
     * @brief Check if equal to other matrix in terms of bitwise equality
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
        requires(std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        const auto func = [](const T lhs, const T rhs) -> bool { return almost_equals<T>(lhs, rhs); };

        return std::ranges::equal(lhs.range(), rhs.range(), func);
    }

    /**
     * @brief Check if approximately not equal to other matrix
     */
    friend bool operator!=(const Mat& lhs, const Mat& rhs)
        requires(std::is_integral_v<T> || std::is_same_v<T, float> || std::is_same_v<T, double>)
    {
        return !(lhs == rhs);
    }

public:
    /**
     * @brief In-place component-wise addition with matrix
     */
    Mat& operator+=(const Mat& that)
    {
        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest += src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise subtraction with matrix
     */
    Mat& operator-=(const Mat& that)
    {
        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest -= src;
        }
        return *this;
    }

    /**
     * @brief In-place component-wise multiplication with matrix
     */
    Mat& operator*=(const Mat& that)
    {
        for (const std::tuple<T&, const T> t : std::views::zip(this->range(), that.range())) {
            auto [dest, src] = t;

            dest *= src;
        }
        return *this;
    }

    /**
     * @brief In-place matrix-scalar multiplication
     */
    Mat& operator*=(const T scalar)
    {
        for (T& x : this->range()) {
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

        for (T& x : this->range()) {
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
        const auto func = [](const T x) -> T { return -x; };
        const auto view = std::ranges::views::transform(this->range(), func);

        return Mat{ view };
    }

    /**
     * @brief Matrix-matrix component-wise addition
     */
    friend Mat operator+(const Mat& lhs, const Mat& rhs)
    {
        const auto func = std::plus();
        const auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Mat{ view };
    }

    /**
     * @brief Matrix-matrix component-wise subtraction
     */
    friend Mat operator-(const Mat& lhs, const Mat& rhs)
    {
        const auto func = std::minus();
        const auto view = std::views::zip_transform(func, lhs.range(), rhs.range());

        return Mat{ view };
    }

    /**
     * @brief Scalar-matrix multiplication
     */
    friend Mat operator*(const T scalar, const Mat& rhs)
    {
        const auto func = [=](const T x) -> T { return scalar * x; };
        const auto view = std::views::transform(rhs.range(), func);

        return Mat{ view };
    }

    /**
     * @brief Matrix-scalar multiplication
     */
    friend Mat operator*(const Mat& lhs, const T scalar)
    {
        const auto func = [=](const T x) -> T { return x * scalar; };
        const auto view = std::views::transform(lhs.range(), func);

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

        const auto func = [=](const T x) -> T { return x / scalar; };
        const auto view = std::views::transform(lhs.range(), func);

        return Mat{ view };
    }

public:
    /**
     * @brief Perform matrix-matrix multiplication.
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. But the transposed matrix does take extra time to create.
     */
    friend Mat<M_y, M_y, T, is_col_major> operator*(const Mat<M_y, N_x, T, is_col_major>& lhs,
                                                    const Mat<N_x, M_y, T, is_col_major>& rhs)
    {
        Mat<M_y, M_y, T, is_col_major> res{};

        if constexpr (is_col_major) {
            const auto lhs_T = lhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < rhs.col_count(); i++) {
                for (std::size_t j = 0; j < lhs_T.col_count(); j++) {
                    res[idx++] = std::ranges::fold_left(
                            std::ranges::views::zip_transform(std::multiplies(), rhs.col_range(i), lhs_T.col_range(j)),
                            T{ 0 },
                            std::plus());
                }
            }
        } else {
            const auto rhs_T = rhs.transposed();

            std::size_t idx = 0;
            for (std::size_t i = 0; i < lhs.row_count(); i++) {
                for (std::size_t j = 0; j < rhs_T.row_count(); j++) {
                    res[idx++] = std::ranges::fold_left(
                            std::ranges::views::zip_transform(std::multiplies(), lhs.row_range(i), rhs_T.row_range(j)),
                            T{ 0 },
                            std::plus());
                }
            }
        }
        return res;
    }

    /**
     * @brief Perform matrix-vector multiplication
     */
    friend Vec<M_y, T> operator*(const Mat<M_y, N_x, T, is_col_major>& mat, const Vec<N_x, T>& vec)
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
            for (std::size_t y = 0; y < mat.row_count(); y++) {
                res[y] = std::ranges::fold_left(
                        std::ranges::views::zip_transform(std::multiplies(), mat.row_range(y), vec.range()),
                        T{ 0 },
                        std::plus());
            }
        }
        return res;
    }

private:
    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y)
        requires(is_col_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        const auto func = [this, y](const std::size_t x) -> T& { return m_elements[map_index(y, x)]; };

        return std::ranges::views::iota(0U, N_x) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y) const
        requires(is_col_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        const auto func = [this, y](const std::size_t x) -> T { return m_elements[map_index(y, x)]; };

        return std::ranges::views::iota(0U, N_x) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y)
        requires(!is_col_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), N_x * y) | std::ranges::views::take(N_x);
    }

    /**
     * @brief Get range over elements at row y.
     */
    std::ranges::view auto row_range(const std::size_t y) const
        requires(!is_col_major)
    {
        assert(y < this->row_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), N_x * y) | std::ranges::views::take(N_x);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto col_range(const std::size_t x)
        requires(!is_col_major)
    {
        assert(x < this->col_count() && "index is inside bounds");

        const auto func = [this, x](const std::size_t y) -> T& { return m_elements[map_index(y, x)]; };

        return std::ranges::views::iota(0U, M_y) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto col_range(const std::size_t x) const
        requires(!is_col_major)
    {
        assert(x < this->col_count() && "index is inside bounds");

        const auto func = [this, x](const std::size_t y) -> T { return m_elements[map_index(y, x)]; };

        return std::ranges::views::iota(0U, M_y) | std::ranges::views::transform(func);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto col_range(const std::size_t x)
        requires(is_col_major)
    {
        assert(x < this->col_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), M_y * x) | std::ranges::views::take(M_y);
    }

    /**
     * @brief Get range over elements at column x.
     */
    std::ranges::view auto col_range(const std::size_t x) const
        requires(is_col_major)
    {
        assert(x < this->col_count() && "index is inside bounds");

        return std::ranges::views::drop(this->range(), M_y * x) | std::ranges::views::take(M_y);
    }
};

namespace detail {

template<typename TT>
struct mat_info_impl
{
    using value_type                    = void;  ///< value type
    static constexpr bool value         = false; ///< whether the type is a matrix
    static constexpr std::size_t height = 0;     ///< matrix height
    static constexpr std::size_t width  = 0;     ///< matrix width
};

template<std::size_t M_y, std::size_t N_x, typename T, bool is_col_major>
struct mat_info_impl<Mat<M_y, N_x, T, is_col_major>>
{
    using value_type                    = T;    ///< value type
    static constexpr bool value         = true; ///< whether the type is a matrix
    static constexpr std::size_t height = M_y;  ///< matrix height
    static constexpr std::size_t width  = N_x;  ///< matrix width
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

    static constexpr bool total_area_in_bounds = (is_col_major && (std::max(vec_max_length, mat_max_height) <= M_y) &&
                                                  (num_vecs + (mat_info<Args>::width + ...) <= N_x)) ||
                                                 (!is_col_major && (std::max(vec_max_length, mat_max_width) <= N_x) &&
                                                  (num_vecs + (mat_info<Args>::height + ...) <= M_y));

public:
    static constexpr bool value = accepted_types && total_area_in_bounds; ///< whether the arguments are of acceptable
                                                                          ///< types and total area is inside bounds
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
        if constexpr (is_col_major) {
            for (const std::size_t i : std::views::iota(idx, out.col_count())) {
                out.col_set(i, Vec<M_y, T>{ T{ 0 } });
            }
        } else {
            for (const std::size_t i : std::views::iota(idx, out.row_count())) {
                out.row_set(i, Vec<N_x, T>{ T{ 0 } });
            }
        }
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
            for (const std::size_t i : std::ranges::views::iota(0U, arg.col_count())) {
                out.col_set(idx + i, Vec<M_y, T>{ arg.col_get(i) });
            }
            idx += arg.col_count();
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
                const auto lpad  = (max_length - lengths[index]) / 2;
                const auto rpad  = max_length - (lpad + lengths[index]);

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
                const auto lpad  = (max_length - lengths[index]) / 2;
                const auto rpad  = max_length - (lpad + lengths[index]);

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
