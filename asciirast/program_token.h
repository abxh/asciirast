/**
 * @file program_token.h
 * @brief Special program tokens
 *
 * See:
 * @link https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1365r0.pdf
 */

#pragma once

#include <array>
#include <cassert>
#include <coroutine>
#include <cstddef>
#include <new>
#include <optional>
#include <ranges>

namespace asciirast {

/**
 * @brief Program tokens to be emitted to do special procedures
 */
enum class ProgramToken
{
    Discard,
    Keep,
    Syncronize,
};

/// @cond DO_NOT_DOCUMENT
namespace detail {
template<typename T, std::size_t PoolSize>
class StaticGenerator;
};
/// @endcond

/**
 * @brief Program Token Generator to be used
 */
using ProgramTokenGenerator = detail::StaticGenerator<ProgramToken, 4>;

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<typename T, std::size_t FrameCount, std::size_t FrameSize>
struct FramePool
{
public:
    constexpr FramePool()
            : m_free_indices{}
            , m_pool{}
    {
        for (std::size_t i = 0; i < FrameCount; ++i) {
            m_free_indices.push(i);
        }
    }

    void* allocate() { return std::launder(reinterpret_cast<T*>(&m_pool[m_free_indices.pop()])); }

    void deallocate(void* ptr)
    {
        const auto start = reinterpret_cast<std::byte*>(&m_pool[0]);
        [[maybe_unused]] const auto end = reinterpret_cast<std::byte*>(&m_pool[FrameCount]);
        const auto p = reinterpret_cast<std::byte*>(ptr);
        assert(start <= p && p < end && "pointer is in range");

        const std::size_t offset = static_cast<std::size_t>(p - start);
        assert(offset % sizeof(Frame) == 0 && "pointer is aligned");

        const std::size_t idx = offset / sizeof(Frame);
        m_free_indices.push(idx);
    }

private:
    struct Frame
    {
        alignas(std::max_align_t) std::byte storage[FrameSize];
    };
    struct FreeIndicies
    {
        constexpr void push(const std::size_t& value)
        {
            assert(m_size < FrameCount && "enough memory allocated");
            m_data[m_size++] = value;
        }
        constexpr std::size_t pop()
        {
            assert(m_size > 0 && "no double free or corruption");
            return m_data[--m_size];
        }
        std::array<std::size_t, FrameCount> m_data = {};
        std::size_t m_size = 0;
    };

    FreeIndicies m_free_indices;
    std::array<Frame, FrameCount> m_pool;
};

template<typename T, std::size_t PoolSize>
class StaticGenerator
{
public:
    struct Promise;

    using promise_type = Promise;
    using handle_type = std::coroutine_handle<promise_type>;

    static constexpr std::size_t MaxFrameSize = 512;

    inline static FramePool<promise_type, PoolSize, MaxFrameSize> frame_pool;

    struct Promise
    {
        static void* operator new([[maybe_unused]] std::size_t n)
        {
            assert(n < MaxFrameSize && "frame size is smaller than 1024 bytes");
            return frame_pool.allocate();
        }
        static void operator delete(void* ptr, std::size_t) noexcept { frame_pool.deallocate(ptr); }

        auto get_return_object() { return StaticGenerator{ handle_type::from_promise(*this) }; }
        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T value) noexcept
        {
            current_value = std::move(value);
            return {};
        }
        void await_transform() = delete; // no co_await allowed
        void return_void() {};
        void unhandled_exception() { throw; }

        std::optional<T> current_value;
    };

    explicit StaticGenerator(const handle_type coroutine)
            : m_coroutine{ coroutine } {};
    StaticGenerator() = default;
    ~StaticGenerator()
    {
        if (m_coroutine) m_coroutine.destroy();
    }
    StaticGenerator(const StaticGenerator&) = delete;
    StaticGenerator& operator=(const StaticGenerator&) = delete;
    StaticGenerator(StaticGenerator&& other) noexcept
            : m_coroutine{ other.m_coroutine }
    {
        other.m_coroutine = {};
    }
    StaticGenerator& operator=(StaticGenerator&& other) noexcept
    {
        if (this != &other) {
            if (m_coroutine) m_coroutine.destroy();
            m_coroutine = other.m_coroutine;
            other.m_coroutine = {};
        }
        return *this;
    }

    class Iterator
    {
    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using reference = const T&;
        using pointer = const T*;

        Iterator() noexcept
                : m_coroutine(nullptr) {};

        explicit Iterator(handle_type coroutine)
                : m_coroutine(coroutine)
        {
            if (m_coroutine && !m_coroutine.done()) m_coroutine.resume();
            if (m_coroutine && m_coroutine.done()) m_coroutine = nullptr;
        }

        reference operator*() const { return *m_coroutine.promise().current_value; }

        pointer operator->() const { return std::addressof(operator*()); }

        Iterator& operator++()
        {
            if (m_coroutine) {
                m_coroutine.resume();
                if (m_coroutine.done()) m_coroutine = nullptr;
            }
            return *this;
        }

        Iterator operator++(int)
        {
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        friend bool operator==(const Iterator& iter, std::default_sentinel_t) noexcept { return !iter.m_coroutine; }
        friend bool operator!=(const Iterator& iter, std::default_sentinel_t s) noexcept { return !(iter == s); }

    private:
        handle_type m_coroutine;
    };

    Iterator begin() { return Iterator{ m_coroutine }; }
    std::default_sentinel_t end() noexcept { return {}; }

private:
    handle_type m_coroutine;
};

static_assert(std::ranges::input_range<StaticGenerator<ProgramToken, 4>>);

};

}
