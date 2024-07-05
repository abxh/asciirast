#include "engine.h"
#include "ascii_palettes.h"
#include "draw.h"
#include "engine_sdl_window.h"
#include "engine_timer.h"
#include "screen.h"

#include <SDL2/SDL.h>

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

void engine_run(const scene_type default_scene) {
    set_cmd_desc desc_set = set_cmd_desc_init(cmd_desc_cmp);
    set_cmd_desc_insert(&desc_set, "q: quit");

    struct screen_type* screen_p = screen_create(stdout);
    struct engine_sdl_window_type* sdl_window_p = engine_sdl_window_create(desc_set);
    struct engine_timer_type* timer_p = engine_timer_create();
    struct renderer_type* renderer_p = renderer_create(
        screen_p, sizeof(ascii_short_palette) - 1, ascii_short_palette,
        (perspective_proj_prop_type){.fovy_rad = DEG_TO_ANGLE_RAD(60), .aspect_ratio = ASPECT_RATIO, .z_near = 1.f, .z_far = 100.f});

    float x = 0;

    bool on_running = true;
    while (on_running) {
        sdl_poll_events(&on_running);
        engine_sdl_window_update(sdl_window_p);

        while (engine_timer_scene_should_update(timer_p)) {
            x += 0.1f * MS_PER_UPDATE / 200.f;

            engine_timer_scene_tick(timer_p);
        }

        vec4_type vec0 = {-1.f, -1.f, -3.f - x, 1.f};
        vec4_type vec1 = {1.f, -1.f, -3.f - x, 1.f};
        vec4_type vec2 = {1.f, 1.f, -3.f - x, 1.f};

        vertix_2d_type v0 = {.pos = {vec0[0], vec0[1]}, .prop = {.color = color_red, .ascii_char = '@'}};
        vertix_2d_type v1 = {.pos = {vec1[0], vec1[1]}, .prop = {.color = color_blue, .ascii_char = '%'}};
        vertix_2d_type v2 = {.pos = {vec2[0], vec2[1]}, .prop = {.color = color_green, .ascii_char = '.'}};

        vertix_3d_type v0_3d = {.pos = {vec0[0], vec0[1], vec0[2], vec0[3]}, .prop = {.color = color_red, .ascii_char = '@'}};
        vertix_3d_type v1_3d = {.pos = {vec1[0], vec1[1], vec1[2], vec1[3]}, .prop = {.color = color_blue, .ascii_char = '%'}};
        vertix_3d_type v2_3d = {.pos = {vec2[0], vec2[1], vec2[2], vec2[3]}, .prop = {.color = color_green, .ascii_char = '.'}};

        // draw_point_2d(renderer_p, &v0, 0);
        // draw_point_2d(renderer_p, &v1, 0);
        // draw_point_2d(renderer_p, &v2, 0);

        // draw_line_2d(renderer_p, (vertix_2d_type[2]){v0, v1}, 0);
        // draw_line_2d(renderer_p, (vertix_2d_type[2]){v1, v2}, 0);
        // draw_line_2d(renderer_p, (vertix_2d_type[2]){v2, v0}, 0);
        draw_filled_triangle_2d(renderer_p, (vertix_2d_type[3]){v0, v1, v2}, 0);

        // draw_point_3d(renderer_p, &v0_3d);
        // draw_point_3d(renderer_p, &v1_3d);
        // draw_point_3d(renderer_p, &v2_3d);

        // draw_line_3d(renderer_p, (vertix_3d_type[2]){v0_3d, v1_3d});
        // draw_line_3d(renderer_p, (vertix_3d_type[2]){v1_3d, v2_3d});
        // draw_line_3d(renderer_p, (vertix_3d_type[2]){v2_3d, v0_3d});

        engine_sdl_window_render(sdl_window_p);
        screen_refresh(screen_p);

        screen_clear(screen_p);
        engine_timer_tick(timer_p);
    }

    renderer_destroy(renderer_p);
    set_cmd_desc_free(&desc_set);
    screen_destroy(screen_p);
    engine_sdl_window_destroy(sdl_window_p);
    engine_timer_destroy(timer_p);
}
