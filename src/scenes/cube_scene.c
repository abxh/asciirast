#include "cube_scene.h"
#include "ascii_palettes.h"
#include "engine.h"
#include "rasterizer/draw.h"
#include "rasterizer/renderer.h"
#include "scene_type.h"

#include "sc_map.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float x_mov;
    float y_mov;
    float z_mov;
    struct {
        bool left;
        bool right;
        bool down;
        bool up;
        bool in;
        bool out;
    };
} cube_scene_type;

void* cube_scene_create(struct engine_handle_type* handle_p) {
    cube_scene_type* obj_p = malloc(sizeof(cube_scene_type));
    if (!obj_p) {
        perror("cube_scene_create() failed");
        exit(1);
    }
    obj_p->x_mov = 0.f;
    obj_p->y_mov = 0.f;
    obj_p->z_mov = 0.f;

    obj_p->left = false;
    obj_p->right = false;
    obj_p->down = false;
    obj_p->up = false;
    obj_p->in = false;
    obj_p->out = false;

    sc_map_put_str(engine_get_cmd_text(handle_p), "q", "quit");
    sc_map_put_str(engine_get_cmd_text(handle_p), "left|right|up|down", "movement");
    sc_map_put_str(engine_get_cmd_text(handle_p), "+|-", "movement in/out");

    renderer_use_ascii_palette(engine_get_renderer(handle_p), sizeof(ASCII_SHORT_PALETTE), ASCII_SHORT_PALETTE);

    return (void*)obj_p;
}

void cube_scene_destroy(void* obj_p, struct engine_handle_type* handle_p) {
    sc_map_clear_str(engine_get_cmd_text(handle_p));

    free(obj_p);
}

void cube_scene_on_event(void* obj_p, struct engine_handle_type* engine_handle_p, const SDL_Event* event_p) {
    cube_scene_type* this = (cube_scene_type*)obj_p;

    switch (event_p->type) {
    case SDL_KEYDOWN:
        switch (event_p->key.keysym.sym) {
        case SDLK_q:
            engine_quit(engine_handle_p);
            break;
        case SDLK_LEFT:
            this->left = true;
            break;
        case SDLK_RIGHT:
            this->right = true;
            break;
        case SDLK_UP:
            this->up = true;
            break;
        case SDLK_DOWN:
            this->down = true;
            break;
        case SDLK_PLUS:
            this->in = true;
            break;
        case SDLK_MINUS:
            this->out = true;
            break;
        }
        break;
    case SDL_KEYUP:
        switch (event_p->key.keysym.sym) {
        case SDLK_LEFT:
            this->left = false;
            break;
        case SDLK_RIGHT:
            this->right = false;
            break;
        case SDLK_UP:
            this->up = false;
            break;
        case SDLK_DOWN:
            this->down = false;
            break;
        case SDLK_PLUS:
            this->in = false;
            break;
        case SDLK_MINUS:
            this->out = false;
            break;
        }
        break;
    }
}

void cube_scene_update(void* obj_p, uint64_t dt_ms) {
    cube_scene_type* this = (cube_scene_type*)obj_p;

    this->x_mov += (-this->left + this->right) * 0.001f * dt_ms;
    this->y_mov += (-this->down + this->up) * 0.001f * dt_ms;
    this->z_mov += (-this->in + this->out) * 0.001f * dt_ms;
}

void cube_scene_render(const void* obj_p, struct renderer_type* renderer_p) {
    const cube_scene_type* this = (cube_scene_type*)obj_p;

    vec4_type v0 = {.x = -0.5 + this->x_mov, .y = -0.5 + this->y_mov, .z = -2.f + this->z_mov, .w = 1};
    vec4_type v1 = {.x = 1 + this->x_mov, .y = -1 + this->y_mov, .z = -4.f + this->z_mov, .w = 1};

    // draw_line_3d(renderer_p,
    //              (vec4_type[2]){vec4_add(v0, (vec4_type){0, 0.5f, -0.5f, 0.f}), vec4_add(v1, (vec4_type){0, 0.5f, -0.5f, 0.f})},
    //              (prop_type[2]){{.color = g_color_red, .ascii_char = '*'}, {.color = g_color_red, .ascii_char = '*'}});
    //
    //
    // draw_line_3d(renderer_p,
    //              (vec4_type[2]){vec4_add(v0, (vec4_type){0, 0.5f, 0.f, 0.f}), vec4_add(v1, (vec4_type){0, 0.5f, 0.f, 0.f})},
    //              (prop_type[2]){{.color = g_color_red, .ascii_char = '*'}, {.color = g_color_red, .ascii_char = '*'}});
    //
    // draw_line_3d(renderer_p, (vec4_type[2]){v0, v1},
    //              (prop_type[2]){{.color = g_color_red, .ascii_char = '*'}, {.color = g_color_red, .ascii_char = '*'}});
}
