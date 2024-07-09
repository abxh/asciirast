
#include "engine.h"

#include "scenes/ascii_palettes.h"

#include "scenes/spiral_scene.h"
#include "scenes/star_scene.h"

int main(void) {
    engine_run(g_star_scene, sizeof(g_ascii_short_palette_w_numbers), g_ascii_short_palette_w_numbers);
    return 0;
}
