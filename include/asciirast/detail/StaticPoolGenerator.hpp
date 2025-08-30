/**
 * @file StaticPoolGenerator.h
 * @brief Static Pool Generator
 *
 * See:
 * @link https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2018/p1365r0.pdf
 */

#pragma once

#include <array>
#include <coroutine>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <new>
#include <optional>

#include "./assert.hpp"

namespace asciirast {

/// @cond DO_NOT_DOCUMENT
namespace detail {

template<typename T, std::size_t MaxFrameSize, std::size_t FrameCount>
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

    void* allocate() { return &m_pool[m_free_indices.pop()]; }

    void deallocate(void* ptr)
    {
        const auto p = reinterpret_cast<std::byte*>(ptr);

        ASCIIRAST_ASSERT(begin() <= p && p < end(), "pointer points to part of the pool", begin(), p, end());

        const auto offset = static_cast<std::uintptr_t>(p - begin());

        ASCIIRAST_ASSERT(offset % sizeof(Frame) == 0, "pointer is aligned correctly");

        const auto index = offset / sizeof(Frame);

        m_free_indices.push(index);
    }

private:
    std::byte* begin() { return reinterpret_cast<std::byte*>(&m_pool[0]); }

    const std::byte* begin() const { return reinterpret_cast<const std::byte*>(&m_pool[0]); }

    std::byte* end() { return reinterpret_cast<std::byte*>(&m_pool[FrameCount]); }

    const std::byte* end() const { return reinterpret_cast<const std::byte*>(&m_pool[FrameCount]); }

    struct Frame
    {
        alignas(std::max_align_t) std::byte storage[MaxFrameSize];
    };
    struct FreeIndicies
    {
        constexpr void push(const std::uintptr_t& value)
        {
            ASCIIRAST_ASSERT(m_size < FrameCount, "enough frames allocated");
            m_data[m_size++] = value;
        }
        constexpr std::uintptr_t pop()
        {
            ASCIIRAST_ASSERT(m_size > 0, "no double free or corruption");
            return m_data[--m_size];
        }
        std::array<std::uintptr_t, FrameCount> m_data = {};
        std::size_t m_size = 0;
    };

    FreeIndicies m_free_indices;
    std::array<Frame, FrameCount> m_pool;
};

template<typename T, std::size_t MaxFrameSize, std::size_t FrameCount>
class StaticPoolGenerator
{
public:
    struct Promise;

    using promise_type = Promise;
    using handle_type = std::coroutine_handle<promise_type>;

    static inline FramePool<promise_type, MaxFrameSize, FrameCount> frame_pool;

    struct bad_alloc_generator : public std::bad_alloc
    {
        const char* what() const noexcept override
        {
            return "coroutine frame size exceeds the statically allocated limit";
        }
    };

    struct Promise
    {
        std::optional<T> current_value;

        static void* operator new(const std::size_t n)
        {
            if (n >= MaxFrameSize) [[unlikely]] {
                throw bad_alloc_generator();
            }
            return frame_pool.allocate();
        }
        static void operator delete(void* ptr, std::size_t) noexcept { frame_pool.deallocate(ptr); }

        StaticPoolGenerator get_return_object() { return StaticPoolGenerator{ handle_type::from_promise(*this) }; }
        void await_transform() = delete; // no co_await allowed

        std::suspend_always initial_suspend() noexcept { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }

        std::suspend_always yield_value(T value) noexcept
        {
            current_value = value;
            return {};
        }
        void return_void(){};
    };

    explicit StaticPoolGenerator(const handle_type coroutine)
        : m_coroutine{ coroutine } {};
    StaticPoolGenerator() = default;
    ~StaticPoolGenerator()
    {
        if (m_coroutine) m_coroutine.destroy();
    }
    StaticPoolGenerator(const StaticPoolGenerator&) = delete;
    StaticPoolGenerator& operator=(const StaticPoolGenerator&) = delete;
    StaticPoolGenerator(StaticPoolGenerator&& other) noexcept
        : m_coroutine{ other.m_coroutine }
    {
        other.m_coroutine = {};
    }
    StaticPoolGenerator& operator=(StaticPoolGenerator&& other) noexcept
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
        using difference_type = std::ptrdiff_t;
        using value_type = T;

        explicit Iterator(const handle_type coroutine)
            : m_coroutine{ coroutine } {};

        value_type operator*() const { return *m_coroutine.promise().current_value; }

        Iterator& operator++()
        {
            m_coroutine.resume();
            return *this;
        }

        Iterator operator++(int)
        {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        bool operator==(std::default_sentinel_t) const { return !m_coroutine || m_coroutine.done(); }

    private:
        handle_type m_coroutine;
    };

    static_assert(std::input_iterator<Iterator>);

    Iterator begin()
    {
        if (m_coroutine) m_coroutine.resume();
        return Iterator{ m_coroutine };
    }

    std::default_sentinel_t end() noexcept { return {}; }

private:
    handle_type m_coroutine;
};

}; // namespace detail
/// @endcond

}; // namespace asciirast
