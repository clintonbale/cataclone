#include <stdlib.h>
#include <string.h>

#include "catacomb_level.h"

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

    itoa(num, &level_num, 10);
    level_str[5] = 0;
    strcat(level_str, level_num);
    strcat(level_str, ".CAT");

    debug("Loading: %s", level_str);
    current_level = catacomb_level_load(level_str);
    current_level->level_number = num;
}
