#include "spiral_scene.h"
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
} spiral_scene_type;

void* spiral_scene_create(struct engine_handle_type* handle_p) {
    spiral_scene_type* obj_p = malloc(sizeof(spiral_scene_type));
    if (!obj_p) {
        perror("spiral_scene_create() failed");
        exit(1);
    }
    obj_p->angle_deg = 0.f;
    obj_p->left = false;
    obj_p->right = false;

    sc_map_put_str(engine_get_cmd_text(handle_p), "q", "quit");
    sc_map_put_str(engine_get_cmd_text(handle_p), "left|right", "rotate");

    renderer_use_ascii_palette(engine_get_renderer(handle_p), sizeof(g_ascii_short_palette), g_ascii_short_palette);

    return (void*)obj_p;
}

void spiral_scene_destroy(void* obj_p, struct engine_handle_type* handle_p) {
    sc_map_del_str(engine_get_cmd_text(handle_p), "q");

    free(obj_p);
}

void spiral_scene_on_event(void* obj_p, struct engine_handle_type* engine_handle_p, const SDL_Event* event_p) {
    spiral_scene_type* this = (spiral_scene_type*)obj_p;

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

void spiral_scene_update(void* obj_p, uint64_t dt_ms) {
    spiral_scene_type* this = (spiral_scene_type*)obj_p;

    this->angle_deg += (this->left - this->right) * 0.1f * dt_ms;
}

void spiral_scene_render(const void* obj_p, struct renderer_type* renderer_p) {
    const spiral_scene_type* this = (spiral_scene_type*)obj_p;

    for (size_t o = 0; o < 6; o++) {
        for (size_t i = 0; i <= 6; i++) {
            const float angle_rad = ANGLE_DEG_TO_RAD((this->angle_deg - 20.f * (float)i - 60.f * (float)o));

            const vec2_type v_base = {.x = 0.6f - 0.1f * (float)i, .y = 0.6f - 0.1f * (float)i};

            const vec2_type v = vec2_rotate_origo(v_base, angle_rad);

            const prop_type prop = {.color = color_lerp(g_color_red, g_color_yellow, i / 6.f), .ascii_char = g_ascii_short_palette[i]};

            draw_point_2d(renderer_p, (vec2_type[1]){v}, (prop_type[1]){prop}, 0);
        }
    }
}
