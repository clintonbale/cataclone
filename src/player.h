#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <SDL.h>
#include "draw.h"

#define PLAYER_WIDTH (TILE_WIDTH*2)
#define PLAYER_HEIGHT (TILE_HEIGHT*2)

// should be calculated if we want the game to be bigger than 320x200
#define PLAYER_CENTER 88

typedef enum {UP = 0,RIGHT = 1,DOWN = 2,LEFT = 3} dir_t;

#define ITEM_KEY    0
#define ITEM_POTION 1
#define ITEM_BOLT   2
#define ITEM_NUKE   3
#define MAX_PLAYER_ITEMS 10

#define MAX_PLAYER_HEALTH 13
#define DEFAULT_PLAYER_HEALTH MAX_PLAYER_HEALTH
#define DEFAULT_PLAYER_BOLTS   3
#define DEFAULT_PLAYER_NUKES   2
#define DEFAULT_PLAYER_POTIONS 3
#define DEFAULT_PLAYER_KEYS    0

#define MAX_SHOT_POWER 0xE

#define MAX_BULLETS 16

#define BULLET_SIZE_SMALL   0
#define BULLET_SIZE_BIG     1

typedef struct {
    vec2_t  position;   //postion in game world.
    dir_t   direction;  //direction moving
    byte    size;       //BULLET_SIZE_SMALL or BULLET_SIZE_BIG
    byte    curanim;    //0 or 1 if not exploding, 0-3 if exploding
    bool    exploding;
    bool    active;     //determines draw and update
} bullet_t;

typedef struct {
    vec2_t  position;
    dir_t   last_dir;
    int     score;
    byte    curanim;
    byte    shotpower;
    byte    health;
    bool    charging;
    bool    last_shot;
    bullet_t bullets[MAX_BULLETS];
    byte    items[4];
    bool    directions[4]; // the keys that the player has pressed
    ushort  todraw[4];     // the four parts of the player to draw
} player_t;

extern player_t player;

void player_init(void);
void player_reset(void);

void player_event(SDL_Event* event);
void player_update(float frame_time);
void player_draw(void);

bool player_colliding_tiles(byte collisions[4]);
bool player_check_collision();

#endif //_PLAYER_H_
