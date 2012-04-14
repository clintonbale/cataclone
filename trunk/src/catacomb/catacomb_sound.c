#include "catacomb_sound.h"
#include "..\sound_manager.h"

#include <stdio.h>
#include <math.h>
#include <SDL.h>

//Thanks Anders Gavare
static byte* catacomb_sounds_raw_to_pcm(byte* raw_data, short raw_size, int* pcm_size) {
    static const double dt = 1.0/44100.0;
    static const double freqdiv = 1193180.0;
    static const int short_while = 44100/128;
    const unsigned int new_size = (short_while * 4 * ((raw_size) / 2));
    double t = 0.0;

    byte* pcm = (byte*)malloc(new_size);
    memset(pcm, 0, new_size);
    if(!pcm) {
        error("Out of memory.");
    }
    if(!pcm_size) {
        error("catacomb_sounds_raw_to_pcm: pcm_size is NULL.");
    }
    *pcm_size = new_size;

    unsigned int cur = 0;
    for(unsigned c = 0; c < raw_size; c += 2) {
        if(raw_data[0]!=0xff && raw_data[1]!=0xff) {
            int freq = raw_data[c]+raw_data[c+1]*256;
            for (uint j=0; j < short_while; ++j)
            {
                double y = freq ? 10000.0 * sin(2.0*M_PI*freqdiv/(double)freq*t) : 0.0;

                if (y>1) y=10000;
                if (y<1) y=-10000;

                t += dt;
                int b1 = ((int)y) & 255;
                int b2 = ((int)y) / 256;

                pcm[cur++] = b1;
                pcm[cur++] = b2;
                pcm[cur++] = b1;
                pcm[cur++] = b2;
            }
        }
    }
    if(new_size != cur) {
        warn("Expected PCM size of %d... got %d.", new_size, cur);
    }

    return pcm;
}

void catacomb_sounds_play(const char* sound_name) {
    cat_sound* snd;
    int sndidx = catacomb_sounds_find(sound_name);

    if(sndidx < 0) {
        error("Cannot find sound with name: %s", sound_name);
    }
    snd = &cat_sounds[sndidx];

    if(!sound_manager_play_data(snd->sound_data, snd->size)) {
        warn("Maximum sounds playing, cannot play sound '%s'", sound_name);
    }
}

int catacomb_sounds_find(const char* sound_name) {
    for(int i = 0; i < MAX_SOUND_DEFS; i++) {
        if(cat_sounds[i].name &&
           !strcmp(cat_sounds[i].name, sound_name))
           return i;
    }
    return -1;
}

void catacomb_sounds_load(const char* file_name) {
    FILE* fp = NULL;
    byte raw_sound_data[MAX_SOUND_SIZE];
    cat_sound_header header;
    cat_sound_def defs[MAX_SOUND_DEFS];
    ushort i, sound_size;
    uint loaded_size = 0;

    memset(&header, 0, sizeof(cat_sound_header));
    memset(&defs, 0, sizeof(cat_sound_def)*MAX_SOUND_DEFS);

    if(!file_name)
        error("catacomb_load_sound: NULL file_name");

    fp = fopen(file_name, "rb");
    if(!fp)
        error("catacomb_load_sound: Cannot open file '%s'", file_name);

    if(fread(&header, 1, sizeof(cat_sound_header), fp) != sizeof(cat_sound_header))
        error("catacomb_load_sound: Error reading SND header.");

    //Only support statically sized files
    if(header.count > 0) {
        error("catacomb_load_sound: Unsupported file type.");
    }

    //TODO: Add support for BIG-ENDIAN
    for(i = 0; i < MAX_SOUND_DEFS; i++) {
        if(fread(&defs[i], 1, sizeof(cat_sound_def), fp) != sizeof(cat_sound_def))
            error("catacomb_load_sound: Error reading SND definition %d", i);
    }

    //Safely remove and clean all existing sounds...
    catacomb_sounds_finish();

    for(i = 0; i < MAX_SOUND_DEFS; i++) {
        sound_size = defs[i+1].offset - defs[i].offset;

        //Don't load unused sounds!
        if(NO_UNUSED_SOUNDS && !strcmp("->UNUSED<-", defs[i].name))
            continue;

        cat_sounds[i].priority = defs[i].priority;
        cat_sounds[i].flags = defs[i].flags;
        strcpy(cat_sounds[i].name, defs[i].name);

        //Seek to the data offset
        if(fseek(fp, defs[i].offset, SEEK_SET)) {
            error("catacomb_sounds_load: Error seeking to sound '%s' at offset %02X.", defs[i].name, defs[i].offset);
        }
        //Read the data.
        if(fread(&raw_sound_data, 1, sound_size, fp) != sound_size) {
            error("catacomb_sounds_load: Error loading sound '%s'", defs[i].name);
        }

        cat_sounds[i].sound_data = catacomb_sounds_raw_to_pcm((byte*)&raw_sound_data, sound_size, (int*)&cat_sounds[i].size);

        debug("Sound '%s' loaded...", cat_sounds[i].name);
        loaded_size += cat_sounds[i].size;
    }
    debug("Loaded %.4f MB of sounds...", loaded_size/1024.0/1024.0);

    fclose(fp);
}

void catacomb_sounds_finish(void) {
    for(int i = 0; i < MAX_SOUND_DEFS; i++) {
        if(cat_sounds[i].sound_data) {
            free(cat_sounds[i].sound_data);
        }
    }
    //empty the arr
    memset(&cat_sounds[0], 0, sizeof(cat_sound)*MAX_SOUND_DEFS);
}
