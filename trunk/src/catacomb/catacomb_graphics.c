#include <SDL.h>

#include "catacomb_graphics.h"
#include "catacomb_data.h"
#include "catacomb_defs.h"

#include "../render.h"

#define put_pixel(s,x,y,v) ((uint*)s->pixels)[((y)*s->w)+(x)]=(v)

#define PIC_COLOR_COUNT 4
static uint colormap[PIC_COLOR_COUNT] = {
    0,                          //black
    (255<<16)|(255<<8),         //cyan
    (255<<16)|(0<<8)|(255<<0),  //magenta
    (255<<16)|(255<<8)|(255<<0) //white
};

//Gets the four colors for four pixels in the 1 byte.
static void get_byte_colors(byte b, byte colors[4]) {
    colors[0] = (b>>6)&3; //xx000000
    colors[1] = (b>>4)&3; //00xx0000
    colors[2] = (b>>2)&3; //0000xx00
    colors[3] = (b>>0)&3; //000000xx
}

uint catacomb_graphics_load_pic(const char* ident, byte* data) {
    uint texture;
    if((texture = r_find_textureid(ident))>0) {
        warn("PIC %s.PIC already loaded!", ident);
        return texture;
    }

    SDL_Surface* pic = SDL_CreateRGBSurface(SDL_SWSURFACE, PIC_WIDTH, PIC_HEIGHT,32,0,0,0,0);
    if(!pic) { error("CreateRGBSurface failed: %s", SDL_GetError()); }

    ushort x = 0, y = 0;
    byte even_colors[4], odd_colors[4];
    for(ushort i = 0; i < 8000; ++i) {
        get_byte_colors(data[i],        (byte*)&even_colors);
        get_byte_colors(data[i+0x2000], (byte*)&odd_colors);

        for(byte p = 0; p < 4; ++p) {
            put_pixel(pic, x, y, colormap[even_colors[p]]);
            put_pixel(pic, x++, y+1, colormap[odd_colors[p]]);
        }
        if(x == 320) { x = 0; y += 2; }
    }

    texture = r_load_texture(ident, PIC_WIDTH, PIC_HEIGHT, pic->pixels);
    SDL_FreeSurface(pic);

    debug("PIC %s.PIC loaded... w: %d, h: %d", ident, pic->w, pic->h);
    return texture;
}

/*  ident:        the name of the texture
    data:         the pointer to the raw tile data
    tile_size:    is amount of bytes one tile takes
    start_index:  is the first tile index
    end_index:    is the last tile index  */
uint catacomb_graphics_load_tiles(const char* ident, const byte* data, uint tile_size, uint start_index, uint end_index) {
    uint texture;
    if((texture = r_find_textureid(ident))>0) {
        warn("Texture '%s' already loaded.", ident);
        return texture;
    }
    if(start_index > end_index) {
        error("catacomb_graphics_load_tiles: start_index > end_index");
    }

    SDL_Surface* pic = SDL_CreateRGBSurface(SDL_SWSURFACE, (int)(end_index-start_index)*TILE_WIDTH, TILE_HEIGHT, 32,0,0,0,0);
    if(!pic) { error("CreateRGBSurface failed: %s", SDL_GetError()); }

    //translate the tile index to bytes.
    start_index *= tile_size;
    end_index   *= tile_size;

    if(tile_size == EGA_TILE_SIZE) {
        for(uint i = start_index; i < end_index; i += tile_size) {
            for(byte y = 0; y < TILE_HEIGHT; ++y) {
                for(byte x = 0; x < TILE_WIDTH; ++x) {
                    byte n = data[i+y+24]>>(7-x)&1; //intensity
                    byte b = data[i+y+16]>>(7-x)&1; //blue
                    byte g = data[i+y+8 ]>>(7-x)&1; //green
                    byte r = data[i+y   ]>>(7-x)&1; //red

                    //get the color
                    uint color = (uint)((r<<16)|(g<<8)|(b));
                    //add the intensity to the color
                    //if the color is black and has intensity, set it to grey,
                    //else, add the intensity to the original color
                    color = (!color&&n)?(0x555555):(color*(255-(n ? 0 : 0x55)));
                    put_pixel(pic, TILE_WIDTH*((i-start_index)/tile_size)+x, y, color);
                }
            }
        }
    }
    else {
        for(uint i = start_index; i < end_index; i += tile_size) {
            byte left[4], right[4];
            for(byte y = 0; y < TILE_HEIGHT; ++y) {
                get_byte_colors(data[i+(y<<1)],  (byte*)&left);
                get_byte_colors(data[i+(y<<1)+1],(byte*)&right);

                for(byte x = 0; x < TILE_WIDTH/2; ++x) {
                    put_pixel(pic, TILE_WIDTH*((i-start_index)/tile_size)+x,               y, colormap[left[x]]);
                    put_pixel(pic, TILE_WIDTH*((i-start_index)/tile_size)+x+(TILE_WIDTH/2),y, colormap[right[x]]);
                }
            }
        }
    }

    texture = r_load_texture(ident, pic->w, pic->h, pic->pixels);
    SDL_FreeSurface(pic);

    debug("Texture '%s' loaded... w: %d, h: %d", ident, pic->w, pic->h);

    return texture;
}

void catacomb_graphics_init_pics() {
    catacomb_graphics_load_pic("TITLE", (byte*)&TITLE_PIC);
    catacomb_graphics_load_pic("END", (byte*)&END_PIC);
}

void catacomb_graphics_init_tiles(graphics_mode_t mode) {
    byte* data = NULL;
    uint tile_size = 0;

    if(mode == GFX_MODE_EGA) {
        data = (byte*)&EGA_DATA;
        tile_size = EGA_TILE_SIZE;
    }
    else if(mode == GFX_MODE_CGA) {
        data = (byte*)&CGA_DATA;
        tile_size = CGA_TILE_SIZE;
    }
    else {
        error("Invalid graphics mode: %d", mode);
    }

/*
    Tiles:
    000-256  : Main Map Tiles
    256-320  : Player
    320-384  : Red Imp (2 tiles wide)
    384-448  : White Imp
    448-480  : Bolt
    480-492  : Animation for monster death
    492-512  : End Level/ Teleport Gate
    512-656  : Big Red Imp (3 tiles wide)
    656-684  : Unknown? Fireball?
    684-939  : Big Purple Imp (4 tiles wide)
    939-987  : Unknown? Another fireball?
    987-1387 : Last boss
    1387-1487: Unknown? Another fireball? Unsure of size? could be 4 tiles @ 1387+100 or 3 tiles at 1387+75
*/

    catacomb_graphics_load_tiles("MAIN",   data, tile_size, MAINTILES_LUMP_START, MAINTILES_LUMP_END);
    catacomb_graphics_load_tiles("PLAYER", data, tile_size, PLAYER_LUMP_START, PLAYER_LUMP_END);
    catacomb_graphics_load_tiles("REDIMP", data, tile_size, REDIMP_LUMP_START, REDIMP_LUMP_END);
    catacomb_graphics_load_tiles("WHITEIMP", data, tile_size, WHITEIMP_LUMP_START, WHITEIMP_LUMP_END);
    catacomb_graphics_load_tiles("BOLT", data, tile_size, BOLT_LUMP_START, BOLT_LUMP_END);
    catacomb_graphics_load_tiles("MONSTERDEAD", data, tile_size, MONSTERDEAD_LUMP_START, MONSTERDEAD_LUMP_END);
    catacomb_graphics_load_tiles("TELE", data, tile_size, TELE_LUMP_START, TELE_LUMP_END);
    catacomb_graphics_load_tiles("BIGREDIMP", data, tile_size, BIGREDIMP_LUMP_START, BIGREDIMP_LUMP_END);
    catacomb_graphics_load_tiles("BIGREDIMP_FIREBALL", data, tile_size, BIGREDIMPATK_LUMP_START, BIGREDIMPATK_LUMP_END);
    catacomb_graphics_load_tiles("BIGPURPIMP", data, tile_size, BIGPURPIMP_LUMP_START, BIGPURPIMP_LUMP_END);
    catacomb_graphics_load_tiles("BIGPURPIMP_FIREBALL", data, tile_size, BIGPURPIMPATK_LUMP_START, BIGPURPIMPATK_LUMP_END);
    catacomb_graphics_load_tiles("LASTBOSS", data, tile_size, LASTBOSS_LUMP_START, LASTBOSS_LUMP_END);
    catacomb_graphics_load_tiles("LASTBOSS_FIREBALL", data, tile_size, LASTBOSSATK_LUMP_START, LASTBOSSATK_LUMP_END);
    //catacomb_graphics_load_tiles("ALLTILES",  data, tile_size, 0, NUM_TILES);
}

void catacomb_graphics_finish() {

}
