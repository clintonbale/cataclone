#include "catacomb.h"

void catacomb_init_all(void) {
    catacomb_graphics_init();
    catacomb_sounds_load("SOUNDS.CAT");
    catacomb_level_init();
    catacomb_level_change(1);
#if LOAD_HIGHSCORES
    catacomb_scores_load();
#endif
}

void catacomb_finish_all(void) {
    catacomb_graphics_finish();
    catacomb_sounds_finish();
    catacomb_level_finish();
}
