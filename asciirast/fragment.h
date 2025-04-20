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
    class FragmentContextPrepare
    {};

    std::variant<Targets, FragmentResultDiscard, FragmentContextPrepare> m_value;

    friend class Renderer;

    template<typename... ValueTypes>
        requires(detail::has_minus_operator<ValueTypes> && ...)
    friend class FragmentContextType;

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

    /**
     * @brief Implicit convertion from context prepare tag
     */
    FragmentResult(const FragmentContextPrepare&)
            : m_value{ FragmentContextPrepare() } {};
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
     * @brief Initialize context with value for this particular fragment
     *
     * @param value The value at hand
     * @return The result type to co_yield back to the renderer
     */
    template<typename T, typename Targets>
    [[nodiscard]] FragmentResult<Targets> init(const T& value, std::type_identity<Targets> = {})
        requires(std::is_same_v<T, ValueTypes> || ...)
    {
        assert(m_quad_ptr != nullptr);
        m_quad_ptr[m_id] = value;
        return FragmentResult<Targets>{ typename FragmentResult<Targets>::FragmentContextPrepare() };
    }

    /**
     * @brief Check if this fragment shader is called solely as a helper invocation
     *
     * @return A bool indicating whether this call is a helper invocation
     */
    [[nodiscard]] bool is_helper_invocation() const { return m_is_helper_invocation; }

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
     * @throws std::bad_access_variant If context wasn't initialized with value
     *
     * @return The value at the fragment with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T at(const std::size_t id, std::type_identity<T> = {}) const
    {
        assert(m_type != Type::UINITIALIZED);
        assert(m_quad_ptr != nullptr);

        return std::get<T>(m_quad_ptr[id]);
    }

    /**
     * @brief Get a derivative estimate of value with respect to x
     *
     * @throws std::bad_access_variant If context wasn't initialized with value
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdx(std::type_identity<T> = {}) const
    {
        assert(m_type == Type::FILLED);
        assert(m_quad_ptr != nullptr);

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
     * @throws std::bad_access_variant If context wasn't initialized with value
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdy(std::type_identity<T> = {}) const
    {
        assert(m_type == Type::FILLED);
        assert(m_quad_ptr != nullptr);

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

    /**
     * @brief Get derivative of straight line with respect to the direction it's drawn
     *
     * @throws std::bad_access_variant If context wasn't initialized with value
     *
     * @return The value difference with value type deduced by destination type
     */
    template<typename T>
    [[nodiscard]] T dFdv(std::type_identity<T> = {}) const
    {
        assert(m_type == Type::LINE);
        assert(m_quad_ptr != nullptr);

        return std::get<T>(m_quad_ptr[1]) - std::get<T>(m_quad_ptr[0]);
    }

private:
    std::size_t m_id;
    ValueVariant* m_quad_ptr;
    bool m_is_helper_invocation;
    Type m_type;

    FragmentContextType(const std::size_t id,
                        ValueVariant* quad_ptr,
                        const bool is_helper_invocation = false,
                        const Type type = Type::UINITIALIZED)
            : m_id{ id }
            , m_quad_ptr{ quad_ptr }
            , m_is_helper_invocation{ is_helper_invocation }
            , m_type{ type } {};

    friend class Renderer;
};

}; // namespace asciirast
