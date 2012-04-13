#include <stdlib.h>
#include <string.h>

#include "catacomb_level.h"
#include "../draw.h"

static level_t* current_level;

static level_t* catacomb_level_load(const char* file) {
    FILE* fp = NULL;
    level_t* level;

    fp = fopen(file, "rb");
    if(!fp) {
        error("Error opening level: %s", file);
    }

    level = malloc(sizeof(level_t));
    if(!level) {
        error("Out of memory!");
    }

    //TODO: Add support for big endian computers.
    uint32_t desired_size;
    fread(&desired_size, sizeof(uint32_t), 1, fp);
    if(desired_size != (LEVEL_WIDTH*LEVEL_HEIGHT)) {
        error("Incorrect level size, or bad header.");
    }

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
            level->tiles[tile_index++] = next ? next : fgetc(fp);
        }
    }
    if(ferror(fp)) {
        error("Error reading level: %s", file);
    }
    debug("Loaded level: %s", file);

    return level;
}

static void catacomb_level_free(level_t* level) {
    if(level) {
        free(level);
        level = (level_t*)0;
    }
}

//returns index that tile was found + 1
static int catacomb_level_find_tile(vec2_t location, uint16_t start, byte tile_id) {
    for(uint16_t i = start; i < (LEVEL_WIDTH*LEVEL_HEIGHT); ++i) {
        if(current_level->tiles[i] == tile_id) {
            location[0] = i % LEVEL_WIDTH; // x
            location[1] = (int)(i / LEVEL_WIDTH); // y
            return i + 1;
        }
    }
    return -1;
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

void catacomb_level_render() {
    static gltexture_t* tile_textures = NULL;
    if(!tile_textures) tile_textures = gl_find_gltexture("MISC");

    //draw the map!
    for(int y = -32; y < 96; ++y) {
        for(int x = -32; x < 96; ++x) {
            if(x > 0 && x < 64 && y > 0 && y < 64)
                gl_draw_tile_spritesheet(tile_textures, catacomb_level_current()->tiles[(y*64)+x]*8, x*8, y*8);
            else
                gl_draw_tile_spritesheet(tile_textures, ('z'+7)*8, x*8, y*8);
        }
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

bool catacomb_level_player_start(vec2_t out_start) {
    if(!current_level || !out_start)
        return false;
    return catacomb_level_find_tile(out_start, 0, 0xE6) > 0;
}
