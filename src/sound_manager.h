#ifndef _SOUND_MANAGER_H_
#define _SOUND_MANAGER_H_

#include "common.h"
#define NUM_SOUNDS 6

extern bool sounds_enabled;

void sound_manager_init();
void sound_manager_finish();

bool sound_manager_play_data(byte *data, uint dlen);
bool sound_manager_play_wav(const char* wavpath);

#endif // _SOUND_MANAGER_H_
