#include "player.h"
#include "common.h"
#include "graphics.h"
#include "catacomb/catacomb_level.h"
#include "catacomb/catacomb_sound.h"

static uint16_t animations[16]={
    0<<3, 4<<3, //up
    8<<3, //shoot up
    12<<3, //? weird legs
    16<<3, 20<<3, //right
    24<<3, //shoot right
    28<<3, //?
    32<<3, 36<<3, //down
    40<<3, //shoot down
    44<<3, //?
    48<<3, 52<<3, //left
    56<<3, //left shoot
    60<<3, //?
};

void player_init(void) {
    player.position[0] = catacomb_level_current()->spawn[0] * TILE_WIDTH;
    player.position[1] = catacomb_level_current()->spawn[1] * TILE_HEIGHT;

    player_tiles = gl_find_gltexture("PLAYER");
    player.last_dir = RIGHT; //all maps start facing right.
}

void player_event(SDL_Event* event) {
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_UP:    player.directions[UP]   = true; break;
                case SDLK_DOWN:  player.directions[DOWN] = true; break;
                case SDLK_LEFT:  player.directions[LEFT] = true; break;
                case SDLK_RIGHT: player.directions[RIGHT]= true; break;
                default: break;
            }
        break;
        case SDL_KEYUP:
            switch(event->key.keysym.sym) {
                case SDLK_UP:    player.directions[UP]   = false; break;
                case SDLK_DOWN:  player.directions[DOWN] = false; break;
                case SDLK_LEFT:  player.directions[LEFT] = false; break;
                case SDLK_RIGHT: player.directions[RIGHT]= false; break;
                default: break;
            }
        break;
    }
}

//Gets colliding tiles,
void player_colliding_tiles(byte collisions[4]) {
    const byte* level_tiles = (const byte*)&catacomb_level_current()->tiles;
    const int x = player.position[0]/TILE_WIDTH;
    const int y = player.position[1]/TILE_HEIGHT;

    collisions[0] = level_tiles[(y*LEVEL_HEIGHT)+x];      //top left
    collisions[1] = level_tiles[((y+1)*LEVEL_WIDTH)+x];  //bottom left
    collisions[2] = level_tiles[(y*LEVEL_HEIGHT)+x+1];    //top right
    collisions[3] = level_tiles[((y+1)*LEVEL_WIDTH)+x+1];//bottom right
}

bool player_check_collision(void) {
    static byte collisions[4];
    player_colliding_tiles(collisions);

    for(byte i = 0; i < sizeof(collisions); ++i) {
        if(collisions[i] > 128 && collisions[i] < 153)
            return true;
    }

    return false;
}

void player_update(float frame_time) {
    static float elapsed = 0;
    elapsed += frame_time;

    //default to last direction
    dir_t direction = player.last_dir;
    bool move = false;
    for(byte i = 0; i < 4; ++i) {
        //Get the first direction key pressed and set it
        if(player.directions[i]) { direction = i; move = true; break; }
    }
    //only move if time sinse last move is > 0.05;
    if(move && elapsed > 0.05f) {
        elapsed = 0;

        int ox = player.position[0], oy = player.position[1];

        //animate the move
        player.curanim = player.curanim ? 0 : 1;
        //move
        player.position[0] += (direction == LEFT) ? -TILE_WIDTH  : (direction == RIGHT) ? TILE_WIDTH  : 0;
        player.position[1] += (direction == UP)   ? -TILE_HEIGHT : (direction == DOWN)  ? TILE_HEIGHT : 0;
        if(player_check_collision()) {
            //player.position[0] = ox;
            //player.position[1] = oy;
            //catacomb_sounds_play("blocked");
        }

        static char ss[8];
        sprintf(ss, "%d", player.position[0]/8);
        graphics_viewport_set_title(ss);
    }

    player.todraw[0] = animations[(direction<<2)+player.curanim]+0;
    player.todraw[1] = animations[(direction<<2)+player.curanim]+8;
    player.todraw[2] = animations[(direction<<2)+player.curanim]+16;
    player.todraw[3] = animations[(direction<<2)+player.curanim]+24;
    //TODO: Support shooting animations...

    player.last_dir = direction;
}

void player_draw(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(player.position[0]-(graphics_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position[0]+(graphics_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position[1]+(graphics_viewport_height()/2.0f) + PLAYER_HEIGHT,
            player.position[1]-(graphics_viewport_height()/2.0f) + PLAYER_HEIGHT, 0.0f, 1.0f);

    gl_draw_tile_spritesheet(player_tiles, player.todraw[0], player.position[0],             player.position[1]);
    gl_draw_tile_spritesheet(player_tiles, player.todraw[1], player.position[0] + TILE_WIDTH,player.position[1]);
    gl_draw_tile_spritesheet(player_tiles, player.todraw[2], player.position[0],             player.position[1] + TILE_HEIGHT);
    gl_draw_tile_spritesheet(player_tiles, player.todraw[3], player.position[0] + TILE_WIDTH,player.position[1] + TILE_HEIGHT);
}
