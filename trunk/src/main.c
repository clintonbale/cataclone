#include <SDL.h>
#include <SDL_opengl.h>

#include "common.h"
#include "graphics.h"
#include "draw.h"
#include "player.h"

#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_scores.h"
#include "catacomb/catacomb_graphics.h"
#include "catacomb/catacomb_level.h"

void check_collision() {
   // level_t* l = catacomb_level_current();

}

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

    catacomb_level_change(1);
    player_init();

    gltexture_t* tiles = gl_find_gltexture("MISC");

    while(running) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    if(event.key.keysym.sym == SDLK_ESCAPE)
                        running = false;
                    else if(event.key.keysym.sym == SDLK_SPACE) {
                        //other stuff?
                        catacomb_level_next();
                        player_init();
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
            }

            //handle other events here
            player_event(&event);
        }

        player_update();

        //update
        //player_update() etc..

        //draw
        //player_draw() etc..

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);

        catacomb_level_render();

        player_draw();



/*
        vec2_t ps;
        int p = catacomb_level_player_start(ps);
        printf("x: %d, y: %d\n", ps[0], ps[1]);
        gl_draw_tile_spritesheet(tiles, 190<<3, ps[0]*8, ps[1]*8);

*/
        int x = 0, y = 0;
        for(int i = 0; i < 11696/8; i++) {
            gl_draw_tile_spritesheet(tiles, i<<3, x, y);
            x+=8;
        }


glDisable(GL_TEXTURE_2D);

        SDL_GL_SwapBuffers();

        //SDL_ShowFPS();
    }

    gl_draw_finish();
    graphics_finish();
    catacomb_sounds_finish();
    catacomb_graphics_finish();
    SDL_Quit();

    return 0;
}
