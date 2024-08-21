#include "star_scene.h"
#include "ascii_palettes.h"
#include "engine.h"
#include "math/angle.h"
#include "rasterizer/color.h"
#include "rasterizer/draw.h"
#include "rasterizer/renderer.h"
#include "rasterizer/screen.h"
#include "scene_type.h"

#include "sc_map.h"

#include <SDL2/SDL_events.h>
#include <SDL2/SDL_keycode.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    float angle_deg;
    float zoom;
    struct {
        bool left;
        bool right;
    };
} star_scene_type;

void* star_scene_create(struct engine_handle_type* handle_p) {
    star_scene_type* this = malloc(sizeof(star_scene_type));
    if (!this) {
        perror("star_scene_create() failed");
        exit(1);
    }
    this->angle_deg = 0.f;
    this->left = false;
    this->right = false;
    this->zoom = 0.6f;

    sc_map_put_str(engine_get_cmd_text(handle_p), "q", "quit");
    sc_map_put_str(engine_get_cmd_text(handle_p), "left|right", "rotate");
    sc_map_put_str(engine_get_cmd_text(handle_p), "+|-", "zoom");

    renderer_use_ascii_palette(engine_get_renderer(handle_p), sizeof(ASCII_SHORT_PALETTE "[]_0123456789"),
                               ASCII_SHORT_PALETTE "[]_0123456789");

    return (void*)this;
}

void star_scene_destroy(void* obj_p, struct engine_handle_type* handle_p) {
    sc_map_clear_str(engine_get_cmd_text(handle_p));

    free(obj_p);
}

void star_scene_on_event(void* obj_p, struct engine_handle_type* engine_handle_p, const SDL_Event* event_p) {
    star_scene_type* this = (star_scene_type*)obj_p;

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
        case SDLK_PLUS:
            this->zoom += 0.1f;
            break;
        case SDLK_MINUS:
            this->zoom -= 0.1f;
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
        }
        break;
    }
}

void star_scene_update(void* obj_p, uint64_t dt_ms) {
    star_scene_type* this = (star_scene_type*)obj_p;

    this->angle_deg += (this->left - this->right) * 0.1f * dt_ms;
}

void star_scene_render(const void* obj_p, struct renderer_type* renderer_p) {
    const star_scene_type* this = (star_scene_type*)obj_p;

    for (size_t l = 0; l < 5; l++) {
        const vec2_type v_base = {.x = this->zoom, .y = -this->zoom};

        const float angle_rad1 = ANGLE_DEG_TO_RAD((float)(this->angle_deg + (72 + 72) * ((int)l + 0)));
        const float angle_rad2 = ANGLE_DEG_TO_RAD((float)(this->angle_deg + (72 + 72) * ((int)l + 1)));

        const vec2_type v1 = vec2_rotate_origo(v_base, angle_rad1);
        const vec2_type v2 = vec2_rotate_origo(v_base, angle_rad2);

        draw_point_2d(renderer_p, (vec2_type[1]){v1},
                      (prop_type[1]){{.color = color_scale(g_color_green, 0.7f), .ascii_char = l + '0'}}, 2);

        draw_point_2d(renderer_p, (vec2_type[1]){vec2_add(v1, (vec2_type){.x = -2.f / (SCREEN_WIDTH - 1.f), .y = 0.f})},
                      (prop_type[1]){{.color = g_color_white, .ascii_char = '['}}, 2);
        draw_point_2d(renderer_p, (vec2_type[1]){vec2_add(v1, (vec2_type){.x = 2.f / (SCREEN_WIDTH - 1.f), .y = 0.f})},
                      (prop_type[1]){{.color = g_color_white, .ascii_char = ']'}}, 2);
        draw_point_2d(renderer_p, (vec2_type[1]){vec2_add(v1, (vec2_type){.x = 0.f, .y = 2.f / (SCREEN_HEIGHT - 1.f)})},
                      (prop_type[1]){{.color = g_color_white, .ascii_char = '_'}}, 0);

        draw_edge_2d(renderer_p, (vec2_type[2]){v1, v2}, g_color_white, 1);
    }
}
