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

    debug("Loaded %s.PIC ...", ident);
    return texture;
}


GLuint catacomb_graphics_load_tiles(const char* ident, const byte* data, const uint length, const uint tile_size, const uint start_index, const uint end_index) {
    SDL_Surface* pic = SDL_CreateRGBSurface(SDL_SWSURFACE, NUM_EGA_TILES*TILE_WIDTH, TILE_HEIGHT,32,0,0,0,0);
    if(!pic) { error("CreateRGBSurface failed: %s", SDL_GetError()); }

    for(uint i = start_index; i < end_index; i += tile_size) {
        for(byte y = 0; y < 8; ++y) {
            for(byte x = 0; x < 8; ++x) {
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
                put_pixel(pic, 8*(i/32)+x, y, color);
            }
        }
    }

    GLuint texture = gl_load_texture(ident, pic->w, pic->h, pic->pixels);
    SDL_FreeSurface(pic);

    debug("Loaded %s...", ident);

    return texture;
}

#define TRANSLATE_INDEX(x) ((x)*4)
void catacomb_graphics_init() {
    catacomb_graphics_load_pic("TITLE", &TITLE_PIC);
    catacomb_graphics_load_pic("END", &END_PIC);

    byte* data = NULL;
    uint length = 0;
    uint tile_size = 0;

    graphics_mode_t mode = graphics_get_mode();
    if(mode == GFX_MODE_EGA) {
        data = &EGA_DATA;
        length = EGA_DATA_LENGTH;
        tile_size = EGA_TILE_SIZE;
    }
    else if(mode == GFX_MODE_CGA) {
        data = &CGA_DATA;
        length = CGA_DATA_LENGTH;
        tile_size = CGA_TILE_SIZE;
    }
    else {
        error("Invalid graphics mode: %d", mode);
    }


    catacomb_graphics_load_tiles("PLAYER", data, length, tile_size, TRANSLATE_INDEX(2048), TRANSLATE_INDEX(2560));
    catacomb_graphics_load_tiles("MISC",   data, length, tile_size, 0,    TRANSLATE_INDEX(2048));
    //catacomb_graphics_load_tiles("TILES", data, length, tile_size, 0, EGA_DATA_LENGTH);
}

void catacomb_graphics_finish() {

}
