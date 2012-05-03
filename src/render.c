#include "render.h"
#include <SDL_opengl.h>

#define MAX_TEXTURES 64
static texture_t    textures[MAX_TEXTURES];
static ushort       numgltextures = 0;
static uint         currenttexture = -1;


/*
===============
r_find_texture

Gets the texture_t* assosciated with the ident.
===============
*/
texture_t* r_find_texture(const char* ident) {
    for(ushort i = 0; i < numgltextures; ++i) {
        if(!strcmp(ident, textures[i].identifier))
            return &textures[i];
    }
    return NULL;
}


/*
===============
r_find_textureid

Gets the OpenGL texture id assosciated with the ident.
===============
*/
uint r_find_textureid(const char* ident) {
    texture_t* texture = r_find_texture(ident);
    return texture ? texture->texnum : 0;
}


/*
===============
r_load_texture

Loads an RGBA texture.
===============
*/
uint r_load_texture(const char* ident, ushort width, ushort height, byte* data) {
    uint texture = 0;
    texture_t* tex = NULL;

    //Already loaded?
    if(ident[0] && (texture = r_find_textureid(ident)) > 0) {
        return texture;
    }

    if(data == NULL) {
        error("gl_load_texture: data is NULL");
    }

    glGenTextures(1, &texture);
    if(!texture) {
        error("gl_load_texture: glGenTextures() failure: %d", glGetError());
    }
    tex = &textures[numgltextures];
    numgltextures++;

    glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, 3, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    tex->height = height;
    tex->width = width;
    tex->texnum = texture;
    strcpy(tex->identifier, ident);

    return texture;
}


/*
===============
r_bind
===============
*/
void r_bind(uint texnum) {
    if(currenttexture == texnum)
        return;
    glBindTexture(GL_TEXTURE_2D, currenttexture = texnum);
}


/*
===============
r_draw_tile

Draws a single tile from a spritesheet.
===============
*/
void r_draw_tile(const texture_t* spritesheet, ushort tile_x, int32_t x, int32_t y) {
    r_bind(spritesheet->texnum);
    glBegin(GL_QUADS);
        glTexCoord2d(tile_x/(float)spritesheet->width, 0);              glVertex2f(x,y);
        glTexCoord2d((tile_x+TILE_WIDTH)/(float)spritesheet->width,0);  glVertex2f(x+TILE_WIDTH,y);
        glTexCoord2d((tile_x+TILE_WIDTH)/(float)spritesheet->width,1);  glVertex2f(x+TILE_WIDTH,y+TILE_HEIGHT);
        glTexCoord2d(tile_x/(float)spritesheet->width, 1);              glVertex2f(x,y+TILE_HEIGHT);
    glEnd();
}


/*
===============
r_draw_image

Draws a image with specified dimensions.
===============
*/
void r_draw_image(int32_t x, int32_t y, uint w, uint h, uint texture) {
    r_bind(texture);
    glBegin( GL_QUADS );
        glTexCoord2f(0, 0); glVertex3f(x, y, 0);
        glTexCoord2f(1, 0); glVertex3f(x+w, y , 0);
        glTexCoord2f(1, 1); glVertex3f(x+w, y+h, 0);
        glTexCoord2f(0, 1); glVertex3f(x, y+h, 0);
    glEnd();
}
