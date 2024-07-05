#include "engine_sdl_window.h"
#include "log.h"

#include <str.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>

#include <stdlib.h>

typedef struct {
    TTF_Font* sdl_font_obj;
    SDL_Surface* sdl_surface_obj;
    SDL_Texture* sdl_text_obj;
    SDL_Rect sdl_rect;

    size_t prev_size;
    str desc_str;
    set_cmd_desc desc_set;
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
    str_clear(&this->text.desc_str);
    foreach (set_cmd_desc, &this->text.desc_set, it) {
        str_append(&this->text.desc_str, *it.ref);
        str_append(&this->text.desc_str, (it.next == it.end) ? "." : ", ");
    }

    if (this->text.sdl_surface_obj != NULL) {
        SDL_FreeSurface(this->text.sdl_surface_obj);
        SDL_DestroyTexture(this->text.sdl_text_obj);
    }

    this->text.sdl_surface_obj = TTF_RenderText_Solid_Wrapped(this->text.sdl_font_obj, this->text.desc_str.value,
                                                              (SDL_Color){0, 0, 0, 0}, (uint32_t)this->win.width_pixels);
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

engine_sdl_window_type* engine_sdl_window_create(set_cmd_desc desc_set) {
    engine_sdl_window_type* this = malloc(sizeof(engine_sdl_window_type));
    HANDLE_NULL(this, "malloc");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        abort();
    }
    this->win.sdl_win_obj = SDL_CreateWindow("ascii-rasterizer - controls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                             SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    HANDLE_NULL(this->win.sdl_win_obj, "SDL_CreateWindow");

    this->win.sdl_rend_obj = SDL_CreateRenderer(this->win.sdl_win_obj, -1, 0);
    HANDLE_NULL(this->win.sdl_win_obj, "SDL_CreateRenderer");

    SDL_GetWindowSizeInPixels(this->win.sdl_win_obj, &this->win.width_pixels, &this->win.height_pixels);

    if (TTF_Init() < 0) {
        abort();
    }

    this->text.sdl_font_obj = TTF_OpenFont(SDL_FONT_PATH, SDL_FONT_SIZE);
    if (this->text.sdl_font_obj == NULL) {
        fprintf(stderr, "error: '" SDL_FONT_PATH "' not found\n");
        exit(EXIT_FAILURE);
    }
    this->text.sdl_surface_obj = NULL;
    this->text.sdl_text_obj = NULL;

    this->text.desc_str = str_init("");
    this->text.desc_set = desc_set;
    this->text.prev_size = desc_set.size;

    update_text(this);

    return this;
}

void engine_sdl_window_destroy(engine_sdl_window_type* this) {
    SDL_FreeSurface(this->text.sdl_surface_obj);
    SDL_DestroyTexture(this->text.sdl_text_obj);
    TTF_CloseFont(this->text.sdl_font_obj);
    TTF_Quit();
    str_free(&this->text.desc_str);

    SDL_DestroyRenderer(this->win.sdl_rend_obj);
    SDL_DestroyWindow(this->win.sdl_win_obj);
    SDL_Quit();

    free(this);
}

void engine_sdl_window_update(engine_sdl_window_type* this) {
    if (this->text.prev_size != this->text.desc_set.size) {
        update_text(this);
        this->text.prev_size = this->text.desc_set.size;
    }
}

void engine_sdl_window_render(const engine_sdl_window_type* this) {
    sdl_render_text(this);
}
