
#pragma once

#include <SDL.h>
#include <SDL_pixels.h>

#include <cassert>
#include <cstdint>
#include <cstdlib>

class SDLClock
{
public:
    SDLClock(int ms_per_update = 100)
    {
        m_previous_time_ms = SDL_GetTicks64();
        m_lag_ms = 0;
        m_ms_per_update = ms_per_update;
    }

    template<typename F>
        requires(std::invocable<F, float>)
    void update(F callback)
    {
        while (m_lag_ms >= 0) {
            const auto dt_sec = m_ms_per_update / 1000.f;
            callback(dt_sec);
            m_lag_ms -= m_ms_per_update;
        }
    }

    void tick()
    {
        const uint64_t current_time_ms = SDL_GetTicks64();
        const int elapsed_ms = static_cast<int>(current_time_ms - m_previous_time_ms);
        m_previous_time_ms = current_time_ms;
        m_lag_ms += elapsed_ms;
    }

private:
    std::uint64_t m_previous_time_ms;
    int m_lag_ms;
    int m_ms_per_update;
};
