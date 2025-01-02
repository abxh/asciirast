/**
 * @file Mat.h
 * @brief Matrix class.
 */

#pragma once

#include <algorithm>
#include <array>
#include <cassert>
#include <functional>
#include <iomanip>
#include <numeric>
#include <ostream>
#include <sstream>

#include "Vec.h"

namespace asciirast::math {

template <int M_y, int N_x, typename T>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
class Mat;

/**
 * @brief Matrix information trait
 */
template <typename TT, bool is_col_major>
struct mat_info {
    using value_type = void;
    static constexpr bool is_mat = false;
    static constexpr int rows = !is_col_major ? 1 : 0;
    static constexpr int columns = is_col_major ? 1 : 0;
};

/**
 * @brief Matrix information trait
 */
template <int M_y, int N_x, typename T, bool is_col_major>
struct mat_info<Mat<M_y, N_x, T>, is_col_major> {
    using value_type = T;
    static constexpr bool is_mat = true;
    static constexpr int rows = M_y;
    static constexpr int columns = N_x;
};

/**
 * @brief Is convertible matrix trait.
 * E.g. Mat<2, 2, float> is convertible to Mat<2, 2, double>.
 */
template <typename TT, typename U, bool is_col_major>
constexpr bool is_conv_mat_v =
        mat_info<TT, is_col_major>::is_mat &&
        non_narrowing_conversion<
                typename mat_info<TT, is_col_major>::value_type,
                U>;

/**
 * @brief MxN dimensional matrix.
 */
template <int M_y, int N_x, typename T>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
class Mat {
public:
    /**
     * @brief storage order definition
     *
     * Using column-major order by default. To learn more about the difference
     * between row-major and column major, see:
     * https://en.wikipedia.org/wiki/row-_and_column-major_order
     */
    static constexpr bool is_column_major = true;

    /**
     * @name Size info functions
     * @{
     * With size(), row_count(), and column_count()
     */
    static constexpr auto size() { return M_y * N_x; }
    static constexpr auto row_count() { return M_y; }
    static constexpr auto column_count() { return N_x; }
    ///@}

    /**
     * @brief Map a 2d matrix index to a 1d array index.
     */
    static constexpr int map_index(const int y, const int x) {
        if constexpr (is_column_major) {
            return M_y * x + y;
        } else {
            return N_x * y + x;
        }
    }

    /**
     * @brief Generate the identity matrix. Applies only for squared matricies.
     */
    static constexpr Mat identity()
        requires(N_x == M_y)
    {
        return Mat{T{1}};
    }

    /**
     * @brief Create matrix columns
     */
    template <typename... Vecs>
        requires(is_convertible_vec_v<Vecs, T> && ...)
    static constexpr Mat<M_y, N_x, T> from_columns(const Vecs&... vecs)
        requires((sizeof...(Vecs) <= N_x) &&
                 ((vec_info<Vecs>::size <= M_y) && ...))
    {
        if constexpr (is_column_major) {
            return Mat<M_y, N_x, T>{vecs...};
        } else {
            return Mat<N_x, M_y, T>{vecs...}.transposed();
        }
    }

    /**
     * @brief Create matrix from rows
     */
    template <typename... Vecs>
        requires(is_convertible_vec_v<Vecs, T> && ...)
    static constexpr Mat<M_y, N_x, T> from_rows(const Vecs&... vecs)
        requires((sizeof...(Vecs) <= M_y) &&
                 ((vec_info<Vecs>::size <= N_x) && ...))
    {
        if constexpr (is_column_major) {
            return Mat<N_x, M_y, T>{vecs...}.transposed();
        } else {
            return Mat<M_y, N_x, T>{vecs...};
        }
    }

public:
    std::array<T, Mat::size()> m_elements;  ///< Linear array of elements

    /**
     * @brief Default constructor. Set values to zero.
     */
    constexpr Mat() {
        for (int i = 0; i < Mat::size(); i++) {
            (*this)[i] = T{0};
        }
    }

    /**
     * @brief Initiate diagonal elements to some value
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    constexpr explicit Mat(const U diagonal_element) {
        auto f = [&](int y, int x) {
            (*this)(y, x) = (x == y) ? T{diagonal_element} : T{0};
        };
        Mat::for_each_indicies(f);
    }

    /**
     * @brief Initiate matrix from a mix of vectors and matrices. *Depending on
     * storage order*, sets either the column or row vectors. Fills out the rest
     * of the elements to zero, and diagonal elements to one.
     */
    template <typename... Args>
        requires((is_convertible_vec_v<Args, T> ||
                  is_conv_mat_v<Args, T, is_column_major>) &&
                 ...)
    constexpr explicit Mat(const Args&... args)
        requires((mat_info<Args, is_column_major>::rows + ...) <= M_y &&
                 (mat_info<Args, is_column_major>::columns + ...) <= N_x &&
                 (!is_column_major || ((vec_info<Args>::size <= M_y) && ...)) &&
                 (is_column_major || ((vec_info<Args>::size <= N_x) && ...)))
    {
        int idx = 0;
        set_from(idx, *this, args...);
        if constexpr (is_column_major) {
            for (int x = idx; x < N_x; x++) {
                for (int y = 0; y < M_y; y++) {
                    (*this)(y, x) = (x == y) ? T{1} : T{0};
                }
            }
        } else {
            for (int y = idx; y < M_y; y++) {
                for (int x = 0; x < N_x; x++) {
                    (*this)(y, x) = (x == y) ? T{1} : T{0};
                }
            }
        }
    }

public:
    /**
     * @brief Print the matrix (column major)
     */
    template <int M1_y, int N1_x, typename T1>
        requires(M1_y > 0 && N1_x > 0 && std::is_arithmetic_v<T1>)
    friend std::ostream& operator<<(std::ostream& out,
                                    const Mat<M1_y, N1_x, T1>& mat)
        requires(Mat<M1_y, N1_x, T1>::is_column_major == true);

    /**
     * @brief Print the matrix (row major)
     */
    template <int M1_y, int N1_x, typename T1>
        requires(M1_y > 0 && N1_x > 0 && std::is_arithmetic_v<T1>)
    friend std::ostream& operator<<(std::ostream& out,
                                    const Mat<M1_y, N1_x, T1>& mat)
        requires(Mat<M1_y, N1_x, T1>::is_column_major == false);

    /**
     * @name Basic iterator support
     * @{
     * Exposing the std::array iterator
     */
    using iterator = typename std::array<T, Mat::size()>::iterator;
    using const_iterator = typename std::array<T, Mat::size()>::const_iterator;

    iterator begin() { return m_elements.begin(); }
    iterator end() { return m_elements.end(); }
    const_iterator begin() const { return m_elements.begin(); }
    const_iterator end() const { return m_elements.end(); }
    ///@}

public:
    /**
     * @brief Index the array
     */
    constexpr T operator[](int i) const { return this->m_elements[i]; }

    /**
     * @brief Index the array
     */
    constexpr T& operator[](int i) { return this->m_elements[i]; }

    /**
     * @brief Index the matrix
     */
    constexpr T operator()(int y, int x) const {
        assert(0 <= y && y < M_y);
        assert(0 <= x && x < N_x);
        return this->m_elements[map_index(y, x)];
    }

    /**
     * @brief Index the matrix
     */
    constexpr T& operator()(int y, int x) {
        assert(0 <= y && y < M_y);
        assert(0 <= x && x < N_x);
        return this->m_elements[map_index(y, x)];
    }

    /**
     * @brief Get the transposed matrix
     */
    constexpr Mat<N_x, M_y, T> transposed() const {
        Mat<N_x, M_y, T> res{};
        auto f = [&](auto y, auto x) { res(y, x) = (*this)(x, y); };
        Mat<N_x, M_y, T>::for_each_indicies(f);
        return res;
    }

    /**
     * @brief Get x'th column
     */
    constexpr Vec<M_y, T> column_get(int x) const {
        assert(0 <= x && x < N_x);
        Vec<M_y, T> res{};
        if constexpr (is_column_major == true) {
            std::copy(this->column_begin(x), this->column_end(x), res.begin());
        } else {
            for (int y = 0; y < M_y; y++) {
                res[y] = (*this)(y, x);
            }
        }
        return res;
    }

    /**
     * @brief Get x'th column
     */
    constexpr Mat column_set(int x, const Vec<M_y, T>& v) {
        assert(0 <= x && x < N_x);
        if constexpr (is_column_major == true) {
            std::copy(v.begin(), v.end(), this->column_begin(x));
        } else {
            for (int y = 0; y < M_y; y++) {
                (*this)(y, x) = v[y];
            }
        }
        return *this;
    }

    /**
     * @brief Get y'th row
     */
    constexpr Vec<N_x, T> row_get(int y) const {
        assert(0 <= y && y < M_y);
        Vec<N_x, T> res{};
        if constexpr (is_column_major == false) {
            std::copy(this->row_begin(y), this->row_end(y), res.begin());
        } else {
            for (int x = 0; x < M_y; x++) {
                res[x] = (*this)(y, x);
            }
        }
        return res;
    }

    /**
     * @brief Set y'th row
     */
    constexpr Mat row_set(int y, const Vec<N_x, T>& v) {
        assert(0 <= y && y < M_y);
        if constexpr (is_column_major == false) {
            std::copy(v.begin(), v.end(), this->row_begin(y));
        } else {
            for (int x = 0; x < M_y; x++) {
                (*this)(y, x) = v[x];
            }
        }
        return *this;
    }

public:
    /**
     * @brief Perform matrix-matrix multiplication.
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. But the transposed matrix does take extra time to create.
     */
    friend Mat<M_y, M_y, T> operator*(const Mat<M_y, N_x, T>& lhs,
                                      const Mat<N_x, M_y, T>& rhs) {
        Mat<M_y, M_y, T> res{};
        if constexpr (is_column_major) {
            const auto lhs_transposed = lhs.transposed();
            size_t idx = 0;
            for (int i = 0; i < rhs.column_count(); i++) {
                for (int j = 0; j < lhs_transposed.column_count(); j++) {
                    res[idx++] = std::transform_reduce(
                            rhs.column_begin(i), rhs.column_end(i),
                            lhs_transposed.column_begin(j), T{0}, std::plus(),
                            std::multiplies());
                }
            }
        } else {
            const auto rhs_transposed = rhs.transposed();
            size_t idx = 0;
            for (int i = 0; i < lhs.row_count(); i++) {
                for (int j = 0; j < rhs_transposed.row_count(); j++) {
                    res[idx++] = std::transform_reduce(
                            lhs.row_get(i), rhs.row_end(i),
                            rhs_transposed.row_begin(j), T{0}, std::plus(),
                            std::multiplies());
                }
            }
        }
        return res;
    }

    /**
     * @brief Perform matrix-vector multiplication
     */
    friend Vec<M_y, T> operator*(const Mat<M_y, N_x, T>& lhs,
                                 const Vec<N_x, T>& rhs) {
        Vec<M_y, T> res{T{0}};
        if constexpr (is_column_major) {
            size_t idx = 0;
            for (int x = 0; x < N_x; x++) {
                for (int y = 0; y < M_y; y++) {
                    res[y] += lhs[idx++] * rhs[x];
                }
            }
        } else {
            for (size_t y = 0; y < M_y; y++) {
                res[y] = std::transform_reduce(lhs.row_begin(y), lhs.row_end(y),
                                               rhs.begin(), T{0}, std::plus(),
                                               std::multiplies());
            }
        }
        return res;
    }

public:
    /**
     * @brief Add with another matrix
     */
    Mat operator+=(const Mat& that) {
        for (int i = 0; i < Mat::size(); i++) {
            (*this)[i] += that[i];
        }
        return *this;
    }

    /**
     * @brief Matrix-matrix addition
     */
    friend Mat operator+(const Mat& lhs, const Mat& rhs) {
        Mat res{};
        for (int i = 0; i < Mat::size(); i++) {
            res[i] = lhs[i] + rhs[i];
        }
        return res;
    }

    /**
     * @brief Subtract with another matrix
     */
    Mat operator-=(const Mat& that) {
        for (int i = 0; i < Mat::size(); i++) {
            (*this)[i] -= that[i];
        }
        return *this;
    }

    /**
     * @brief Matrix-matrix subtraction
     */
    friend Mat operator-(const Mat& lhs, const Mat& rhs) {
        Mat res{};
        for (int i = 0; i < Mat::size(); i++) {
            res[i] = lhs[i] - rhs[i];
        }
        return res;
    }

    /**
     * @brief Multiply with a scalar
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    Mat operator*=(const U scalar) {
        for (int i = 0; i < Mat::size(); i++) {
            (*this)[i] *= scalar;
        }
        return *this;
    }

    /**
     * @brief Matrix-scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Mat operator*(const U scalar, const Mat& mat) {
        Mat res{};
        for (int i = 0; i < Mat::size(); i++) {
            res[i] = scalar * mat[i];
        }
        return res;
    }

    /**
     * @brief Matrix-scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Mat operator*(const Mat& mat, const U scalar) {
        Mat res{};
        for (int i = 0; i < Mat::size(); i++) {
            res[i] = mat[i] * scalar;
        }
        return res;
    }

    /**
     * @brief Multiply with a inverse scalar
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    Mat operator/=(const U scalar) {
        assert(scalar != T{0});
        for (int i = 0; i < Mat::size(); i++) {
            (*this)[i] /= scalar;
        }
        return *this;
    }

    /**
     * @brief Matrix-inverse scalar multiplication
     */
    template <typename U>
        requires(non_narrowing_conversion<U, T>)
    friend Mat operator/(const Mat& mat, const U scalar) {
        Mat res{};
        for (int i = 0; i < Mat::size(); i++) {
            res[i] = mat[i] / scalar;
        }
        return res;
    }

private:
    template <int M1_y, int N1_x, typename T1>
        requires(M1_y > 0 && N1_x > 0 && std::is_arithmetic_v<T1>)
    friend class Mat;

    /**
     * @brief Apply function to indicies in the optimal order
     */
    template <typename Callable>
        requires(std::is_invocable_v<Callable, int, int>)
    static constexpr void for_each_indicies(Callable f) {
        if constexpr (is_column_major) {
            for (int x = 0; x < N_x; x++) {
                for (int y = 0; y < M_y; y++) {
                    f(y, x);
                }
            }
        } else {
            for (int y = 0; y < M_y; y++) {
                for (int x = 0; x < N_x; x++) {
                    f(y, x);
                }
            }
        }
    }

    // Major-specific iterators
    // clang-format off
    constexpr auto column_begin(int x)       requires(is_column_major == true) { return &this->m_elements[M_y * x]; }
    constexpr auto column_begin(int x) const requires(is_column_major == true) { return &this->m_elements[M_y * x]; }
    constexpr auto column_end(int x)         requires(is_column_major == true) { return &this->m_elements[M_y * (x + 1)]; }
    constexpr auto column_end(int x) const   requires(is_column_major == true) { return &this->m_elements[M_y * (x + 1)]; }

    constexpr auto row_begin(int y)       requires(is_column_major == false) { return &this->m_elements[N_x * y]; }
    constexpr auto row_begin(int y) const requires(is_column_major == false) { return &this->m_elements[N_x * y]; }
    constexpr auto row_end(int y)         requires(is_column_major == false) { return &this->m_elements[N_x * (y + 1)]; }
    constexpr auto row_end(int y) const   requires(is_column_major == false) { return &this->m_elements[N_x * (y + 1)]; }
    // clang-format on

    static constexpr void set_from(int& idx, Mat& out) {};

    template <int M, typename U, typename... Args>
    static constexpr void set_from(int& idx,
                                   Mat& out,
                                   const Vec<M, U>& vec,
                                   const Args&... args) {
        if (is_column_major) {
            out.column_set(idx++, Vec<M_y, T>{vec});
        } else {
            out.row_set(idx++, Vec<N_x, T>{vec});
        }
        set_from(idx, out, args...);
    };

    template <int M1_y, int N1_x, typename U, typename... Args>
    static constexpr void set_from(int& idx,
                                   Mat& out,
                                   const Mat<M1_y, N1_x, U>& mat,
                                   const Args&... args) {
        if constexpr (is_column_major) {
            for (int x = 0; x < N1_x; x++) {
                out.column_set(idx++, Vec<M_y, T>{mat.column_get(x)});
            }
        } else {
            for (int y = 0; y < M1_y; y++) {
                out.row_set(idx++, Vec<N_x, T>{mat.row_get(y)});
            }
        }
        set_from(idx, out, args...);
    };
};

template <int M_y, int N_x, typename T>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
inline std::ostream& operator<<(std::ostream& out, const Mat<M_y, N_x, T>& mat)
    requires(Mat<M_y, N_x, T>::is_column_major == true)
{
    std::stringstream s{};
    auto count_chars = [&](const T& value) {
        s.str("");
        s << value;
        return s.str().size();
    };
    std::array<int, mat.size()> lengths;
    std::transform(mat.begin(), mat.end(), lengths.begin(), count_chars);
    auto max_length = *std::max_element(lengths.begin(), lengths.end());

    out << "[";
    for (int y = 0; y < M_y; y++) {
        if (y != 0) {
            out << " ";
        }
        out << "[ ";
        for (int x = 0; x < N_x; x++) {
            auto index = Mat<M_y, N_x, T>::map_index(y, x);
            auto lpad = (max_length - lengths[index]) / 2;
            auto rpad = max_length - (lpad + lengths[index]);

            out << std::setw(lpad) << "";
            out << mat.m_elements[index];
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

template <int M_y, int N_x, typename T>
    requires(M_y > 0 && N_x > 0 && std::is_arithmetic_v<T>)
inline std::ostream& operator<<(std::ostream& out, const Mat<M_y, N_x, T>& mat)
    requires(Mat<M_y, N_x, T>::is_column_major == false)
{
    std::stringstream s{};
    auto count_chars = [&](const T& value) {
        s.str("");
        s << value;
        return s.str().size();
    };
    std::array<int, mat.size()> lengths;
    std::transform(mat.begin(), mat.end(), lengths.begin(), count_chars);
    auto max_length = *std::max_element(lengths.begin(), lengths.end());

    out << "[";
    for (int y = 0; y < M_y; y++) {
        if (y != 0) {
            out << " ";
        }
        for (int x = 0; x < N_x; x++) {
            if (y == 0) {
                out << "[ ";
            } else {
                out << "  ";
            }
            auto index = Mat<M_y, N_x, T>::map_index(y, x);
            auto lpad = (max_length - lengths[index]) / 2;
            auto rpad = max_length - (lpad + lengths[index]);

            out << std::setw(lpad) << "";
            out << mat.m_elements[index];
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

}  // namespace asciirast::math
