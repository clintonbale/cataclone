#ifndef _CATACOMB_LEVEL_H
#define _CATACOMB_LEVEL_H

#include "../common.h"
#include "../draw.h"

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 64
#define NUMBER_OF_LEVELS 10

#define MAX_TELES 16
#define MAX_MONSTERS 64

//only used if there is no spawn location on the map
#define DEFAULT_SPAWN_X 2
#define DEFAULT_SPAWN_Y 2

typedef struct {
    byte tiles[LEVEL_WIDTH*LEVEL_HEIGHT];
    byte level_number;

    //teleport stuff
    byte num_teles;
    byte tele_anim;
    ushort tele_locations[MAX_TELES];

    byte num_monsters;
    //xxxxyyyy yyyyyyyy
    //xxxx is monster type
    //y... is spawn location
    ushort monster_spawns[MAX_MONSTERS];

    //player stuff
    vec2_t spawn;
} level_t;

void catacomb_level_init();
void catacomb_level_finish();

void catacomb_level_change(byte level);
void catacomb_level_next();

const level_t* catacomb_level_current(void);

void catacomb_level_update(float game_time);
void catacomb_level_render(void);
void catacomb_level_remove_door(ushort x, ushort y);

#endif // _CATACOMB_LEVEL_H
