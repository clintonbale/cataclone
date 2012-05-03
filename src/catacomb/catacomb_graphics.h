#ifndef _CATACOMB_GRAPHICS_H_
#define _CATACOMB_GRAPHICS_H_

#include <SDL_opengl.h>
#include "../common.h"

#define PIC_WIDTH  320
#define PIC_HEIGHT 200

typedef enum { GFX_MODE_CGA, GFX_MODE_EGA } graphics_mode_t;

void catacomb_graphics_init_pics();
void catacomb_graphics_init_tiles(graphics_mode_t mode);
void catacomb_graphics_finish();

uint catacomb_graphics_load_pic(const char* ident, byte* data);
uint catacomb_graphics_load_tiles(const char* ident, const byte* data, const uint tile_size, uint start_index, uint end_index);

#endif // _CATACOMB_GRAPHICS_H_
