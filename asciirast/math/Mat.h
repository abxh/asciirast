/**
 * @file Mat.h
 * @brief Column-major matrix class
 */

#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <iomanip>
#include <numeric>
#include <ranges>
#include <sstream>
#include <type_traits>

#include "Vec.h"
#include "types.h"

namespace asciirast::math {

template <size_t M_y, size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
class Mat;

/**
 * @brief Matrix information trait
 */
template <typename TT, bool is_col_major>
struct mat_info {
    using value_type = void;
    static constexpr bool is_mat = false;
    static constexpr size_t rows = !is_col_major ? 1 : 0;
    static constexpr size_t columns = is_col_major ? 1 : 0;
};

/**
 * @brief Matrix information trait
 */
template <size_t M_y, size_t N_x, typename T, bool is_col_major>
struct mat_info<Mat<M_y, N_x, T>, is_col_major> {
    using value_type = T;
    static constexpr bool is_mat = true;
    static constexpr size_t rows = M_y;
    static constexpr size_t columns = N_x;
};

/**
 * @brief Is convertible matrix trait.
 * E.g. Mat<2, 2, float> is convertible to Mat<2, 2, double>.
 */
template <typename TT, typename U, bool is_col_major>
constexpr bool is_conv_mat_v =
        mat_info<TT, is_col_major>::is_mat &&
        non_narrowing_conv<typename mat_info<TT, is_col_major>::value_type, U>;

/**
 * @brief MxN dimensional matrix.
 */
template <size_t M_y, size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
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

    template <typename... Vs>
        requires(is_conv_vec_v<Vs, T> && ...)
    static inline Mat from_columns(const Vs&... vecs) {
        if (is_column_major == true) {
            return Mat{vecs...};
        } else {
            return Mat{vecs...}.transposed();
        }
    }

    template <typename... Vs>
        requires(is_conv_vec_v<Vs, T> && ...)
    static inline Mat from_rows(const Vs&... vecs) {
        if (is_column_major == false) {
            return Mat{vecs...};
        } else {
            return Mat{vecs...}.transposed();
        }
    }

    /**
     * @brief Generate the identity matrix. Applies only for squared matricies.
     */
    static inline Mat identity()
        requires(N_x == M_y)
    {
        return Mat{T{1}};
    }

    /**
     * @brief Map a 2d matrix index to a 1d array index.
     */
    static inline size_t map_index(const size_t y, const size_t x) {
        if constexpr (is_column_major) {
            return M_y * x + y;
        } else {
            return N_x * y + x;
        }
    }

    /**
     * @brief Apply function with signature void(*)(size_t, size_t) to indicies
     * in the optimal order.
     */
    template <typename Callable>
        requires(std::is_invocable_v<Callable, std::size_t, std::size_t>)
    static inline void apply_on_indicies(Callable f) {
        if constexpr (is_column_major) {
            for (size_t x = 0; x < N_x; x++) {
                for (size_t y = 0; y < M_y; y++) {
                    f(y, x);
                }
            }
        } else {
            for (size_t y = 0; y < M_y; y++) {
                for (size_t x = 0; x < N_x; x++) {
                    f(y, x);
                }
            }
        }
    }

private:
    void set_elements_from_args(size_t& idx, Mat& out) {};

    template <size_t M, typename U, typename... Args>
    void set_elements_from_args(size_t& idx,
                                Mat& out,
                                const Vec<M, U>& vector,
                                Args... args) {
        if constexpr (is_column_major) {
            out.column_set(idx++, Vec<M_y, T>{vector});
        } else {
            out.row_set(idx++, Vec<N_x, T>{vector});
        }
        set_elements_from_args(idx, out, args...);
    };

    template <size_t M1_y, size_t N1_x, typename U, typename... Args>
    void set_elements_from_args(size_t& idx,
                                Mat& out,
                                const Mat<M1_y, N1_x, U>& mat,
                                Args... args) {
        if constexpr (is_column_major) {
            for (const auto& vector : mat.column_vectors()) {
                out.column_set(idx++, Vec<M_y, T>{vector});
            }
        } else {
            for (const auto& vector : mat.row_vectors()) {
                out.row_set(idx++, Vec<N_x, T>{vector});
            }
        }
        set_elements_from_args(idx, out, args...);
    };

public:
    std::array<T, Mat::size()> m_elements;  ///< 1d array of elements

    /**
     * @brief Default constructor. Set values to zero.
     */
    Mat() {
        for (size_t i = 0; i < size(); i++) {
            m_elements[i] = T{0};
        }
    }

    /**
     * @brief Initiate diagonal elements to some value
     */
    template <typename U>
        requires(non_narrowing_conv<T, U>)
    explicit Mat(const U& diagonal_element) {
        auto f = [this, &diagonal_element](size_t y, size_t x) {
            m_elements[map_index(y, x)] = (x == y) ? T{diagonal_element} : T{0};
        };
        Mat::apply_on_indicies(f);
    }

    /**
     * @brief Initiate matrix from a mix of vectors and matrices. *Depending on
     * storage order*, sets either the column or row vectors. Fills out the rest
     * of the elements to zero.
     */
    template <typename... Args>
        requires((is_conv_vec_v<Args, T> ||
                  is_conv_mat_v<Args, T, is_column_major>) &&
                 ...)
    explicit Mat(const Args&... args)
        requires((mat_info<Args, is_column_major>::rows + ...) <= M_y &&
                 (mat_info<Args, is_column_major>::columns + ...) <= N_x &&
                 (!is_column_major || ((vec_info<Args>::size <= M_y) && ...)) &&
                 (is_column_major || ((vec_info<Args>::size <= N_x) && ...)))
    {
        size_t idx = 0;
        set_elements_from_args(idx, *this, args...);
        if constexpr (is_column_major) {
            for (size_t x = idx; x < N_x; x++) {
                this->column_set(x, Vec<M_y, T>{T{0}});
            }
        } else {
            for (size_t y = idx; y < M_y; y++) {
                this->row_set(y, Vec<N_x, T>{T{0}});
            }
        }
    }

    /**
     * @brief Set matrix using iterator. Iterator must point to M_y * N_x
     * elements.
     */
    template <std::input_iterator Iterator>
        requires(std::same_as<std::iter_value_t<Iterator>, T>)
    explicit Mat(Iterator begin) {
        std::copy_n(begin, this->size(), this->begin());
    };

public:
    /**
     * @brief Print the matrix (column major)
     */
    template <size_t M1_y, size_t N1_x, typename T1>
        requires((M1_y + N1_x) > 1 && std::is_arithmetic_v<T1>)
    friend std::ostream& operator<<(std::ostream& out,
                                    const Mat<M1_y, N1_x, T1>& mat)
        requires(Mat<M1_y, N1_x, T1>::is_column_major == true);

    /**
     * @brief Print the matrix (row major)
     */
    template <size_t M1_y, size_t N1_x, typename T1>
        requires((M1_y + N1_x) > 1 && std::is_arithmetic_v<T1>)
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
     * @brief Index the matrix with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    T& operator()(size_t y, size_t x) {
        if (y >= M_y || x >= N_x) {
            throw std::out_of_range("asciirast::math::Mat::operator()");
        }
        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Index the matrix with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    const T& operator()(size_t y, size_t x) const {
        if (y >= M_y || x >= N_x) {
            throw std::out_of_range("asciirast::math::Mat::operator()");
        }
        return m_elements[map_index(y, x)];
    }

    /**
     * @brief Get x'th column with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    Vec<M_y, T> column_get(size_t x) const {
        if (x >= N_x) {
            throw std::out_of_range("asciirast::math::Mat::column_get()");
        }
        if constexpr (is_column_major) {
            return Vec<M_y, T>(&m_elements[M_y * x]);
        } else {
            auto view = std::ranges::iota_view(0U, M_y) |
                        std::views::transform([this, &x](auto y) {
                            return m_elements[map_index(y, x)];
                        });
            return Vec<M_y, T>(view.begin());
        }
    }

    /**
     * @brief Get y'th row with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    Vec<N_x, T> row_get(size_t y) const {
        if (y >= M_y) {
            throw std::out_of_range("asciirast::math::Mat::row_get()");
        }
        if constexpr (!is_column_major) {
            return Vec<N_x, T>(&m_elements[N_x * y]);
        } else {
            auto view = std::ranges::iota_view(0U, M_y) |
                        std::views::transform([this, &y](auto x) {
                            return m_elements[map_index(y, x)];
                        });
            return Vec<N_x, T>(view.begin());
        }
    }

    /**
     * @brief Set x'th column with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    Mat& column_set(size_t x, const Vec<M_y, T>& v) {
        if (x >= N_x) {
            throw std::out_of_range("asciirast::math::Mat::column_set()");
        }
        if constexpr (is_column_major) {
            std::copy(v.begin(), v.end(), &m_elements[M_y * x]);
        } else {
            for (size_t y = 0; y < M_y; y++) {
                m_elements[map_index(y, x)] = v.m_components[y];
            }
        }
        return *this;
    }

    /**
     * @brief Set y'th row with bounds checking
     * @throws out_of_range if indicies are out of bounds.
     */
    Mat& row_set(size_t y, const Vec<N_x, T>& v) {
        if (y >= M_y) {
            throw std::out_of_range("asciirast::math::Mat::row_set()");
        }
        if constexpr (!is_column_major) {
            std::copy(v.begin(), v.end(), &m_elements[N_x * y]);
        } else {
            for (size_t x = 0; x < N_x; x++) {
                m_elements[map_index(y, x)] = v.m_components[x];
            }
        }
        return *this;
    }

    /**
     * @brief Get range over row vectors
     */
    auto row_vectors() const {
        return std::ranges::iota_view(0U, M_y) |
               std::views::transform([this](auto x) { return row_get(x); });
    }

    /**
     * @brief Get range over column vectors
     */
    auto column_vectors() const {
        return std::ranges::iota_view(0U, N_x) |
               std::views::transform([this](auto y) { return column_get(y); });
    }

    /**
     * @brief Get transposed matrix
     */
    Mat<N_x, M_y, T> transposed() const {
        using TranMat = Mat<N_x, M_y, T>;
        using OrigMat = Mat;

        auto res = TranMat{};
        auto f = [this, &res](auto y, auto x) {
            auto tran_i = TranMat::map_index(y, x);
            auto orig_i = OrigMat::map_index(x, y);

            res.m_elements[tran_i] = this->m_elements[orig_i];
        };
        TranMat::apply_on_indicies(f);

        return res;
    }

public:
    /**
     * @brief Add with another matrix
     */
    const Mat& operator+=(const Mat& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), std::plus());
        return *this;
    }

    /**
     * @brief Perform matrix addition
     */
    friend Mat operator+(const Mat& lhs, const Mat& rhs) {
        Mat res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       std::plus());
        return res;
    }

    /**
     * @brief Subtract with another matrix
     */
    const Mat& operator-=(const Mat& rvalue) {
        std::transform(this->begin(), this->end(), rvalue.begin(),
                       this->begin(), std::minus());
        return *this;
    }

    /**
     * @brief Perform matrix subtraction
     */
    friend Mat operator-(const Mat& lhs, const Mat& rhs) {
        Mat res{};
        std::transform(lhs.begin(), lhs.end(), rhs.begin(), res.begin(),
                       std::minus());
        return res;
    }

    /**
     * @brief Perform matrix multiplication.
     *
     * This takes advantage of the abstractions present and the linear
     * array used to store the matrix.
     *
     * Does a small optimisation of using the transposed matrix for
     * optimal access. The transposed matrix does take extra time to create.
     */
    friend Mat<M_y, M_y, T> operator*(const Mat<M_y, N_x, T>& lhs,
                                      const Mat<N_x, M_y, T>& rhs) {
        Mat<M_y, M_y, T> res{};
        size_t idx = 0;
        if constexpr (is_column_major) {
            auto lhs_transposed = lhs.transposed();

            for (const auto& v : rhs.column_vectors()) {
                for (const auto& w : lhs_transposed.column_vectors()) {
                    res.m_elements[idx++] = w.dot(v);
                }
            }
        } else {
            auto rhs_transposed = rhs.transposed();

            for (const auto& w : lhs.row_vectors()) {
                for (const auto& v : rhs_transposed.row_vectors()) {
                    res.m_elements[idx++] = v.dot(w);
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
        if constexpr (is_column_major) {
            auto cols = lhs.column_vectors();
            auto view = std::views::zip_transform(std::multiplies(), cols, rhs);

            return std::accumulate(view.begin(), view.end(), Vec<M_y, T>{});

        } else {
            auto rows = lhs.row_vectors();
            Vec<M_y, T> res{0};
            for (size_t y = 0; y < M_y; y++) {
                res.m_components[y] = rows[y].dot(rhs);
            }
            return res;
        }
    }
};

template <size_t M_y, size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
std::ostream& operator<<(std::ostream& out, const Mat<M_y, N_x, T>& mat)
    requires(Mat<M_y, N_x, T>::is_column_major == false)
{
    std::stringstream s{};
    auto count_chars = [&](const T& value) {
        s.str("");
        s << value;
        return s.str().size();
    };
    std::array<std::size_t, mat.size()> lengths;
    std::transform(mat.begin(), mat.end(), lengths.begin(), count_chars);
    auto max_length = *std::max_element(lengths.begin(), lengths.end());

    out << "[";
    for (std::size_t y = 0; y < M_y; y++) {
        if (y != 0) {
            out << " ";
        }
        out << "[ ";
        for (std::size_t x = 0; x < N_x; x++) {
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

template <size_t M_y, size_t N_x, typename T>
    requires((M_y + N_x) > 1 && std::is_arithmetic_v<T>)
std::ostream& operator<<(std::ostream& out, const Mat<M_y, N_x, T>& mat)
    requires(Mat<M_y, N_x, T>::is_column_major == true)
{
    std::stringstream s{};
    auto count_chars = [&](const T& value) {
        s.str("");
        s << value;
        return s.str().size();
    };
    std::array<std::size_t, mat.size()> lengths;
    std::transform(mat.begin(), mat.end(), lengths.begin(), count_chars);
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
