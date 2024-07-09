#include "star_scene.h"
#include "ascii_palettes.h"
#include "engine.h"
#include "math/angle.h"
#include "rasterizer/color.h"
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
    float angle_deg;
    struct {
        bool left;
        bool right;
    };
} star_scene_type;

void* star_scene_create(struct engine_handle_type* handle_p) {
    star_scene_type* obj_p = malloc(sizeof(star_scene_type));
    if (!obj_p) {
        perror("star_scene_create() failed");
        exit(1);
    }
    obj_p->angle_deg = 0.f;
    obj_p->left = false;
    obj_p->right = false;

    sc_map_put_str(engine_get_cmd_text(handle_p), "q", "quit");
    sc_map_put_str(engine_get_cmd_text(handle_p), "left|right", "rotate");

    renderer_use_ascii_palette(engine_get_renderer(handle_p), sizeof(g_ascii_short_palette_w_numbers),
                               g_ascii_short_palette_w_numbers);

    return (void*)obj_p;
}

void star_scene_destroy(void* obj_p, struct engine_handle_type* handle_p) {
    sc_map_del_str(engine_get_cmd_text(handle_p), "q");

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
        const vec2_type v_base = {.x = 0.6f, .y = -0.6f};

        const float angle_rad1 = ANGLE_DEG_TO_RAD((float)(this->angle_deg + (72 + 72) * ((int)l + 0)));
        const float angle_rad2 = ANGLE_DEG_TO_RAD((float)(this->angle_deg + (72 + 72) * ((int)l + 1)));

        const vec2_type v1 = vec2_rotate_origo(v_base, angle_rad1);
        const vec2_type v2 = vec2_rotate_origo(v_base, angle_rad2);

        // draw_point_2d(renderer_p, (vec2_type[1]){v1}, (prop_type[1]){{.color = g_color_green, .ascii_char = l + '0'}}, 1);
        // draw_line_2d(renderer_p, (vec2_type[2]){v1, v2},
        //              (prop_type[2]){{.color = g_color_red, .ascii_char = '*'}, {.color = g_color_green, .ascii_char = '.'}}, 0);

        draw_edge_2d(renderer_p, (vec2_type[2]){v1, v2}, g_color_white, 0);
    }
}
