#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <SDL.h>
#include "common.h"

void window_set_title(const char* title);
uint window_viewport_height();
uint window_viewport_width();

void window_init();
void window_finish();

#endif
