#ifndef _RENDER_H_
#define _RENDER_H_

#include "common.h"

#define TILE_HEIGHT 8
#define TILE_WIDTH  8

typedef struct {
    uint    texnum;
    char    identifier[24];
    ushort  width;
    ushort  height;
} texture_t;

texture_t*  r_find_texture(const char* ident);
uint        r_find_textureid(const char* ident);
uint        r_load_texture(const char* ident, ushort width, ushort height, byte* data);

void        r_bind(uint texnum);

void        r_draw_tile(const texture_t* spritesheet, ushort tile_x, int32_t x, int32_t y);
void        r_draw_image(int32_t x, int32_t y, uint w, uint h, uint texture);

#endif //_RENDER_H_
