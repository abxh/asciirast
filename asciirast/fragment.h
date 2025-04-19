/**
 * @file fragment.h
 * @brief Fragment related classes
 */

#pragma once

#include <cassert>
#include <type_traits>
#include <variant>

#include "./math/types.h"

namespace asciirast {

/**
 * @brief Empty varying type
 */
struct EmptyVarying
{};

/**
 * @brief Concept to follow the varying interface
 *
 * Varying are the interpolated attributes of verticies.
 *
 * @tparam T The type to check
 */
template<typename T>
concept VaryingInterface = std::same_as<T, EmptyVarying> || requires(const T x, T y) {
    { x + x } -> std::same_as<T>;
    { x * math::Float{ -1 } } -> std::same_as<T>;
    { y = x } -> std::same_as<T&>;
    { T() } -> std::same_as<T>;
};

/**
 * @brief Linear interpolation of varying types
 *
 * @param a Left hand side varying
 * @param b Right hand side varying
 * @param t How much to interpolate between the two
 * @return The interpolated varying
 */
template<VaryingInterface Varying>
static Varying
lerp_varying(const Varying& a, const Varying& b, const math::Float t)
{
    if constexpr (std::is_same_v<Varying, EmptyVarying>) {
        return EmptyVarying();
    } else {
        return a * (1 - t) + b * t;
    }
}

/**
 * @brief Fragment type
 */
template<VaryingInterface Varying>
struct Fragment
{
    math::Vec4 pos; ///< world position in homogenous space
    Varying attrs;  ///< vertex attributes
};

/**
 * @brief Projected fragment type
 */
template<VaryingInterface Varying>
struct ProjectedFragment
{
    math::Vec2 pos;    ///< window position
    math::Float depth; ///< aka z
    math::Float Z_inv; ///< aka 1/w
    Varying attrs;     ///< fragment attributes

    ProjectedFragment operator-(const ProjectedFragment& that)
    {
        return { this->pos - that.pos, //
                 this->depth - that.depth,
                 this->Z_inv - that.Z_inv,
                 this->attrs + that.attrs * -1 };
    }
};

/**
 * @brief Project a Fragment to convert it to ProjectFragment
 *
 * @param frag The Fragment object
 * @return The ProjectedFragment object
 */
template<VaryingInterface Varying>
static auto
project_fragment(const Fragment<Varying>& frag) -> ProjectedFragment<Varying>
{
    assert(frag.pos.w != 0 && "non-zero w coordinate."
                              "the fragment should be culled otherwise");

    const auto Z_inv = 1 / frag.pos.w;
    const auto v = frag.pos.xyz * Z_inv;

    return ProjectedFragment<Varying>{
        .pos = v.xy,
        .depth = v.z,
        .Z_inv = Z_inv,
        .attrs = frag.attrs,
    };
}

/// @cond DO_NOT_DOCUMENT
namespace detail {
template<typename T>
concept has_minus_operator = requires(T t) {
    { T() } -> std::same_as<T>;
    { t - t } -> std::same_as<T>;
};
};
/// @endcond

/**
 * @brief Pass as fragment result to discard fragment
 */
class FragmentResultDiscard
{};

/**
 * @brief Fragment result type
 */
template<typename Targets>
class FragmentResult
{
    class ContextPrepare
    {};

    std::variant<Targets, FragmentResultDiscard, ContextPrepare> m_value;

    friend class Renderer;

    template<typename... ValueTypes>
        requires(detail::has_minus_operator<ValueTypes> && ...)
    friend class FragmentContextType;

    FragmentResult(const ContextPrepare&)
            : m_value{ ContextPrepare() } {};

public:
    /**
     * @brief Implicit convertion from Targets type
     *
     * @param targets The given Targets
     */
    FragmentResult(const Targets& targets)
            : m_value{ targets } {};

    /**
     * @brief Implicit convertion from discard tag
     */
    FragmentResult(const FragmentResultDiscard&)
            : m_value{ FragmentResultDiscard() } {};
};

// On 2x2 block processing:
// https://www.gamedev.net/forums/topic/614000-ddxddy-functions-software-rasterization-and-texture-filtering/

// opengl "helper" (fragment shader) invocation check:
// https://registry.khronos.org/OpenGL-Refpages/gl4/html/gl_HelperInvocation.xhtml

/**
 * @brief Fragment context type
 *
 * @tparam ValueTypes The value types it can be initialized with
 */
template<typename... ValueTypes>
    requires(detail::has_minus_operator<ValueTypes> && ...)
class FragmentContextType
{
public:
    using ValueVariant = std::variant<std::monostate, ValueTypes...>; ///< value type variant

    /**
     * @brief Fragment context (runtime) type
     */
    enum class Type
    {
        UINITIALIZED,
        POINT,
        LINE,
        FILLED
    };

    /**
     * @brief Check if this fragment shader is called solely as a helper invocation
     *
     * @return A bool indicating whether this call is a helper invocation
     */
    [[nodiscard]] bool is_helper_invocation() const { return m_is_helper_invocation; }

    /**
     * @brief Initialize context with value for this particular fragment
     *
     * @param value The value at hand
     * @param targets Value to deduce return type
     * @return The result type to co_yield back to the renderer
     */
    template<typename T, typename Targets>
    [[nodiscard]] FragmentResult<Targets> init(const T& value)
        requires(std::is_same_v<T, ValueTypes> || ...)
    {
        m_quad_ptr[m_id] = value;

        using ContextPrepare = typename FragmentResult<Targets>::ContextPrepare;

        return FragmentResult<Targets>{ ContextPrepare() };
    }

    /**
     * @brief Check fragment context (runtime) type
     *
     * @return A FragmentContextType enum value
     */
    [[nodiscard]] Type type() const { return m_type; }

    /**
     * @brief Get fragment id
     *
     * @return The fragment id as std::size_t
     */
    [[nodiscard]] std::size_t id() const { return m_id; }

    /**
     * @brief Get the value at the (neighbouring) fragment with id
     *
     * @return The value at the fragment with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T at(const std::size_t id) const
    {
        assert(m_type != ContextType::UINITIALIZED);
        assert((id < 1 && m_type == ContextType::POINT) || (m_type != ContextType::POINT));
        assert((id < 2 && m_type == ContextType::LINE) || (m_type != ContextType::LINE));
        assert((id < 4 && m_type == ContextType::FILLED) || (m_type != ContextType::FILLED));

        return std::get<T>(m_quad_ptr[id]);
    }

    /**
     * @brief Get derivative of line
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdv() const
    {
        assert(m_type == ContextType::LINE);

        return std::get<T>(m_quad_ptr[1]) - std::get<T>(m_quad_ptr[0]);
    }

    /**
     * @brief Get a derivative estimate of value with respect to x
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdx() const
    {
        assert(m_type == ContextType::FILLED);

        /*
            0 --> 1
            |     |
            v     v
            2 --> 3
        */

        if (m_id == 0 || m_id == 1) {
            return std::get<T>(m_quad_ptr[1]) - std::get<T>(m_quad_ptr[0]);
        } else {
            return std::get<T>(m_quad_ptr[3]) - std::get<T>(m_quad_ptr[2]);
        }
    }

    /**
     * @brief Get a derivative estimate of value with respect to y
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdy() const
    {
        assert(m_type == ContextType::FILLED);

        /*
            0 --> 1
            |     |
            v     v
            2 --> 3
        */

        if (m_id == 0 || m_id == 2) {
            return std::get<T>(m_quad_ptr[2]) - std::get<T>(m_quad_ptr[0]);
        } else {
            return std::get<T>(m_quad_ptr[3]) - std::get<T>(m_quad_ptr[1]);
        }
    }

private:
    std::size_t m_id = 4;

    ValueVariant* m_quad_ptr = nullptr;

    Type m_type = Type::UINITIALIZED;

    bool m_is_helper_invocation = false;

    friend class Renderer;
};

}; // namespace asciirast
