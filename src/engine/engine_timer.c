#include "engine/engine_timer.h"

#include <SDL2/SDL_timer.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef struct engine_timer_type {
    uint64_t previous_time_ms;
    uint64_t elapsed_ms;
    int lag_ms;
} engine_timer_type;

engine_timer_type* engine_timer_create(void) {
    engine_timer_type* this = malloc(sizeof(engine_timer_type));
    if (!this) {
        perror("engine_timer_create() failed");
        exit(1);
    }

    this->previous_time_ms = SDL_GetTicks64();
    this->lag_ms = 0.;

    return this;
}

void engine_timer_destroy(engine_timer_type* this) {
    free(this);
}

uint64_t engine_timer_get_elapsed_time_ms(struct engine_timer_type* this) {
    return this->elapsed_ms;
}

void engine_timer_frame_tick(engine_timer_type* this) {
    const uint64_t current_time_ms = SDL_GetTicks64();
    this->elapsed_ms = current_time_ms - this->previous_time_ms;
    this->previous_time_ms = current_time_ms;
    this->lag_ms += this->elapsed_ms;
}

bool engine_timer_scene_should_update(engine_timer_type* this) {
    return this->lag_ms >= 0;
}

void engine_timer_scene_tick(engine_timer_type* this) {
    this->lag_ms -= MS_PER_UPDATE;
}
