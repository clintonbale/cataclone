#include "player.h"
#include "common.h"
#include "window.h"
#include "catacomb/catacomb_level.h"
#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_defs.h"
#include <SDL_opengl.h>

//
// Ignore the warning for clearing the player array.
//
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

player_t player;

static texture_t* tex_player;
static texture_t* tex_bolt;
static texture_t* tex_main;

static const ushort player_anims[16]={
    0<<3, 4<<3,     //UP
    8<<3,           //SHOOT UP
    12<<3,          //DANCE UP
    16<<3, 20<<3,   //RIGHT
    24<<3,          //SHOOT RIGHT
    28<<3,          //DANCE RIGHT
    32<<3, 36<<3,   //DOWN
    40<<3,          //SHOOT DOWN
    44<<3,          //DANCE DOWN
    48<<3, 52<<3,   //LEFT
    56<<3,          //SHOOT LEFT
    60<<3,          //DANCE LEFT
};

static const ushort bullet_anims[14]={
    154<<3, 155<<3,      //UP
    156<<3, 157<<3,      //RIGHT
    158<<3, 159<<3,      //DOWN
    160<<3, 161<<3,      //LEFT
    26<<3, 27<<3, 28<<3, //NORMAL WALL EXPLODE
    29<<3, 30<<3, 31<<3, //HIDDEN WALL EXPLODE
};

static const ushort bolt_anims[8] = {
    0<<4, 4<<3,     //UP
    8<<3, 12<<3,    //RIGHT
    16<<3, 20<<3,   //DOWN
    24<<3, 28<<3    //LEFT
};


/*
===============
player_start

Sets up a player for the level.
===============
*/
void player_start(void) {
    player.position.x = catacomb_level_current()->spawn.x * TILE_WIDTH;
    player.position.y = catacomb_level_current()->spawn.y * TILE_HEIGHT;

    player.last_dir = RIGHT; //all maps start facing right.
    player.todraw[0] = player_anims[(player.last_dir<<2)+player.curanim]+0;
    player.todraw[1] = player_anims[(player.last_dir<<2)+player.curanim]+8;
    player.todraw[2] = player_anims[(player.last_dir<<2)+player.curanim]+16;
    player.todraw[3] = player_anims[(player.last_dir<<2)+player.curanim]+24;

    memset(player.bullets, 0, sizeof(player.bullets));
}


/*
===============
player_init

Loads textures needed and resets all players items/health.
===============
*/
void player_init(void) {
    player_start();

    player.items[ITEM_POTION] = DEFAULT_PLAYER_POTIONS;
    player.items[ITEM_KEY] = DEFAULT_PLAYER_KEYS;
    player.items[ITEM_BOLT] = DEFAULT_PLAYER_BOLTS;
    player.items[ITEM_NUKE] = DEFAULT_PLAYER_NUKES;

    player.health = DEFAULT_PLAYER_HEALTH;

    tex_player = r_find_texture("PLAYER");
    tex_bolt = r_find_texture("BOLT");
    tex_main = r_find_texture("MAIN");
}


/*
===============
player_clear_input
===============
*/
void player_clear_input(void) {
    //quickly set all 4 (chars) in the array to 0.
    *(int32_t*)&player.directions = 0;
    player.charging = player.strafing = false;
}


/*
===============
player_event

Hanles player input
===============
*/
void player_event(SDL_Event* event) {
    switch(event->type) {
        case SDL_KEYDOWN:
            switch(event->key.keysym.sym) {
                case SDLK_UP:    player.directions[UP]   = true; break;
                case SDLK_DOWN:  player.directions[DOWN] = true; break;
                case SDLK_LEFT:  player.directions[LEFT] = true; break;
                case SDLK_RIGHT: player.directions[RIGHT]= true; break;
                case SDLK_LCTRL: player.charging = true; break;
                case SDLK_LALT:  player.strafing = true; break;
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
                case SDLK_LALT:  player.strafing = false; break;
                default: break;
            }
        break;
        default: break;
    }
}


/*
===============
player_colliding_tiles

Gets the four colliding tiles for the player.
Returns whether or not the player is out of bounds.
===============
*/
bool player_colliding_tiles(byte collisions[4]) {
    const byte* level_tiles = (const byte*)&catacomb_level_current()->tiles;
    const int x = player.position.x/TILE_WIDTH;
    const int y = player.position.y/TILE_HEIGHT;

    collisions[0] = level_tiles[(y*LEVEL_HEIGHT)+x];      //top left
    collisions[1] = level_tiles[((y+1)*LEVEL_WIDTH)+x];  //bottom left
    collisions[2] = level_tiles[(y*LEVEL_HEIGHT)+x+1];    //top right
    collisions[3] = level_tiles[((y+1)*LEVEL_WIDTH)+x+1];//bottom right

    return (y < 0 || x < 0 || y > LEVEL_HEIGHT || x > LEVEL_WIDTH);
}


/*
===============
player_check_collision

Checks if the player is colliding with anything.
===============
*/
bool player_check_collision(void) {
    static byte collisions[4];
    if(player_colliding_tiles(collisions))
        return true;

    for(byte i = 0; i < sizeof(collisions); ++i) {
        if(T_ISWALL(collisions[i])) {
            catacomb_sounds_play("blocked");
            return true;
        }
        //door that requires key
        else if(collisions[i] == T_DOOR_V || collisions[i] == T_DOOR_H) {
            if(player.items[ITEM_KEY] > 0) {
                player.items[ITEM_KEY]--;
                catacomb_level_remove_door(player.position.x/TILE_WIDTH, player.position.y/TILE_HEIGHT);
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


/*
===============
player_check_items

Checks if the player is on any items, and adds them to inventory.
===============
*/
void player_check_items(void) {
    const byte* level_tiles = (const byte*)&catacomb_level_current()->tiles;
    const int x = player.position.x/TILE_WIDTH;
    const int y = player.position.y/TILE_HEIGHT;
    byte* tiles[4] = {
        (byte*)&level_tiles[(y*LEVEL_HEIGHT)+x],
        (byte*)&level_tiles[((y+1)*LEVEL_WIDTH)+x],
        (byte*)&level_tiles[(y*LEVEL_HEIGHT)+x+1],
        (byte*)&level_tiles[((y+1)*LEVEL_WIDTH)+x+1]
    };
    for(byte i = 0; i < 4; ++i) {
        switch(*tiles[i]) {
            case T_KEY:
                player.items[ITEM_KEY]++;
                *tiles[i] = T_FLOOR;

                catacomb_sounds_play("item");
                break;
            case T_POTION:
                player.items[ITEM_POTION]++;
                *tiles[i] = T_FLOOR;

                catacomb_sounds_play("item");
                break;
            case T_SCROLL:
                player.items[ITEM_BOLT]++;
                *tiles[i] = T_FLOOR;

                catacomb_sounds_play("item");
                break;
            case T_TREASURE:
                player.score += 500;
                *tiles[i] = T_FLOOR;

                catacomb_sounds_play("treasure");
                break;
            default: break;
        }
    }
}


/*
===============
player_next_bullet

Gets the first non active bullet. NULL if no bullet.
===============
*/
static bullet_t* player_next_bullet(void) {
    for(byte i = 0; i < MAX_BULLETS; i++)
        if(!player.bullets[i].active)
            return &player.bullets[i];
    return NULL;
}


/*
===============
player_update
===============
*/
void player_update(float frame_time) {
    static float charge_timer = 0.0f;
    static float move_timer = 0.0f;
    static float fire_anim_timer = 0.0f;
    static float bullet_timer = 0.0f;
    move_timer += frame_time;
    charge_timer += frame_time;
    fire_anim_timer -= frame_time;
    bullet_timer += frame_time;

    //
    // Default to last direction
    //
    dir_t direction = player.last_dir;
    bool move = false;
    for(byte i = 0; i < 4; ++i) {
        //Get the first direction key pressed and set it
        if(player.directions[i]) { direction = i; move = true; break; }
    }
    //
    // Only move if time sinse last move is > 0.05;
    //
    if(move && move_timer > 0.05f) {
        move_timer = 0.0f;
        ushort ox = player.position.x, oy = player.position.y;

        //
        // Animate the move (animates even if we dont move!)
        //
        player.curanim = player.curanim ? 0 : 1;

        player.position.x += (direction == LEFT) ? -TILE_WIDTH  : (direction == RIGHT) ? TILE_WIDTH  : 0;
        player.position.y += (direction == UP)   ? -TILE_HEIGHT : (direction == DOWN)  ? TILE_HEIGHT : 0;
        if(player_check_collision()) {
           //
           // Collsion, reset position back to the old position.
           //
           player.position.x = ox;
           player.position.y = oy;
        }
        else {
            player_check_items();
            //TODO: Monster check?
        }

        //DEBUG STUFF
        static byte colliding_tiles[4];
        player_colliding_tiles(colliding_tiles);
        static char ss[64];
        sprintf(ss, "pos: %d, 0: %d, 1: %d, 2: %d, 3: %d", player.position.x/8,
                colliding_tiles[0], colliding_tiles[1], colliding_tiles[2], colliding_tiles[3]);
        window_set_title(ss);
    }
    //
    // If we're strafing, put the direction back to what it was
    // this will allow us to strafe/shoot properly on draw/shoot calls.
    //
    if(player.strafing)
        direction = player.last_dir;


    if(bullet_timer > 0.025f) {
        bullet_timer = 0.0f;
        byte* bullet_tile;
        bullet_t* bullet;
        for(byte i = 0; i < MAX_BULLETS; ++i) {
            if(player.bullets[i].active) {
                bullet = &player.bullets[i];
                bullet_tile = (byte*)&catacomb_level_current()->tiles[((bullet->position.y/TILE_HEIGHT)*LEVEL_WIDTH)+(bullet->position.x/TILE_WIDTH)];

                //TODO: Add support for colliding BOLTS into walls and hidden walls.
                if(!bullet->exploding) {
                    if(T_ISWALL(*bullet_tile)) {
                        bullet->exploding = true;
                        //
                        // Make it a small bullet so we can switch
                        // animations and draw normal bullets.
                        //
                        bullet->type = BULLET_TYPE_NORMAL;
                        bullet->curanim = T_ISHIDDEN(*bullet_tile) ? 11 : 8;
                    }
                    else if(T_ISDOOR(*bullet_tile)) {
                        //
                        // No explosions for doors, just remove it
                        //
                        bullet->active = false;
                    }
                    else {
                        if(bullet->type == BULLET_TYPE_NORMAL) {
                            bullet->curanim = !bullet->curanim;
                        }
                        else {
                            bullet->curanim += (bullet->curanim&1) ? -1 : 1;
                        }
                        bullet->position.x += (bullet->direction == LEFT ? -TILE_WIDTH : bullet->direction == RIGHT ? TILE_WIDTH : 0);
                        bullet->position.y += (bullet->direction == UP ? -TILE_HEIGHT : bullet->direction == DOWN ? TILE_HEIGHT : 0);
                    }
                }
                else {
                    bullet->curanim++;
                    if(T_ISHIDDEN(*bullet_tile) && bullet->curanim == 13) {
                        bullet->exploding = bullet->active = false;
                        *bullet_tile = T_FLOOR;
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

        bullet_t* new_bullet = player_next_bullet();
        if(new_bullet) {
            new_bullet->active = true;
            new_bullet->type = player.shotpower == MAX_SHOT_POWER ? BULLET_TYPE_BOLT : BULLET_TYPE_NORMAL;
            //
            // Only switch firing directions when we're firing a normal projectile.
            //
            if(new_bullet->type == BULLET_TYPE_NORMAL)
                player.last_shot = !player.last_shot;
            new_bullet->direction = direction;
            switch(direction) {
                case UP:
                    //ensure only the normal types of bullets switch sides.
                    new_bullet->position.x = player.position.x + ((new_bullet->type)?0:player.last_shot*TILE_WIDTH);
                    new_bullet->position.y = player.position.y - (new_bullet->type+1)*TILE_HEIGHT;
                    break;
                case DOWN:
                    new_bullet->position.x = player.position.x + ((new_bullet->type)?0:player.last_shot*TILE_WIDTH);
                    new_bullet->position.y = player.position.y + (new_bullet->type+1)*TILE_HEIGHT;
                    break;
                case LEFT:
                    new_bullet->position.x = player.position.x - (new_bullet->type+1)*TILE_WIDTH;
                    new_bullet->position.y = player.position.y + ((new_bullet->type)?0:player.last_shot*TILE_HEIGHT);
                    break;
                case RIGHT:
                    new_bullet->position.x = player.position.x + (new_bullet->type+1)*TILE_WIDTH;
                    new_bullet->position.y = player.position.y + ((new_bullet->type)?0:player.last_shot*TILE_HEIGHT);
                    break;
                default:
                    warn("Invalid direction: %d", direction);
                    break;
            }
            //TODO: add delay between shots...
            new_bullet->curanim = 0;
        }
        else {
            warn("All bullets are active. Cannot create bullet.");
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


/*
===============
player_draw
===============
*/
void player_draw(void) {
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(player.position.x+PLAYER_CENTER-(window_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position.x+PLAYER_CENTER+(window_viewport_width()/2.0f) - PLAYER_WIDTH,
            player.position.y+(window_viewport_height()/2.0f) + PLAYER_HEIGHT - (TILE_HEIGHT/2),
            player.position.y-(window_viewport_height()/2.0f) + PLAYER_HEIGHT - (TILE_HEIGHT/2), 0.0f, 1.0f);

    bullet_t* bullet;
    for(byte i = 0; i < MAX_BULLETS; ++i) {
        if(player.bullets[i].active) {
            bullet = &player.bullets[i];
            if(bullet->type == BULLET_TYPE_BOLT) {
                r_draw_tile(tex_bolt, bolt_anims[(bullet->direction<<1)+bullet->curanim]+0, bullet->position.x,             bullet->position.y);
                r_draw_tile(tex_bolt, bolt_anims[(bullet->direction<<1)+bullet->curanim]+8, bullet->position.x + TILE_WIDTH,bullet->position.y);
                r_draw_tile(tex_bolt, bolt_anims[(bullet->direction<<1)+bullet->curanim]+16,bullet->position.x,             bullet->position.y + TILE_HEIGHT);
                r_draw_tile(tex_bolt, bolt_anims[(bullet->direction<<1)+bullet->curanim]+24,bullet->position.x + TILE_WIDTH,bullet->position.y + TILE_HEIGHT);
            }
            else {
                r_draw_tile(tex_main,
                                         bullet_anims[(bullet->exploding) ? bullet->curanim : ((bullet->direction*2)+bullet->curanim)],
                                         bullet->position.x, bullet->position.y);
            }
        }
    }

    r_draw_tile(tex_player, player.todraw[0], player.position.x,             player.position.y);
    r_draw_tile(tex_player, player.todraw[1], player.position.x + TILE_WIDTH,player.position.y);
    r_draw_tile(tex_player, player.todraw[2], player.position.x,             player.position.y + TILE_HEIGHT);
    r_draw_tile(tex_player, player.todraw[3], player.position.x + TILE_WIDTH,player.position.y + TILE_HEIGHT);
}
