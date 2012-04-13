#include <SDL.h>

#include "catacomb_graphics.h"
#include "catacomb_data.h"

#include "../draw.h"
#include "../graphics.h"

#define put_pixel(s,x,y,v) ((uint*)s->pixels)[((y)*s->w)+(x)]=(v)

#define PIC_COLOR_COUNT 4
static uint colormap[PIC_COLOR_COUNT] = {
    0,                          //black
    (255<<16)|(255<<8),         //cyan
    (255<<16)|(0<<8)|(255<<0),  //magenta
    (255<<16)|(255<<8)|(255<<0) //white
};

//Gets the four colors for four pixels in the 1 byte.
static inline void get_byte_colors(byte b, byte* colors) {
    colors[0] = (b>>6)&3; //xx000000
    colors[1] = (b>>4)&3; //00xx0000
    colors[2] = (b>>2)&3; //0000xx00
    colors[3] = (b>>0)&3; //000000xx
}

GLuint catacomb_graphics_load_pic(const char* ident, byte* data) {
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

    GLuint texture = gl_load_texture(ident, PIC_WIDTH, PIC_HEIGHT, pic->pixels);
    SDL_FreeSurface(pic);

    debug("Loaded %s.PIC... w: %d, h: %d", ident, pic->w, pic->h);
    return texture;
}

/*  ident:        the name of the texture
    data:         the pointer to the raw tile data
    tile_size:    is amount of bytes one tile takes
    start_index:  is the first tile index
    end_index:    is the last tile index  */
GLuint catacomb_graphics_load_tiles(const char* ident, const byte* data, uint tile_size, uint start_index, uint end_index) {
    SDL_Surface* pic = SDL_CreateRGBSurface(SDL_SWSURFACE, (end_index-start_index)*8, TILE_HEIGHT,32,0,0,0,0);
    if(!pic) { error("CreateRGBSurface failed: %s", SDL_GetError()); }

    //translate the tile index to bytes.
    start_index *= tile_size;
    end_index   *= tile_size;

    for(uint i = start_index; i < end_index; i += tile_size) {
        for(byte y = 0; y < TILE_HEIGHT; ++y) {
            for(byte x = 0; x < TILE_WIDTH; ++x) {
                byte n = data[i+y+24]>>(7-x)&1; //intensity
                byte b = data[i+y+16]>>(7-x)&1; //blue
                byte g = data[i+y+8 ]>>(7-x)&1; //green
                byte r = data[i+y   ]>>(7-x)&1; //red

                //get the color
                uint color = (r<<16)|(g<<8)|(b);
                //add the intensity to the color
                //if the color is black and has intensity, set it to grey,
                //else, add the intensity to the original color
                color = (!color&&n)?(0x555555):(color*(255-(n ? 0 : 0x55)));
                put_pixel(pic, TILE_WIDTH*((i-start_index)/tile_size)+x, y, color);
            }
        }
    }

    GLuint texture = gl_load_texture(ident, pic->w, pic->h, pic->pixels);
    SDL_FreeSurface(pic);

    debug("Loaded %s... w: %d, h: %d", ident, pic->w, pic->h);

    return texture;
}

void catacomb_graphics_init() {
    catacomb_graphics_load_pic("TITLE", (byte*)&TITLE_PIC);
    catacomb_graphics_load_pic("END", (byte*)&END_PIC);

    byte* data = NULL;
    uint tile_size = 0;

    graphics_mode_t mode = graphics_get_mode();
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

    catacomb_graphics_load_tiles("PLAYER", data, tile_size, 256, 320);
    catacomb_graphics_load_tiles("MISC",   data, tile_size, 0  , 256);
    catacomb_graphics_load_tiles("TILES",  data, tile_size, 0  , NUM_EGA_TILES);
}

void catacomb_graphics_finish() {

}
