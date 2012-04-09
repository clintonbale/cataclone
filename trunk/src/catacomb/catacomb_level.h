#ifndef _CATACOMB_LEVEL_H
#define _CATACOMB_LEVEL_H

#include "../common.h"

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 64
#define NUMBER_OF_LEVELS 10

typedef struct {
    byte tiles[LEVEL_WIDTH*LEVEL_HEIGHT];
    byte level_number;
} level_t;

void catacomb_level_change(byte level);
void catacomb_level_next();

const level_t* catacomb_level_current(void);

#endif // _CATACOMB_LEVEL_H
