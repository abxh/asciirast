#include "engine.h"
#include "draw.h"
#include "engine_commands_view.h"
#include "screen.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_ttf.h>

#ifndef EXTRA_WINDOW_WIDTH
#define EXTRA_WINDOW_WIDTH 600
#endif

#ifndef EXTRA_WINDOW_HEIGHT
#define EXTRA_WINDOW_HEIGHT 200
#endif

#ifndef FONT_SIZE
#define FONT_SIZE 32
#endif

#ifndef FONT_PATH
#define FONT_PATH "tff/terminus.ttf"
#endif

cmdht_type* g_commands_view_ht;

static inline void sdl_poll_events(SDL_Event* event_p, bool* on_running_p) {
    while (SDL_PollEvent(event_p)) {
        switch (event_p->type) {
        case SDL_KEYUP:
            break;
        case SDL_KEYDOWN:
            switch (event_p->key.keysym.sym) {
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

#define OUTPUT_TEXT_CHUNK_SIZE (sizeof(key_comb_type) + sizeof(command_name_type) + 4)

static inline void update_output_text(char** output_text_pp, size_t* output_text_capacity_p) {
    if (*output_text_capacity_p < g_commands_view_ht->count) {
        *output_text_capacity_p *= 2;
        *output_text_pp = realloc(*output_text_pp, *output_text_capacity_p * OUTPUT_TEXT_CHUNK_SIZE);
    }

    char* p0 = *output_text_pp;
    {
        size_t count;
        key_comb_type key;
        command_name_type value;
        hashtable_for_each(g_commands_view_ht, count, key, value) {
            char* p1 = key.value;
            while (*p1 != '\0') {
                *p0++ = *p1++;
            }
            *p0++ = ':';
            *p0++ = ' ';
            p1 = value.value;
            while (*p1 != '\0') {
                *p0++ = *p1++;
            }
            if (count != 1) {
                *p0++ = ',';
                *p0++ = ' ';
            }
        }
    }
    *p0++ = '\0';
}

static inline void sdl_refresh_window(SDL_Renderer* renderer_p, TTF_Font* font_p, char* text_p, int win_width) {
    SDL_SetRenderDrawColor(renderer_p, 255, 255, 255, 255);
    SDL_RenderClear(renderer_p);

    SDL_Surface* text_surface = TTF_RenderText_Solid_Wrapped(font_p, text_p, (SDL_Color){0, 0, 0, 0}, (uint32_t)win_width);
    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer_p, text_surface);
    SDL_Rect text_rect = {.x = 0, .y = 0, .w = text_surface->w, .h = text_surface->h};

    SDL_RenderCopy(renderer_p, text_texture, NULL, &text_rect);
    SDL_FreeSurface(text_surface);
    SDL_DestroyTexture(text_texture);

    SDL_RenderPresent(renderer_p);
}

void run(const scene_type default_scene) {
    // init:
    screen_init(stdout);

    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window_p = SDL_CreateWindow("ascii-rasterizer - controls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                            EXTRA_WINDOW_WIDTH, EXTRA_WINDOW_HEIGHT, 0);
    SDL_Renderer* renderer_p = SDL_CreateRenderer(window_p, -1, 0);

    g_commands_view_ht = cmdht_create_with_initial_capacity(16);
    cmdht_insert(g_commands_view_ht, (key_comb_type){"q"}, (command_name_type){"quit"});

    TTF_Init();
    TTF_Font* font_p = TTF_OpenFont(FONT_PATH, FONT_SIZE);
    if (font_p == NULL) {
        fprintf(stderr, "error: '" FONT_PATH "' not found\n");
        exit(EXIT_FAILURE);
    }
    size_t output_text_capacity = 16;
    char* output_text_p = malloc(output_text_capacity * OUTPUT_TEXT_CHUNK_SIZE);
    uint32_t hash = murmur3_32((const uint8_t*)output_text_p, output_text_capacity, 0);

    int win_width, win_height;
    SDL_GetWindowSizeInPixels(window_p, &win_width, &win_height);

    float x = FROM_ANGLE_DEG_TO_ANGLE_RAD(90);

    SDL_Event event;
    bool on_running = true;
    uint64_t previous_time_ms = SDL_GetTicks64();
    int lag_ms = 0.;

    while (on_running) {
        const uint64_t current_time_ms = SDL_GetTicks64();
        const uint64_t elapsed_ms = current_time_ms - previous_time_ms;
        previous_time_ms = current_time_ms;
        lag_ms += elapsed_ms;

        sdl_poll_events(&event, &on_running);

        update_output_text(&output_text_p, &output_text_capacity);
        const uint32_t new_hash = murmur3_32((const uint8_t*)output_text_p, output_text_capacity, 0);

        if (hash != new_hash) {
            sdl_refresh_window(renderer_p, font_p, output_text_p, win_width);
            hash = new_hash;
        }

        const vec3_type v0 = {.x = -1, .y = -1, .z = 0 + x};
        const vec3_type v1 = {.x = 1, .y = -1, .z = 0 + x};
        const vec3_type v2 = {.x = 0, .y = 1, .z = 0 + x};

        // draw_filled_triangle_2d_w_interpolated_color_and_z_order((vec2_type[3]){v0, v1, v2},
        //                                                          (color_type[3]){color_red, color_blue, color_magenta}, 1, '#');
        //
        // draw_line_2d_w_interpolated_color_and_z_order((vec2_type[2]){{.x = -1, .y = -1}, {.x = 1, .y = 1}},
        //                                               (color_type[2]){color_black, color_white}, 3, '*');
        //
        // draw_point_2d_w_color_and_z_order(v0, color_white, 2, '0');
        // draw_point_2d_w_color_and_z_order(v1, color_white, 2, '1');
        // draw_point_2d_w_color_and_z_order(v2, color_white, 2, '2');
        //
        // draw_line_2d_w_interpolated_color(
        //     (vec2_type[2]){rotate_vec2_around_origo((vec2_type){-10.f, 0}, x), rotate_vec2_around_origo((vec2_type){10.f, 0}, x)},
        //     (color_type[2]){color_white, color_black}, '*');

        draw_line_3d(v0, v1, '*');
        draw_line_3d(v1, v2, '*');
        draw_line_3d(v2, v0, '*');

        // draw_point_3d((vec3_type){.x = 0, .y = 0, .z = 2}, '*');

        while (lag_ms >= MS_PER_UPDATE) {
            x += 0.01f; // update
            lag_ms -= MS_PER_UPDATE;
        }

        screen_refresh();
        screen_clear();
    }

    // deinit:
    screen_deinit();
    cmdht_destroy(g_commands_view_ht);
    free(output_text_p);
    TTF_CloseFont(font_p);
    TTF_Quit();
    SDL_DestroyRenderer(renderer_p);
    SDL_DestroyWindow(window_p);
    SDL_Quit();

    exit(0);
}
