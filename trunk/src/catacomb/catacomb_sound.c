#include "catacomb_sound.h"
#include "..\sound_manager.h"

#include <math.h>
#include <SDL.h>

static cat_sound cat_sounds[MAX_SOUND_DEFS];

//Thanks Anders Gavare
static byte* catacomb_sounds_raw_to_pcm(byte* raw_data, ushort raw_size, int* pcm_size) {
    static const double dt = 1.0/22050.0;
    static const double freqdiv = 1192030.0;
    static const uint short_while = 22050/128;
    const uint new_size = (short_while * 4 * ((raw_size) / 2));
    double t = 0.0;

    //empty sound, just return a NULL ptr.
    if(raw_data[0]==0xff && raw_data[1]==0xff)
        return NULL;

    byte* pcm = (byte*)memory_calloc(new_size);
    if(!pcm_size) {
        error("catacomb_sounds_raw_to_pcm: pcm_size is NULL.");
    }
    *pcm_size = new_size;

    uint cur = 0;
    for(ushort c = 0; c < raw_size; c += 2) {
        uint freq = raw_data[c]+raw_data[c+1]*256;
        for (uint j = 0; j < short_while; ++j)
        {
            double y = freq ? 8000 * sin(2.0*M_PI*freqdiv/(double)freq*t) : 0;

            if (y < 1.0) y = -8000;

            t += dt;

            int b1 = ((int)y) & 255;
            int b2 = ((int)y) / 256;

            pcm[cur++] = b1;
            pcm[cur++] = b2;
            pcm[cur++] = b1;
            pcm[cur++] = b2;
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
    uint loaded_size = 0;

    cat_sound_header header = {0};
    cat_sound_def defs[MAX_SOUND_DEFS] = {0};
    byte raw_sound_data[MAX_SOUND_SIZE];

    if(!file_name)
        error("catacomb_load_sound: NULL file_name");

    FILE* fp = fopen(file_name, "rb");
    if(!fp)
        error("catacomb_load_sound: Cannot open file '%s'", file_name);

    if(fread(&header, 1, sizeof(cat_sound_header), fp) != sizeof(cat_sound_header))
        error("catacomb_load_sound: Error reading SND header.");

    //Only support statically sized files
    if(header.count > 0) {
        error("catacomb_load_sound: Unsupported file type.");
    }

    //TODO: Add support for BIG-ENDIAN
    for(byte i = 0; i < MAX_SOUND_DEFS; i++) {
        if(fread(&defs[i], 1, sizeof(cat_sound_def), fp) != sizeof(cat_sound_def))
            error("catacomb_load_sound: Error reading SND definition %d", i);
    }

    //Safely remove and clean all existing sounds...
    catacomb_sounds_finish();

    cat_sound* snd;
    byte num_sounds = 0;
    for(byte i = 0; i < MAX_SOUND_DEFS; i++) {
        //subtract 2 because of the two 0xFF at the end of the sound that indicate it is the end.
        ushort sound_size = defs[i+1].offset - defs[i].offset - 2;

        //Don't load unused sounds!
        if(NO_UNUSED_SOUNDS && defs[i].flags == 0x1)
            continue;

        snd = &cat_sounds[num_sounds++];

        snd->priority = defs[i].priority;
        snd->flags = defs[i].flags;
        strcpy(snd->name, defs[i].name);

        //Seek to the data offset
        if(fseek(fp, defs[i].offset, SEEK_SET)) {
            error("catacomb_sounds_load: Error seeking to sound '%s' at offset %02X.", defs[i].name, defs[i].offset);
        }
        //Read the data.
        if(fread(&raw_sound_data, 1, sound_size, fp) != sound_size) {
            error("catacomb_sounds_load: Error loading sound '%s'", defs[i].name);
        }

        snd->sound_data = catacomb_sounds_raw_to_pcm((byte*)&raw_sound_data, sound_size, (int*)&snd->size);

        debug("Sound '%s' loaded...", snd->name);
        loaded_size += snd->size;
    }
    debug("Loaded %d sounds @ %.4f MB...", num_sounds, loaded_size/1024.0/1024.0);

    fclose(fp);
}

void catacomb_sounds_finish(void) {
    for(int i = 0; i < MAX_SOUND_DEFS; i++) {
        if(cat_sounds[i].sound_data) {
            memory_free(cat_sounds[i].sound_data);
        }
    }
    //empty the arr
    memset(&cat_sounds[0], 0, sizeof(cat_sound)*MAX_SOUND_DEFS);
}
