#include "sound_manager.h"
#include "catacomb/catacomb_sound.h"

#include <SDL.h>
#include <SDL_audio.h>

//http://www.libsdl.org/intro.en/usingsound.html
struct sample {
    byte *data;
    uint dpos;
    uint dlen;
} sounds[NUM_SOUNDS];

bool sounds_enabled = true;

static void sound_manager_mixaudio(void* unused, byte* stream, int len) {
    int amount;

    for (byte i = 0; i < NUM_SOUNDS; ++i) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
        sounds[i].dpos += amount;
    }
}


/*
===============
sound_manager_next_index

Gets the next available index for playing sounds.
===============
*/
static int sound_manager_next_index() {
    uint index;
    for(index = 0; index < NUM_SOUNDS; ++index) {
        if(sounds[index].dpos == sounds[index].dlen)
            break;
    }

    if(index == NUM_SOUNDS) {
        return -1;
    }
    return index;
}


/*
===============
sound_manager_init
===============
*/
void sound_manager_init() {
    int err = SDL_Init(SDL_INIT_AUDIO);
    if(err == -1) {
        error("Cannot initialize SDL audio!");
    }

    SDL_AudioSpec fmt;
    fmt.freq = 22050;
    fmt.format = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples = 512;
    fmt.callback = sound_manager_mixaudio;
    fmt.userdata = NULL;

    if(SDL_OpenAudio(&fmt, NULL) < 0) {
        error("Unable to open audio: %s", SDL_GetError());
    }

    SDL_PauseAudio(0);

    memset(&sounds[0], 0, sizeof(sounds));
}


/*
===============
sound_manager_finish
===============
*/
void sound_manager_finish() {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
}


/*
===============
sound_manager_play_data

Plays raw data.
===============
*/
bool sound_manager_play_data(byte *data, uint dlen) {
    if(!sounds_enabled)
        return true;

    int index = sound_manager_next_index();
    //max sounds  playing
    if(index < 0) {
        return false;
    }

    SDL_LockAudio();
        sounds[index].data = data;
        sounds[index].dlen = dlen;
        sounds[index].dpos = 0;
    SDL_UnlockAudio();

    return true;
}


/*
===============
sound_manager_play_wav
===============
*/
bool sound_manager_play_wav(const char* wav_path) {
    int index = sound_manager_next_index();
    if(index < 0) {
        warn("Maximum sounds playing, cannot play wav '%s'", wav_path);
        return false;
    }

    SDL_AudioSpec wave;
    byte *data;
    uint  dlen;

    if ( SDL_LoadWAV(wav_path, &wave, &data, &dlen) == NULL ) {
        error("Could not load %s : %s", wav_path, SDL_GetError());
    }

    return sound_manager_play_data(data, dlen);
}
