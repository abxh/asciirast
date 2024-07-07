#include "spiral_scene.h"
#include "ascii_palettes.h"
#include "engine.h"
#include "math/angle.h"
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
} spiral;

void* spiral_scene_create(struct engine_handle_type* handle_p) {
    spiral* obj_p = malloc(sizeof(spiral));
    if (!obj_p) {
        perror("spiral_scene_create() failed");
        exit(1);
    }
    obj_p->angle_deg = 0.f;

    sc_map_put_str(engine_get_cmd_text(handle_p), "q", "quit");

    renderer_use_ascii_palette(engine_get_renderer(handle_p), sizeof(g_ascii_short_palette), g_ascii_short_palette);

    return (void*)obj_p;
}

void spiral_scene_destroy(void* obj_p, struct engine_handle_type* handle_p) {
    sc_map_del_str(engine_get_cmd_text(handle_p), "q");

    free(obj_p);
}

void spiral_scene_on_event(void* obj_p, struct engine_handle_type* engine_handle_p, const SDL_Event* event_p) {
    if (event_p->type != SDL_KEYDOWN) {
        return;
    }

    switch (event_p->key.keysym.sym) {
    case SDLK_q:
        engine_quit(engine_handle_p);
        break;
    }
}

void spiral_scene_update(void* obj_p, uint64_t dt_ms) {
    spiral* this = (spiral*)obj_p;

    this->angle_deg += 0.1f * dt_ms;
}

void spiral_scene_render(const void* obj_p, struct renderer_type* renderer_p) {
    spiral* this = (spiral*)obj_p;

    for (size_t o = 0; o < 6; o++) {
        for (size_t i = 0; i <= 6; i++) {
            float angle_rad = DEG_TO_ANGLE_RAD((-this->angle_deg - 20.f * (float)i - 60.f * (float)o));

            vec2_type v_base = {.x = 0.6f - 0.1f * (float)i, .y = 0.6f - 0.1f * (float)i};

            vec2_type v = vec2_rotate_origo(v_base, angle_rad);

            draw_point_2d(renderer_p, (vec2_type[1]){v},
                          (prop_type[1]){{.color = (color_type){.vec3 = vec3_add(vec3_scale(g_color_red.vec3, i / 6.f),
                                                                                 vec3_scale(g_color_green.vec3, o / 6.f))},
                                          .ascii_char = g_ascii_short_palette[i]}},
                          0U);
        }
    }
}
