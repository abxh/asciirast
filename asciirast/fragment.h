/**
 * @file fragment.h
 * @brief Fragment type and related types
 *
 * On 2x2 block processing:
 * https://www.gamedev.net/forums/topic/614000-ddxddy-functions-software-rasterization-and-texture-filtering/
 *
 * opengl "helper" (fragment shader) invocation check:
 * https://registry.khronos.org/OpenGL-Refpages/gl4/html/gl_HelperInvocation.xhtml
 */

#pragma once

#include <type_traits>
#include <variant>

#include "./detail/assert.h"
#include "./detail/has_minus_operator.h"
#include "./math/types.h"
#include "./math/utils.h"
#include "./program_token.h"
#include "./renderer_options.h"
#include "./varying.h"

namespace asciirast {

/**
 * @brief Fragment type
 */
template<VaryingInterface Varying>
struct Fragment
{
    math::Vec4 pos = {}; ///< position in homogenous space
    Varying attrs = {};  ///< vertex attributes
};

/**
 * @brief Projected fragment type
 */
template<VaryingInterface Varying>
struct ProjectedFragment
{
    math::Vec2 pos = {};    ///< window position
    math::Float depth = {}; ///< aka z/w
    math::Float Z_inv = {}; ///< aka 1/w
    Varying attrs = {};     ///< fragment attributes
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
    ASCIIRAST_ASSERT(frag.pos.w != 0, "non-zero w coordinate. the fragment should be culled by now", frag.pos);

    const auto Z_inv = 1 / frag.pos.w;
    const auto v = frag.pos.xyz * Z_inv;

    return ProjectedFragment<Varying>{
        .pos = v.xy,
        .depth = v.z,
        .Z_inv = Z_inv,
        .attrs = frag.attrs,
    };
}

/**
 * @brief Linear interpolation of fragments
 */
template<VaryingInterface T>
[[maybe_unused]]
static auto
lerp(const Fragment<T>& a, const Fragment<T>& b, const math::Float t) -> Fragment<T>
{
    return Fragment<T>{ .pos = lerp(a.pos, b.pos, t), .attrs = lerp_varying(a.attrs, b.attrs, t) };
}

/**
 * @brief Linear interpolation of projected fragments
 */
template<VaryingInterface T>
[[maybe_unused]]
static auto
lerp(const ProjectedFragment<T>& a, const ProjectedFragment<T>& b, const math::Float t) -> ProjectedFragment<T>
{
    if (t == 0) {
        return a;
    } else if (t == 1) {
        return b;
    }
    const auto Z_inv_t = std::lerp(a.Z_inv, b.Z_inv, t);

    return ProjectedFragment<T>{
        .pos = lerp(a.pos, b.pos, t),
        .depth = lerp_varying_perspective_corrected(a.depth, b.depth, t, a.Z_inv, b.Z_inv, Z_inv_t),
        .Z_inv = Z_inv_t,
        .attrs = lerp_varying_perspective_corrected(a.attrs, b.attrs, t, a.Z_inv, b.Z_inv, Z_inv_t)
    };
}

/**
 * @brief Fragment context to access fragment specific things
 *
 * @tparam ValueTypes The value types it can be initialized with
 */
template<typename... ValueTypes>
    requires((detail::has_minus_operator<ValueTypes> && ...) && (std::semiregular<ValueTypes> && ...))
class FragmentContextType
{
public:
    using ValueVariant = std::variant<std::monostate, ValueTypes...>; ///< value type variant

    /**
     * @brief Fragment context (runtime) type
     */
    enum class Type
    {
        POINT,
        LINE,
        FILLED
    };

    /**
     * @brief Construct a new fragment context
     */
    FragmentContextType(const std::size_t id,
                        std::array<ValueVariant, 4>& quad,
                        const Type type,
                        const bool is_helper_invocation = false,
                        const bool is_initialized = false)
            : m_id{ id }
            , m_quad{ quad }
            , m_type{ type }
            , m_is_helper_invocation{ is_helper_invocation }
            , m_is_initialized{ is_initialized } {};

    /**
     * @brief Initialize context with value for this particular fragment
     *
     * @param value The value at hand
     * @return The token to co_yield back to the renderer
     */
    template<typename T>
    [[nodiscard]] ProgramToken init(const T& value)
        requires(std::is_same_v<T, ValueTypes> || ...)
    {
        m_quad[m_id] = value;
        m_is_initialized = true;

        return ProgramToken::Syncronize;
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
        ASCIIRAST_ASSERT(m_is_initialized, "FragmentContext is initalized");
        ASCIIRAST_ASSERT(id < 1 && m_type == Type::POINT || m_type != Type::POINT, "possible point id: 0", id, m_type);
        ASCIIRAST_ASSERT(id < 2 && m_type == Type::LINE || m_type != Type::LINE, "possible line id: 0,1", id, m_type);
        ASCIIRAST_ASSERT(
                id < 4 && m_type == Type::FILLED || m_type != Type::FILLED, "possible filled id: 0,1,2,3", id, m_type);

        return std::get<T>(m_quad[id]);
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
        ASCIIRAST_ASSERT(m_is_initialized, "FragmentContext is initalized");
        ASCIIRAST_ASSERT(m_type == Type::FILLED, "FragmentContext is initialized for filled type");

        /*
            0 --> 1
            |     |
            v     v
            2 --> 3
        */

        if (m_id == 0 || m_id == 1) {
            return std::get<T>(m_quad[1]) - std::get<T>(m_quad[0]);
        } else {
            return std::get<T>(m_quad[3]) - std::get<T>(m_quad[2]);
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
        ASCIIRAST_ASSERT(m_is_initialized, "FragmentContext is initalized");
        ASCIIRAST_ASSERT(m_type == Type::FILLED, "FragmentContext is initialized for filled type");

        /*
             0 --> 1
             |     |
             v     v
             2 --> 3
         */

        if (m_id == 0 || m_id == 2) {
            return std::get<T>(m_quad[2]) - std::get<T>(m_quad[0]);
        } else {
            return std::get<T>(m_quad[3]) - std::get<T>(m_quad[1]);
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
        ASCIIRAST_ASSERT(m_is_initialized, "FragmentContext is initalized");
        ASCIIRAST_ASSERT(m_type == Type::LINE, "FragmentContext is initialized for line type");

        return std::get<T>(m_quad[1]) - std::get<T>(m_quad[0]);
    }

private:
    std::size_t m_id;
    std::array<ValueVariant, 4>& m_quad;
    Type m_type;
    bool m_is_helper_invocation;
    bool m_is_initialized;
};

};
