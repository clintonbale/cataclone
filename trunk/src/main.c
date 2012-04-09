#include <SDL.h>
#include <SDL_opengl.h>

#include "bool.h"
#include "error.h"

#include "graphics.h"
#include "draw.h"
#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_scores.h"
#include "catacomb/catacomb_graphics.h"

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
    sound_manager_init();
    gl_draw_init();
    catacomb_graphics_init();

    bool running = true;
    SDL_Event event;

    gltexture_t* tiles = gl_find_gltexture("TILES");
    while(running) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        running = false;
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


        int x = 0;
        int y = 0;
        /*
        gl_draw_tile_spritesheet(tiles, 300<<3, x, y);
        gl_draw_tile_spritesheet(tiles, 301<<3, x+8, y);
        gl_draw_tile_spritesheet(tiles, 302<<3, x, y+8);
        gl_draw_tile_spritesheet(tiles, 303<<3, x+8, y+8);*/

        for(int i = 0; i < 11696/8; i++) {
            gl_draw_tile_spritesheet(tiles, i<<3, x, y);
            x+=8;
            if(x > 44*8) {
                x = 0;
                y+=8;
            }
        }

        SDL_GL_SwapBuffers();
        SDL_ShowFPS();
    }

    gl_draw_finish();
    sound_manager_finish();
    graphics_finish();
    catacomb_sounds_finish();
    catacomb_graphics_finish();
    SDL_Quit();

    return 0;
}
