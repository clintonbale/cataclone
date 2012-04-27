#include "catacomb_scores.h"
#include "../common.h"

#include <string.h>
#include <stdio.h>

static const highscore_t defualt_hs[MAX_HIGHSCORES] = {
    {100, 1, "JDC"},
    {100, 1, "JDC"},
    {100, 1, "JDC"},
    {100, 1, "JDC"},
    {100, 1, "JDC"}
};
static const char* highscore_file = "TOPSCORS.CAT";
static highscore_t highscores[MAX_HIGHSCORES];

const highscore_t* catacomb_scores_get(void) {
    return (const highscore_t*)&highscores;
}

void catacomb_scores_load(void) {
    FILE* fp = NULL;

    fp = fopen(highscore_file, "rb");
    if(!fp) {
        debug("scores_load: No score file found, creating default scores.");
        memcpy(&highscores[0], &defualt_hs[0], sizeof(highscore_t)*MAX_HIGHSCORES);

        catacomb_scores_save();
        return;
    }

    for(unsigned i = 0; i < MAX_HIGHSCORES; ++i) {
        if(fread(&highscores[i], 1, HS_STRUCT_SIZE, fp) != HS_STRUCT_SIZE) {
            error("scores_load: Error reading highscore at index %d.", i);
        }
    }

    fclose(fp);
}

void catacomb_scores_save(void) {
    FILE* fp = NULL;

    fp = fopen(highscore_file, "wb");
    if(!fp) {
        error("scores_save: Cannot open file '%s'", highscore_file);
    }

    for(unsigned i = 0; i < MAX_HIGHSCORES; ++i) {
        if(fwrite(&highscores[i], 1, HS_STRUCT_SIZE, fp) != HS_STRUCT_SIZE) {
            error("scores_save: Error writing highscore to file.");
        }
    }

    fclose(fp);
}

void catacomb_scores_add(int score, short level, char name[3]) {
    catacomb_scores_load();

    //TODO: Add support for same points different level...
    for(unsigned i = 0; i < MAX_HIGHSCORES; i++) {
        if(highscores[i].score < score) {
            if(i + 1 < MAX_HIGHSCORES)
                memcpy(&highscores[i+1], &highscores[i], HS_STRUCT_SIZE*(MAX_HIGHSCORES-i));
            highscores[i].score = score;
            highscores[i].level = level;
            memcpy(&highscores[i].name, name, sizeof(name));
            break;
        }
    }

    catacomb_scores_save();
}
