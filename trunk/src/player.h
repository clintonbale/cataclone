#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <SDL.h>
#include "draw.h"

#define PLAYER_WIDTH (TILE_WIDTH*2)
#define PLAYER_HEIGHT (TILE_HEIGHT*2)

typedef enum {UP = 0,RIGHT = 1,DOWN = 2,LEFT = 3} dir_t;

typedef struct {
    vec2_t  position;
    dir_t   last_dir;
    byte    curanim;
    bool    directions[4]; // the keys that the player has pressed
    ushort  todraw[4];     // the four parts of the player to draw
} player_t;

gltexture_t* player_tiles;
player_t player;

void player_init(void);

void player_event(SDL_Event* event);
void player_update(float frame_time);
void player_draw(void);

void player_colliding_tiles(byte collisions[4]);
bool player_check_collision(void);

#endif //_PLAYER_H_
