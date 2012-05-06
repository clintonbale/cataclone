#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <SDL.h>
#include "render.h"

#define PLAYER_WIDTH (TILE_WIDTH*2)
#define PLAYER_HEIGHT (TILE_HEIGHT*2)

#define BOLT_WIDTH (TILE_WIDTH*2)
#define BOLT_HEIGHT (TILE_HEIGHT*2)

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

typedef enum { BULLET_TYPE_NORMAL, BULLET_TYPE_BOLT } bullet_type_t;
typedef struct {
    vec2_t        position;   //postion in game world.
    dir_t         direction;  //direction moving
    byte          curanim;    //0 or 1 if not exploding
    bullet_type_t type;
    bool          active : 1;     //determines draw and update
} bullet_t;

typedef struct {
    vec2_t  position;
    dir_t   last_dir;
    int     score;
    byte    curanim;
    byte    shotpower;
    byte    health;
    bool    charging;
    bool    strafing;
    bool    last_shot;
    bullet_t bullets[MAX_BULLETS];
    byte    items[4];
    bool    directions[4]; // the keys that the player has pressed
    ushort  todraw[4];     // the four parts of the player to draw
} player_t;

extern player_t player;

void player_init(void);
void player_start(void);

void player_clear_input(void);
void player_event(SDL_Event* event);
void player_update(float frame_time);
void player_draw(void);

bool player_colliding_tiles(byte collisions[4]);
bool player_check_collision();

#endif //_PLAYER_H_
