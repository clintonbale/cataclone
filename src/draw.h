#ifndef _ASSETS_H_
#define _ASSETS_H_

#include <SDL_opengl.h>
#include "common.h"

#define TILE_HEIGHT 8
#define TILE_WIDTH  8

typedef struct {
    GLuint texnum;
    char identifier[32];
    int width, height;
} gltexture_t;

#define MAX_GLTEXTURES 64
gltexture_t gltextures[MAX_GLTEXTURES];
uint16_t    numgltextures;
GLuint      currenttexture;

void gl_draw_init(void);
void gl_draw_finish(void);

gltexture_t* gl_find_gltexture(const char* ident);
GLuint gl_find_texture(const char* ident);
GLuint gl_load_texture(const char* ident, int width, int height, byte* data);
GLuint gl_load_from_file(const char* filePath, const char* ident);

void gl_bind(GLuint texnum);

void gl_draw_tile_spritesheet(gltexture_t* spritesheet, unsigned tile_x, int x, int y);
void gl_draw_tile(int x, int y, GLuint texture);
void gl_draw_image(int x, int y, uint w, uint h, GLuint texture);

#endif //_ASSETS_H_
