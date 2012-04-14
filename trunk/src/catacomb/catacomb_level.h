#ifndef _CATACOMB_LEVEL_H
#define _CATACOMB_LEVEL_H

#include "../common.h"
#include "../draw.h"

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 64
#define NUMBER_OF_LEVELS 10

#define MAX_TELES 16

typedef struct {
    byte tiles[LEVEL_WIDTH*LEVEL_HEIGHT];
    byte level_number;

    //teleport stuff
    byte num_teles;
    byte tele_anim;
    ushort tele_locations[MAX_TELES];
} level_t;

gltexture_t* level_textures;
gltexture_t* tele_texture;

void catacomb_level_init();

//Main level changing functions
void catacomb_level_change(byte level);
void catacomb_level_next();

const level_t* catacomb_level_current(void);
//-----------------------------

//Level gameplay functions
bool catacomb_level_player_start(vec2_t out_start);
void catacomb_level_render(void);

//-----------------------------

#endif // _CATACOMB_LEVEL_H
