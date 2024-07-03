#include "engine.h"
#include "ascii_palettes.h"
#include "draw.h"
#include "engine_sdl_window.h"
#include "engine_timer.h"
#include "screen.h"

#include <SDL2/SDL.h>

engine_settings_type g_engine_settings;

static inline void sdl_poll_events(bool* on_running_p) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_KEYUP:
            break;
        case SDL_KEYDOWN:
            switch (event.key.keysym.sym) {
            case SDLK_q:
                *on_running_p = false;
                break;
            }
            break;
        case SDL_QUIT:
            *on_running_p = false;
            break;
        default:
            break;
        }
    }
}

void run(void) {
    g_engine_settings.cmdht_p = cmdht_create_with_initial_capacity(16);
    if (!g_engine_settings.cmdht_p) {
        abort();
    }
    cmdht_insert(g_engine_settings.cmdht_p, (key_comb_type){"q"}, (command_name_type){"quit"});

    struct screen_type* screen_context_p = screen_create(stdout);
    struct engine_sdl_window_type* sdl_window_context_p = engine_sdl_window_create();
    struct engine_timer_type* timer_context_p = engine_timer_create();

    renderer_init(screen_context_p, (mat4x4_type){}, sizeof(ascii_short_palette) - 1, ascii_short_palette);

    float x = 0;

    bool on_running = true;
    while (on_running) {
        sdl_poll_events(&on_running);
        engine_sdl_window_update(sdl_window_context_p);

        while (engine_timer_scene_should_update(timer_context_p)) {
            x += 0.1f * MS_PER_UPDATE / 200.f;
            engine_timer_scene_tick(timer_context_p);
        }

        vec2_type vec0 = {-1, -1};
        vec2_type vec1 = {1, -1};
        vec2_type vec2 = {0, 1};

        vertix_2d_type v0 = {.x = vec0[0], .y = vec0[1], .color = color_red, .ascii_char = '@'};
        vertix_2d_type v1 = {.x = vec1[0], .y = vec1[1], .color = color_blue, .ascii_char = '.'};
        vertix_2d_type v2 = {.x = vec2[0], .y = vec2[1], .color = color_green, .ascii_char = '#'};
        // draw_point_2d(&v0, 0);
        // draw_point_2d(&v1, 0);

        // draw_line_2d((vertix_2d_type[2]){v0, v1}, 0);
        // draw_line_2d((vertix_2d_type[2]){v1, v2}, 0);
        // draw_line_2d((vertix_2d_type[2]){v2, v0}, 0);
        draw_filled_triangle_2d((vertix_2d_type[3]){v0, v1, v2}, 0);

        engine_sdl_window_render(sdl_window_context_p);
        screen_refresh(screen_context_p);

        screen_clear(screen_context_p);
        engine_timer_tick(timer_context_p);
    }

    renderer_deinit();

    cmdht_destroy(g_engine_settings.cmdht_p);
    screen_destroy(screen_context_p);
    engine_sdl_window_destroy(sdl_window_context_p);
    engine_timer_destroy(timer_context_p);
}
