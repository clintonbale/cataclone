#ifndef _CATACOMB_LEVEL_H
#define _CATACOMB_LEVEL_H

#include "../common.h"

#define LEVEL_WIDTH 64
#define LEVEL_HEIGHT 64

typedef struct {
    byte tiles[LEVEL_WIDTH*LEVEL_HEIGHT];
    int  level_number;
} level_t;

//TODO:
//      level_t* level_load(const char* file); //set to private, only exposed is level_change
//      level_t* level_change(byte level); //frees the current level_t* and goes to requested
//      level_t* get_current_level();
//      level_t* next_level() //calls level_change(current_level+1);
//
//      byte current_level;
//

level_t* level_load(const char* file) {
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

void level_free(level_t* level) {
    if(level) {
        free(level);
        level = (level_t*)0;
    }
}

#endif // _CATACOMB_LEVEL_H
