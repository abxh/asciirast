#include "../example_scenes/example_scenes.h"
#include "misc.h"
#include "scene.h"
#include "screen.h"

#ifdef DEBUG
#include "time.h"
#endif

#include <signal.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static void on_sigint(int sig) {
    MARK_UNUSED(sig);
    screen_deinit();
    exit(0);
}

int main(void) {
    enable_raw_mode();
    screen_init();
    signal(SIGINT, on_sigint);

    const scene_type scene = g_rgb_triangle_scene; // change scene here
    void** context_ptr = scene.create();

    bool on_running = true;
    int previous_time_ms = get_current_time_ms();
    int lag_ms = 0.;

    while (on_running) {
        const int current_time_ms = get_current_time_ms();
        const int elapsed_ms = current_time_ms - previous_time_ms;
        previous_time_ms = current_time_ms;
        lag_ms += elapsed_ms;

        char c;
        if (on_key(&c)) {
            if (c == 'q') {
                on_running = false;
            }
            if (scene.flags & SCENE_OPS_ON_KEY) {
                scene.on_key(context_ptr, c);
            }
        }

        while (lag_ms >= MS_PER_UPDATE) {
            scene.update(context_ptr);
            lag_ms -= MS_PER_UPDATE;
        }

        scene.render(context_ptr);

        printf("q: exit" NEW_LINE);
        g_extra_lines += 1;
        screen_restore_line_cursor();

        screen_refresh();
        screen_clear();
    }

    scene.destroy(context_ptr);
    screen_deinit();

    return 0;
}
