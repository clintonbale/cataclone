#ifndef _CATACOMB_GRAPHICS_H_
#define _CATACOMB_GRAPHICS_H_

#include <SDL_opengl.h>
#include "../common.h"

#define PIC_WIDTH  320
#define PIC_HEIGHT 200

void catacomb_graphics_init();
void catacomb_graphics_finish();

GLuint catacomb_graphics_load_pic(const char* ident, byte* data);
GLuint catacomb_graphics_load_tiles(const char* ident, byte* data);

#endif // _CATACOMB_GRAPHICS_H_
