#include "player.h"
#include "common.h"
#include "catacomb/catacomb_level.h"

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
    catacomb_level_player_start(player.position);
    player.position[0] *= TILE_WIDTH;
    player.position[1] *= TILE_HEIGHT;

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

void player_update() {
    //default to last direction
    dir_t direction = player.last_dir;
    static char ss[8];
    bool move = false;
    for(byte i = 0; i < 4; ++i) {
        //Get the first direction key pressed and set it
        if(player.directions[i]) { direction = i; move = true; break; }
    }
    if(move) {
        //animate the move
        player.curanim = player.curanim ? 0 : 1;
        //move
        player.position[0] += (direction == LEFT) ? -TILE_WIDTH  : (direction == RIGHT) ? TILE_WIDTH  : 0;
        player.position[1] += (direction == UP)   ? -TILE_HEIGHT : (direction == DOWN)  ? TILE_HEIGHT : 0;


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
