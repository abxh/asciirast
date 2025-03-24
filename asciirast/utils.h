
#pragma once

#include <utility>

namespace asciirast::utils {

template<class T>
class AbstractChangeDetected
{
public:
    virtual bool changed() const = 0;
    virtual const T& get() const = 0;
};

// Inspiration:
// https://meetingcpp.com/blog/items/Detecting-change-to-an-object-in-Cpp.html

template<class T, class BoolLike = bool>
    requires(std::is_convertible_v<BoolLike, bool>)
class ChangeDetected : public AbstractChangeDetected<T>
{
    T m_val;
    BoolLike m_changed;

public:
    ChangeDetected()
            : m_val{}
            , m_changed{ false } {};

    ChangeDetected(const T& val)
            : m_val{ val }
            , m_changed{ true } {};

    ChangeDetected(T&& val)
            : m_val{ std::move(val) }
            , m_changed{ true } {};

    operator const T&() const { return m_val; }
    const T* operator->() const { return &m_val; }

    const T& get() const override { return m_val; }
    bool changed() const override { return m_changed; }
};

}
