#ifndef _CATACOMB_SOUND_H_
#define _CATACOMB_SOUND_H_

#include "..\common.h"

#define MAX_SOUND_DEFS 63
#define MAX_SOUND_SIZE 768
#define NO_UNUSED_SOUNDS 1

typedef struct {
    char        signature[4]; //SND
    uint16_t    size;
    uint16_t    unknown;
    uint16_t    count;
    byte        pad[6];
} cat_sound_header;

//Used for reading the file only
typedef struct {
    uint16_t    offset; //offset to sound
    uint8_t     priority; //255 is max, 0 is inadvisable
    uint8_t     flags;  //placeholder = 1, proper sound = 8
    char        name[12]; //null padded name
} cat_sound_def;

//Used for playing, contains PCM data. NOT raw data.
typedef struct {
    byte*       sound_data;
    uint32_t    size;

    uint8_t     priority;
    uint8_t     flags;
    char        name[12];
} cat_sound;

void catacomb_sounds_play(const char* sound_name);
int  catacomb_sounds_find(const char* sound_name);
void catacomb_sounds_load(const char* file_name);

void catacomb_sounds_finish();

#endif //_CATACOMB_SOUND_H_
