#include <SDL.h>
#include <SDL_opengl.h>

#include "bool.h"
#include "error.h"

#include "graphics.h"
#include "draw.h"
#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_scores.h"
#include "catacomb/catacomb_graphics.h"
#include "catacomb/catacomb_level.h"

void SDL_ShowFPS() {
    #define ALPHA 0.2
    static char fpsstr[16];
    static uint32_t fps_lasttime = 0, fps_ticks, fps_delta;
    static float    fps_frametime, fps_framerate;

    fps_ticks = SDL_GetTicks();
    fps_delta = fps_ticks - fps_lasttime;
    fps_lasttime = fps_ticks;

    fps_frametime = ALPHA * fps_delta + (1.0 - ALPHA) * fps_frametime;
    fps_framerate = 1000.0/fps_frametime;

    sprintf(&fpsstr, "FPS: %.2f", fps_framerate);
    graphics_viewport_set_title(fpsstr);
    #undef ALPHA
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
#endif
    graphics_init();
    gl_draw_init();
    catacomb_graphics_init();

    bool running = true;
    SDL_Event event;

    gltexture_t* tiles = gl_find_gltexture("TILES");

    catacomb_level_change(1);

    while(running) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        running = false;
                    if(event.key.keysym.sym == SDLK_SPACE)
                        catacomb_level_next();
                    //other stuff?
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
            }

            //handle other events here
            //player_event(&event) ... etc..
        }

        //update
        //player_update() etc..

        //draw
        //player_draw() etc..

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        //draw the pink background
        for(int y = 0; y < graphics_viewport_height()/8; y++) {
            for(int x = 0; x < graphics_viewport_width()/8; x++) {
                gl_draw_tile_spritesheet(tiles, ('z'+7)*8, x*8, y*8);
            }
        }

        //draw the map!
        for(int y = 0; y < 64; y++) {
            for(int x = 0; x < 64; x++) {
                gl_draw_tile_spritesheet(tiles, catacomb_level_current()->tiles[(y*64)+x]*8, x*8, y*8);
            }
        }

        SDL_GL_SwapBuffers();
        SDL_ShowFPS();
    }

    gl_draw_finish();
    graphics_finish();
    catacomb_sounds_finish();
    catacomb_graphics_finish();
    SDL_Quit();

    return 0;
}
