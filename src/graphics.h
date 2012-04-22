#ifndef _GRAPHICS_H_
#define _GRAPHICS_H_

#include <SDL.h>
#include "common.h"

typedef enum { GFX_MODE_CGA, GFX_MODE_EGA } graphics_mode_t;

const SDL_Surface* graphics_get_screen();

void graphics_viewport_start();
void graphics_viewport_set_title(const char* title);
uint graphics_viewport_height();
uint graphics_viewport_width();

void graphics_set_mode(graphics_mode_t newMode);
graphics_mode_t graphics_get_mode(void);

void graphics_set_multisamples(int multisamples);
int graphics_get_multisamples(void);

void graphics_init();
void graphics_finish();

#endif
