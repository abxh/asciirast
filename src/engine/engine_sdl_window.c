#include "engine/engine_sdl_window.h"

#include "sc_map.h"
#include "sc_str.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_error.h>
#include <SDL2/SDL_ttf.h>

#include <stdlib.h>

typedef struct {
    TTF_Font* sdl_font_obj;
    SDL_Surface* sdl_surface_obj;
    SDL_Texture* sdl_text_obj;
    SDL_Rect sdl_rect;

    size_t prev_size;
    char* cmd_str;
    struct sc_map_str* cmd_map_p;
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
    sc_str_destroy(&this->text.cmd_str);
    this->text.cmd_str = sc_str_create(this->text.cmd_map_p->size == 0 ? " " : ""); // roundabout since SDL errs with 0-size string.
    const char *key, *value;
    sc_map_foreach(this->text.cmd_map_p, key, value) {
        sc_str_append_fmt(&this->text.cmd_str, "%s: %s, ", key, value);
    }

    if (this->text.sdl_surface_obj != NULL) {
        SDL_FreeSurface(this->text.sdl_surface_obj);
        SDL_DestroyTexture(this->text.sdl_text_obj);
    }

    this->text.sdl_surface_obj = TTF_RenderText_Solid_Wrapped(this->text.sdl_font_obj, this->text.cmd_str, (SDL_Color){0, 0, 0, 0},
                                                              (uint32_t)this->win.width_pixels);
    if (!this->text.sdl_surface_obj) {
        printf("%s\n", SDL_GetError());
        exit(1);
    }
    this->text.sdl_text_obj = SDL_CreateTextureFromSurface(this->win.sdl_rend_obj, this->text.sdl_surface_obj);
    if (!this->text.sdl_text_obj) {
        printf("%s\n", SDL_GetError());
        exit(1);
    }
    this->text.sdl_rect = (SDL_Rect){.x = 0, .y = 0, .w = this->text.sdl_surface_obj->w, .h = this->text.sdl_surface_obj->h};
}

static inline void sdl_render_text(const engine_sdl_window_type* this) {
    SDL_SetRenderDrawColor(this->win.sdl_rend_obj, 255, 255, 255, 255);
    SDL_RenderClear(this->win.sdl_rend_obj);
    SDL_RenderCopy(this->win.sdl_rend_obj, this->text.sdl_text_obj, NULL, &this->text.sdl_rect);
    SDL_RenderPresent(this->win.sdl_rend_obj);
}

engine_sdl_window_type* engine_sdl_window_create(struct sc_map_str* cmd_map_p) {
    engine_sdl_window_type* this = malloc(sizeof(engine_sdl_window_type));
    if (!this) {
        perror("engine_sdl_window_create");
        exit(1);
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("%s", SDL_GetError());
        exit(1);
    }
    this->win.sdl_win_obj = SDL_CreateWindow("ascii-rasterizer - controls", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                             SDL_WINDOW_WIDTH, SDL_WINDOW_HEIGHT, 0);
    if (!this->win.sdl_win_obj) {
        printf("%s", SDL_GetError());
        exit(1);
    }

    this->win.sdl_rend_obj = SDL_CreateRenderer(this->win.sdl_win_obj, -1, 0);
    if (!this->win.sdl_rend_obj) {
        printf("%s", SDL_GetError());
        exit(1);
    }

    SDL_GetWindowSizeInPixels(this->win.sdl_win_obj, &this->win.width_pixels, &this->win.height_pixels);

    if (TTF_Init() < 0) {
        printf("%s", SDL_GetError());
        exit(1);
    }

    this->text.sdl_font_obj = TTF_OpenFont(SDL_FONT_PATH, SDL_FONT_SIZE);
    if (this->text.sdl_font_obj == NULL) {
        printf("%s", SDL_GetError());
        exit(1);
    }
    this->text.sdl_surface_obj = NULL;
    this->text.sdl_text_obj = NULL;

    this->text.cmd_str = sc_str_create("");
    this->text.cmd_map_p = cmd_map_p;
    this->text.prev_size = cmd_map_p->size;

    update_text(this);

    return this;
}

void engine_sdl_window_destroy(engine_sdl_window_type* this) {
    SDL_FreeSurface(this->text.sdl_surface_obj);
    SDL_DestroyTexture(this->text.sdl_text_obj);
    TTF_CloseFont(this->text.sdl_font_obj);
    TTF_Quit();
    sc_str_destroy(&this->text.cmd_str);

    SDL_DestroyRenderer(this->win.sdl_rend_obj);
    SDL_DestroyWindow(this->win.sdl_win_obj);
    SDL_Quit();

    free(this);
}

void engine_sdl_window_update(engine_sdl_window_type* this) {
    if (this->text.prev_size != this->text.cmd_map_p->size) {
        update_text(this);
        this->text.prev_size = this->text.cmd_map_p->size;
    }
}

void engine_sdl_window_render(engine_sdl_window_type* this) {
    sdl_render_text(this);
}
