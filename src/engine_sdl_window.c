
#include "engine_sdl_window.h"
#include "engine.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdlib.h>

#define OUTPUT_TEXT_CHUNK_SIZE (sizeof(key_comb_type) + sizeof(command_name_type) + 4)

typedef struct {
    TTF_Font* sdl_font_obj;
    SDL_Surface* sdl_surface_obj;
    SDL_Texture* sdl_text_obj;
    SDL_Rect sdl_rect;

    char* buf_p;
    size_t buf_capacity;
    size_t prev_entry_count;
} sdl_text_type;

typedef struct {
    int width_pixels;
    int height_pixels;

    SDL_Window* sdl_win_obj;
    SDL_Renderer* sdl_rend_obj;
} sdl_window_type;

typedef struct engine_sdl_window_type {
    sdl_window_type win;
    sdl_text_type text;
} engine_sdl_window_type;

static inline void update_text(engine_sdl_window_type* this) {
    if (this->text.buf_capacity < g_engine_settings.cmdht_p->count) {
        this->text.buf_capacity *= 2;
        void* tmp = realloc(this->text.buf_p, this->text.buf_capacity * OUTPUT_TEXT_CHUNK_SIZE);
        if (!tmp) {
            abort();
        }
        this->text.buf_p = tmp;
    }

    char* p0 = this->text.buf_p;
    {
        size_t count;
        key_comb_type key;
        command_name_type value;
        hashtable_for_each(g_engine_settings.cmdht_p, count, key, value) {
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

    if (this->text.sdl_surface_obj != NULL) {
        SDL_FreeSurface(this->text.sdl_surface_obj);
        SDL_DestroyTexture(this->text.sdl_text_obj);
    }

    this->text.sdl_surface_obj = TTF_RenderText_Solid_Wrapped(this->text.sdl_font_obj, this->text.buf_p, (SDL_Color){0, 0, 0, 0},
                                                              (uint32_t)this->win.width_pixels);
    if (!this->text.sdl_surface_obj) {
        abort();
    }
    this->text.sdl_text_obj = SDL_CreateTextureFromSurface(this->win.sdl_rend_obj, this->text.sdl_surface_obj);
    if (!this->text.sdl_text_obj) {
        abort();
    }
    this->text.sdl_rect = (SDL_Rect){.x = 0, .y = 0, .w = this->text.sdl_surface_obj->w, .h = this->text.sdl_surface_obj->h};
}

static inline void sdl_render_text(const engine_sdl_window_type* this) {
    SDL_SetRenderDrawColor(this->win.sdl_rend_obj, 255, 255, 255, 255);
    SDL_RenderClear(this->win.sdl_rend_obj);
    SDL_RenderCopy(this->win.sdl_rend_obj, this->text.sdl_text_obj, NULL, &this->text.sdl_rect);
    SDL_RenderPresent(this->win.sdl_rend_obj);
}

engine_sdl_window_type* engine_sdl_window_create(void) {
    engine_sdl_window_type* this = malloc(sizeof(engine_sdl_window_type));

    if (!this) {
        abort();
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        abort();
    }
    this->win.sdl_win_obj = SDL_CreateWindow("ascii-rasterizer - controls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                             SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!this->win.sdl_win_obj) {
        abort();
    }
    this->win.sdl_rend_obj = SDL_CreateRenderer(this->win.sdl_win_obj, -1, 0);
    if (!this->win.sdl_rend_obj) {
        abort();
    }
    SDL_GetWindowSizeInPixels(this->win.sdl_win_obj, &this->win.width_pixels, &this->win.height_pixels);

    if (TTF_Init() < 0) {
        abort();
    }
    this->text.sdl_font_obj = TTF_OpenFont(SDL_FONT_PATH, SDL_FONT_SIZE);
    if (this->text.sdl_font_obj == NULL) {
        fprintf(stderr, "error: '" SDL_FONT_PATH "' not found\n");
        exit(EXIT_FAILURE);
    }
    this->text.buf_capacity = 16;
    this->text.buf_p = calloc(OUTPUT_TEXT_CHUNK_SIZE, this->text.buf_capacity);
    if (this->text.buf_p == NULL) {
        abort();
    }
    this->text.prev_entry_count = 0;
    this->text.sdl_surface_obj = NULL;
    this->text.sdl_text_obj = NULL;

    update_text(this);

    return this;
}

void engine_sdl_window_destroy(engine_sdl_window_type* this) {
    SDL_FreeSurface(this->text.sdl_surface_obj);
    SDL_DestroyTexture(this->text.sdl_text_obj);
    TTF_CloseFont(this->text.sdl_font_obj);
    TTF_Quit();
    free(this->text.buf_p);

    SDL_DestroyRenderer(this->win.sdl_rend_obj);
    SDL_DestroyWindow(this->win.sdl_win_obj);
    SDL_Quit();

    free(this);
}

void engine_sdl_window_update(engine_sdl_window_type* this) {
    if (this->text.prev_entry_count != g_engine_settings.cmdht_p->count) {
        update_text(this);
        this->text.prev_entry_count = g_engine_settings.cmdht_p->count;
    }
}

void engine_sdl_window_render(const engine_sdl_window_type* this) {
    sdl_render_text(this);
}
