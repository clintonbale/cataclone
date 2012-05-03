#include <stdio.h>
#include <string.h>

#include "catacomb_defs.h"
#include "catacomb_level.h"
#include "../render.h"
#include "memory.h"

static level_t* current_level;
static texture_t* tex_level;
static texture_t* tex_tele;

static level_t* catacomb_level_load(const char* file) {
    FILE* fp = NULL;
    level_t* level;

    fp = fopen(file, "rb");
    if(!fp) {
        error("Error opening level: %s", file);
    }

    level = memory_alloc(sizeof(level_t));
    if(!level) {
        error("Out of memory!");
    }

    //zero out the level
    memset(level, 0, sizeof(level_t));
    level->spawn.x = DEFAULT_SPAWN_X;
    level->spawn.y = DEFAULT_SPAWN_Y;

    //TODO: Add support for big endian computers.
    uint32_t desired_size;
    fread(&desired_size, sizeof(uint32_t), 1, fp);
    if(desired_size != (LEVEL_WIDTH*LEVEL_HEIGHT)) {
        error("Incorrect level size, or bad header.");
    }

    byte last_tile = 0;
    uint16_t tile_index = 0;
    uint16_t i;

    //load and decompress the map
    while(tile_index < desired_size) {
        byte val = fgetc(fp);
        //determine the type, if high bit, loop for val-127, else loop for val+3
        uint16_t count = (val&0x80) ? ((val & 0x7F)+1) : (val+3);
        //get the next byte, only if the high bit is NOT set
        byte next = (count==val+3) ? fgetc(fp) : 0;
        //loop for the count, set the tiles
        for(i = 0; i < count; ++i) {
            level->tiles[tile_index] = next ? next : fgetc(fp);

            last_tile = level->tiles[tile_index];
            if(last_tile >= T_A && last_tile <= T_K) {
                switch(last_tile) {
                    case T_TELE:
                        //If it is a tele, add it to the tele list.
                        level->tele_locations[level->num_teles++] = tile_index;
                        break;
                    case T_SPAWN:
                        level->spawn.x = (tile_index) % LEVEL_WIDTH;
                        level->spawn.y = (tile_index) / LEVEL_WIDTH;
                        break;
                    case T_MON_WHITEIMP:
                    case T_MON_BIGREDIMP:
                    case T_MON_BIGPURPIMP:
                    case T_MON_REDIMP:
                    case T_MON_LASTBOSS:
                        //store the location of the tile and the monster type
                        level->monster_spawns[level->num_monsters] = tile_index;
                        level->monster_spawns[level->num_monsters++] |= (last_tile-T_A)<<12;
                        break;
                    default:
                        warn("Unhandled default tile: '%c'", 'A'+(last_tile-T_A));
                        break;
                }
                //replace all A-K tiles with a floor tile.
                level->tiles[tile_index] = T_FLOOR;
            }
            ++tile_index;
        }
    }

    if(ferror(fp)) {
        error("Error reading level: %s", file);
    }

    debug("Loaded level: %s", file);
    return level;
}

static void catacomb_level_free(level_t* level) {
    memory_free(level);
}

void catacomb_level_init(void) {
    tex_level = r_find_texture("MAIN");
    tex_tele = r_find_texture("TELE");
}

void catacomb_level_finish() {
    catacomb_level_free(current_level);
}

void catacomb_level_remove_door(ushort x, ushort y) {
    const byte* tiles = current_level->tiles;
    byte* adjacent[4] = {
        (byte*)&tiles[((y-1)*LEVEL_HEIGHT)+x],  //NORTH
        (byte*)&tiles[((y+1)*LEVEL_WIDTH)+x],   //SOUTH
        (byte*)&tiles[(y*LEVEL_HEIGHT)+x+1],    //EAST
        (byte*)&tiles[(y*LEVEL_WIDTH)+x-1]     //WEST
    };

    if(T_ISDOOR(*adjacent[0])) {
        *adjacent[0] = T_FLOOR;
        catacomb_level_remove_door(x, y-1);
    }
    if(T_ISDOOR(*adjacent[1])) {
        *adjacent[1] = T_FLOOR;
        catacomb_level_remove_door(x, y+1);
    }
    if(T_ISDOOR(*adjacent[2])) {
        *adjacent[2] = T_FLOOR;
        catacomb_level_remove_door(x+1, y);
    }
    if(T_ISDOOR(*adjacent[3])) {
        *adjacent[3] = T_FLOOR;
        catacomb_level_remove_door(x-1, y);
    }
}

const level_t* catacomb_level_current(void) {
    if(current_level) {
        return current_level;
    }
    catacomb_level_change(1);
    if(current_level)
        return current_level;
    error("Could not get current level.");
    return NULL;
}

void catacomb_level_next() {
    byte nextlevel = 1;
    if(current_level) {
        nextlevel = current_level->level_number+1;
        if(nextlevel == 11)
            nextlevel = 1;
        return catacomb_level_change(nextlevel);
    }
    return catacomb_level_change(nextlevel);
}

void catacomb_level_update(float game_time) {
    static float elapsed = 0;
    elapsed += game_time;

    //only update frame every 0.10 seconds
    if(elapsed > 0.10f) {
        //increase the animation counter
        current_level->tele_anim = (current_level->tele_anim + 1) % TELE_ANIM_NUM;
        elapsed = 0;
    }
}

void catacomb_level_render() {
    //draw the map!
    for(int y = -32; y < 96; ++y) {
        for(int x = -32; x < 96; ++x) {
            if(x > 0 && x < LEVEL_WIDTH && y > 0 && y < LEVEL_HEIGHT)
                r_draw_tile(tex_level, current_level->tiles[(y*LEVEL_HEIGHT)+x]<<3, x*TILE_WIDTH, y*TILE_HEIGHT);
            else
                r_draw_tile(tex_level, T_PINK<<3, x*TILE_WIDTH, y*TILE_HEIGHT);
        }
    }

    //draw and animate each tile on the map
    for(byte i = 0; i < current_level->num_teles; ++i) {
        uint x = (current_level->tele_locations[i] % LEVEL_WIDTH)<<3;
        uint y = (current_level->tele_locations[i] / LEVEL_WIDTH)<<3;

        r_draw_tile(tex_tele, current_level->tele_anim*TELE_ANIM_SIZE+0,  x,  y);
        r_draw_tile(tex_tele, current_level->tele_anim*TELE_ANIM_SIZE+8,  x+8,y);
        r_draw_tile(tex_tele, current_level->tele_anim*TELE_ANIM_SIZE+16, x,  y+8);
        r_draw_tile(tex_tele, current_level->tele_anim*TELE_ANIM_SIZE+24, x+8,y+8);
    }
}

void catacomb_level_change(byte num) {
    static char level_str[16] = "LEVEL";
    static char level_num[3] = "";

    if(num < 1 || num > NUMBER_OF_LEVELS)
        error("Invalid level number: %d", num);

    if(current_level && current_level->level_number == num) {
        //already on the level...
        return;
    }

    if(current_level) {
        catacomb_level_free(current_level);
    }

    sprintf(level_num, "%d", num);
    level_str[5] = 0;
    strcat(level_str, level_num);
    strcat(level_str, ".CAT");

    debug("Loading: %s", level_str);
    current_level = catacomb_level_load(level_str);
    current_level->level_number = num;
}
