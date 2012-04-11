#include "sound_manager.h"
#include "error.h"
#include "catacomb/catacomb_sound.h"

#include <SDL.h>
#include <SDL_audio.h>

//http://www.libsdl.org/intro.en/usingsound.html

struct sample {
    uint8_t *data;
    uint32_t dpos;
    uint32_t dlen;
} sounds[NUM_SOUNDS];

extern void mixaudio(void* unused, uint8_t* stream, int len) {
    int i;
    uint32_t amount;

    for (i = 0; i < NUM_SOUNDS; ++i) {
        amount = (sounds[i].dlen-sounds[i].dpos);
        if ( amount > len ) {
            amount = len;
        }
        SDL_MixAudio(stream, &sounds[i].data[sounds[i].dpos], amount, SDL_MIX_MAXVOLUME);
        sounds[i].dpos += amount;
    }
}

static int sound_manager_next_index() {
    int index;
    for(index = 0; index < NUM_SOUNDS; ++index) {
        if(sounds[index].dpos == sounds[index].dlen)
            break;
    }

    if(index == NUM_SOUNDS) {
        return -1;
    }
    return index;
}


void sound_manager_init() {
    int err = SDL_Init(SDL_INIT_AUDIO);
    if(err == -1) {
        error("Cannot initialize SDL audio!");
    }

    SDL_AudioSpec fmt;
    fmt.freq = 44100;
    fmt.format = AUDIO_S16;
    fmt.channels = 2;
    fmt.samples = 512;
    fmt.callback = mixaudio;
    fmt.userdata = NULL;

    if(SDL_OpenAudio(&fmt, NULL) < 0) {
        error("Unable to open audio: %d", SDL_GetError());
    }

    SDL_PauseAudio(0);

    memset(&sounds[0], 0, sizeof(sounds)*NUM_SOUNDS);

    catacomb_sounds_load("SOUNDS.CAT");
}

void sound_manager_finish() {
    SDL_PauseAudio(1);
    SDL_CloseAudio();
    catacomb_sounds_finish();
}

//TODO: Add support for dpos?
bool sound_manager_play_data(uint8_t *data, uint32_t dlen) {
    int index = sound_manager_next_index();
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

bool sound_manager_play_wav(const char* wav_path) {
    int index = sound_manager_next_index();
    if(index < 0) {
        warn("Maximum sounds playing, cannot play wav '%s'", wav_path);
        return false;
    }

    SDL_AudioSpec wave;
    Uint8 *data;
    Uint32 dlen;

    if ( SDL_LoadWAV(wav_path, &wave, &data, &dlen) == NULL ) {
        error("Could not load %s : %s", wav_path, SDL_GetError());
    }

    return sound_manager_play_data(data, dlen);
}