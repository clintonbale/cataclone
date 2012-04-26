#include "player.h"
#include "common.h"
#include "graphics.h"
#include "catacomb/catacomb_level.h"
#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_tiles.h"

gltexture_t* tex_player;
gltexture_t* tex_bolt;
gltexture_t* tex_main;
player_t player;

static uint16_t player_anims[16]={
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

static uint16_t bullet_anims[14]={
    154<<3, 155<<3, //UP
    156<<3, 157<<3, //RIGHT
    158<<3, 159<<3, //DOWN
    160<<3, 161<<3, //LEFT
    26<<3, 27<<3, 28<<3, //NORMAL WALL EXPLODE
    29<<3, 30<<3, 31<<3, //HIDDEN WALL EXPLODE
};

void player_init(void) {
    player.position[0] = catacomb_level_current()->spawn[0] * TILE_WIDTH;
    player.position[1] = catacomb_level_current()->spawn[1] * TILE_HEIGHT;

    player.last_dir = RIGHT; //all maps start facing right.
    player.todraw[0] = player_anims[(player.last_dir<<2)+player.curanim]+0;
    player.todraw[1] = player_anims[(player.last_dir<<2)+player.curanim]+8;
    player.todraw[2] = player_anims[(player.last_dir<<2)+player.curanim]+16;
    player.todraw[3] = player_anims[(player.last_dir<<2)+player.curanim]+24;

    memset(player.bullets, 0, sizeof(player.bullets));
}

void player_reset(void) {
    player_init();

    player.items[ITEM_POTION] = DEFAULT_PLAYER_POTIONS;
    player.items[ITEM_KEY] = DEFAULT_PLAYER_KEYS;
    player.items[ITEM_BOLT] = DEFAULT_PLAYER_BOLTS;
    player.items[ITEM_NUKE] = DEFAULT_PLAYER_NUKES;

    player.health = DEFAULT_PLAYER_HEALTH;

    tex_player = gl_find_gltexture("PLAYER");
    tex_bolt = gl_find_gltexture("BOLT");
    tex_main = gl_find_gltexture("MAIN");
}

void player_event(SDL_Event* event) {
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_UP:    player.directions[UP]   = true; break;
                case SDLK_DOWN:  player.directions[DOWN] = true; break;
                case SDLK_LEFT:  player.directions[LEFT] = true; break;
                case SDLK_RIGHT: player.directions[RIGHT]= true; break;
                case SDLK_LCTRL: player.charging = true; break;
                default: break;
            }
        break;
        case SDL_KEYUP:
            switch(event->key.keysym.sym) {
                case SDLK_UP:    player.directions[UP]   = false; break;
                case SDLK_DOWN:  player.directions[DOWN] = false; break;
                case SDLK_LEFT:  player.directions[LEFT] = false; break;
                case SDLK_RIGHT: player.directions[RIGHT]= false; break;
                case SDLK_LCTRL: player.charging = false; break;
                default: break;
            }
        break;
    }
}

//Gets colliding tiles,
bool player_colliding_tiles(byte collisions[4]) {
    const byte* level_tiles = (const byte*)&catacomb_level_current()->tiles;
    const int x = player.position[0]/TILE_WIDTH;
    const int y = player.position[1]/TILE_HEIGHT;

    collisions[0] = level_tiles[(y*LEVEL_HEIGHT)+x];      //top left
    collisions[1] = level_tiles[((y+1)*LEVEL_WIDTH)+x];  //bottom left
    collisions[2] = level_tiles[(y*LEVEL_HEIGHT)+x+1];    //top right
    collisions[3] = level_tiles[((y+1)*LEVEL_WIDTH)+x+1];//bottom right

    return (y < 0 || x < 0 || y > LEVEL_HEIGHT || x > LEVEL_WIDTH);
}

bool player_check_collision(void) {
    static byte collisions[4];
    if(player_colliding_tiles(collisions))
        return true;

    for(byte i = 0; i < sizeof(collisions); ++i) {
        if(IS_TILE_WALL(collisions[i])) {
            catacomb_sounds_play("blocked");
            return true;
        }
        //door that requires key
        else if(collisions[i] == TILE_TYPE_V_DOOR || collisions[i] == TILE_TYPE_H_DOOR) {
            if(player.items[ITEM_KEY] > 0) {
                player.items[ITEM_KEY]--;
                catacomb_level_remove_door(player.position[0]/TILE_WIDTH, player.position[1]/TILE_HEIGHT);
                catacomb_sounds_play("opendoor");
            }
            else {
                catacomb_sounds_play("noitem");
            }
            return true;
        }
    }

    return false;
}

void player_check_items(void) {
    const byte* level_tiles = (const byte*)&catacomb_level_current()->tiles;
    const int x = player.position[0]/TILE_WIDTH;
    const int y = player.position[1]/TILE_HEIGHT;
    byte* tiles[4] = {
        (byte*)&level_tiles[(y*LEVEL_HEIGHT)+x],
        (byte*)&level_tiles[((y+1)*LEVEL_WIDTH)+x],
        (byte*)&level_tiles[(y*LEVEL_HEIGHT)+x+1],
        (byte*)&level_tiles[((y+1)*LEVEL_WIDTH)+x+1]
    };
    for(byte i = 0; i < 4; ++i) {
        switch(*tiles[i]) {
            case TILE_ITEM_KEY:
                player.items[ITEM_KEY]++;
                *tiles[i] = TILE_TYPE_FLOOR;

                catacomb_sounds_play("item");
                break;
            case TILE_ITEM_POTION:
                player.items[ITEM_POTION]++;
                *tiles[i] = TILE_TYPE_FLOOR;

                catacomb_sounds_play("item");
                break;
            case TILE_ITEM_SCROLL:
                player.items[ITEM_BOLT]++;
                *tiles[i] = TILE_TYPE_FLOOR;

                catacomb_sounds_play("item");
                break;
            case TILE_ITEM_TREASURE:
                player.score += 500;
                *tiles[i] = TILE_TYPE_FLOOR;

                catacomb_sounds_play("treasure");
                break;
        }
    }
}

bullet_t* next_bullet(player_t* p) {
    if(p) {
        for(byte i = 0; i < MAX_BULLETS; i++)
            if(!p->bullets[i].active)
                return &p->bullets[i];
    }
    return NULL;
}

void player_update(float frame_time) {
    static float charge_timer = 0.0f;
    static float move_timer = 0.0f;
    static float fire_anim_timer = 0.0f;
    static float bullet_timer = 0.0f;
    move_timer += frame_time;
    charge_timer += frame_time;
    fire_anim_timer -= frame_time;
    bullet_timer += frame_time;

    //default to last direction
    dir_t direction = player.last_dir;
    bool move = false;
    for(byte i = 0; i < 4; ++i) {
        //Get the first direction key pressed and set it
        if(player.directions[i]) { direction = i; move = true; break; }
    }
    //only move if time sinse last move is > 0.05;
    if(move && move_timer > 0.05f) {
        move_timer = 0.0f;
        ushort ox = player.position[0], oy = player.position[1];

        //animate the move
        player.curanim = player.curanim ? 0 : 1;
        //move
        player.position[0] += (direction == LEFT) ? -TILE_WIDTH  : (direction == RIGHT) ? TILE_WIDTH  : 0;
        player.position[1] += (direction == UP)   ? -TILE_HEIGHT : (direction == DOWN)  ? TILE_HEIGHT : 0;
        if(player_check_collision()) {
            player.position[0] = ox;
            player.position[1] = oy;
        }
        else player_check_items();

        //DEBUG STUFF
        static byte colliding_tiles[4];
        player_colliding_tiles(colliding_tiles);
        static char ss[32];
        sprintf(ss, "pos: %d, 0: %d, 1: %d, 2: %d, 3: %d", player.position[0]/8,
                colliding_tiles[0], colliding_tiles[1], colliding_tiles[2], colliding_tiles[3]);
        graphics_viewport_set_title(ss);
    }

    if(bullet_timer > 0.025f) {
        bullet_timer = 0.0f;
        byte* bullet_tile;
        bullet_t* bullet;
        for(byte i = 0; i < MAX_BULLETS; ++i) {
            if(player.bullets[i].active) {
                bullet = &player.bullets[i];
                bullet_tile = (byte*)&catacomb_level_current()->tiles[((bullet->position[1]/TILE_HEIGHT)*LEVEL_WIDTH)+(bullet->position[0]/TILE_WIDTH)];

                if(!bullet->exploding) {
                    if(IS_TILE_WALL(*bullet_tile)) {
                        bullet->exploding = true;
                        bullet->curanim = IS_TILE_HIDDEN(*bullet_tile) ? 11 : 8;
                    }
                    else if(ISDOOR(*bullet_tile)) {
                        //no explosions for doors
                        bullet->active = false;
                    }
                    else {
                        bullet->curanim = !bullet->curanim;
                        bullet->position[0] += (bullet->direction == LEFT ? -TILE_WIDTH : bullet->direction == RIGHT ? TILE_WIDTH : 0);
                        bullet->position[1] += (bullet->direction == UP ? -TILE_HEIGHT : bullet->direction == DOWN ? TILE_HEIGHT : 0);
                    }
                }
                else {
                    bullet->curanim++;
                    if(IS_TILE_HIDDEN(*bullet_tile) && bullet->curanim == 13) {
                        bullet->exploding = bullet->active = false;
                        *bullet_tile = TILE_TYPE_FLOOR;
                    }
                    else if(bullet->curanim == 10)
                        bullet->exploding = bullet->active = false;
                }
            }
        }
    }


    if(player.charging && player.shotpower < MAX_SHOT_POWER && charge_timer > 0.05) {
        charge_timer = 0.0f;
        ++player.shotpower;
    }
    else if(!player.charging && player.shotpower > 0) {
        player.todraw[0] = player_anims[(direction<<2)+2]+0;
        player.todraw[1] = player_anims[(direction<<2)+2]+8;
        player.todraw[2] = player_anims[(direction<<2)+2]+16;
        player.todraw[3] = player_anims[(direction<<2)+2]+24;
        fire_anim_timer = 0.15f;

        bullet_t* new_bullet = next_bullet(&player);
        if(new_bullet) {
            new_bullet->active = true;
            new_bullet->size = player.shotpower == MAX_SHOT_POWER ? BULLET_SIZE_BIG : BULLET_SIZE_SMALL;
            new_bullet->direction = direction;
            switch(direction) {
                case UP:
                    new_bullet->position[0] = player.position[0] + player.last_shot*TILE_WIDTH;
                    new_bullet->position[1] = player.position[1] - (new_bullet->size+1)*TILE_HEIGHT;
                    break;
                case DOWN:
                    new_bullet->position[0] = player.position[0] + player.last_shot*TILE_WIDTH;
                    new_bullet->position[1] = player.position[1] + (new_bullet->size+1)*TILE_HEIGHT;
                    break;
                case LEFT:
                    new_bullet->position[0] = player.position[0] - (new_bullet->size+1)*TILE_WIDTH;
                    new_bullet->position[1] = player.position[1] + player.last_shot*TILE_HEIGHT;
                    break;
                case RIGHT:
                    new_bullet->position[0] = player.position[0] + (new_bullet->size+1)*TILE_WIDTH;
                    new_bullet->position[1] = player.position[1] + player.last_shot*TILE_HEIGHT;
                    break;
            }
            //TODO: add delay between shots...
            new_bullet->curanim = 0;
            player.last_shot = !player.last_shot;
        }
        player.shotpower = 0;
    }
    else if(fire_anim_timer < 0.0f) {
        player.todraw[0] = player_anims[(direction<<2)+player.curanim]+0;
        player.todraw[1] = player_anims[(direction<<2)+player.curanim]+8;
        player.todraw[2] = player_anims[(direction<<2)+player.curanim]+16;
        player.todraw[3] = player_anims[(direction<<2)+player.curanim]+24;
    }
    player.last_dir = direction;
}

void player_draw(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(player.position[0]+PLAYER_CENTER-(graphics_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position[0]+PLAYER_CENTER+(graphics_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position[1]+(graphics_viewport_height()/2.0f) + PLAYER_HEIGHT - (TILE_HEIGHT/2),
            player.position[1]-(graphics_viewport_height()/2.0f) + PLAYER_HEIGHT - (TILE_HEIGHT/2), 0.0f, 1.0f);

    bullet_t* bullet;
    for(byte i = 0; i < MAX_BULLETS; ++i) {
        if(player.bullets[i].active) {
            bullet = &player.bullets[i];

            //TODO: Support big ass bullets!
            if(bullet->size == BULLET_SIZE_BIG) {
                warn("Big bullets unsupported...");
                bullet->active = false;
            }
            else {
                gl_draw_tile_spritesheet(tex_main,
                                         bullet_anims[(bullet->exploding) ? bullet->curanim : ((bullet->direction*2)+bullet->curanim)],
                                         bullet->position[0], bullet->position[1]);
            }
        }
    }

    gl_draw_tile_spritesheet(tex_player, player.todraw[0], player.position[0],             player.position[1]);
    gl_draw_tile_spritesheet(tex_player, player.todraw[1], player.position[0] + TILE_WIDTH,player.position[1]);
    gl_draw_tile_spritesheet(tex_player, player.todraw[2], player.position[0],             player.position[1] + TILE_HEIGHT);
    gl_draw_tile_spritesheet(tex_player, player.todraw[3], player.position[0] + TILE_WIDTH,player.position[1] + TILE_HEIGHT);
}
