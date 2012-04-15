#include <SDL.h>
#include <SDL_opengl.h>

#include "common.h"
#include "graphics.h"
#include "draw.h"
#include "player.h"
#include "sound_manager.h"

#include "catacomb/catacomb_sound.h"
#include "catacomb/catacomb_scores.h"
#include "catacomb/catacomb_graphics.h"
#include "catacomb/catacomb_level.h"
#include "catacomb/catacomb_tiles.h"

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

    sprintf((char*)&fpsstr, "FPS: %.2f", fps_framerate);
    graphics_viewport_set_title(fpsstr);
#undef ALPHA
}

#define MENU_LEVEL_OFFSET 25
#define MENU_SCORE_OFFSET 52
#define MENU_TOP_OFFSET   82
#define MENU_SHOTPOWER_OFFSET 211
#define MENU_BODY_OFFSET      256
#define MENU_ITEM_OFFSET 108

void game_menu_draw() {
    static const vec2_t menu_size = {15, 24};
    static char score[8];
    static char top[8];
    static char level[4];
    static char menu[] =
    "abbbbbbbbbbbbbc"
    "dlllLEVELllllle"
    "dllllllllllllle"
    "dSCORE:llllllle"
    "dllllllllllllle"
    "dTOPll:llllllle"
    "dllllllllllllle"
    "dK:llllllllllle"
    "dP:llllllllllle"
    "dB:llllllllllle"
    "dN:llllllllllle"
    "dllllllllllllle"
    "dllllllllllllle"
    "dlSHOTlPOWERlle"
    "d-------------e"
    "dllllllllllllle"
    "dllllBODYllllle"
    "d-------------e"
    "dllllllllllllle"
    "dllllllllllllle"
    "dllllllllllllle"
    "dllllllllllllle"
    "dllllllllllllle"
    "fgggggggggggggh";

    //Clear the current score/top/level with the WHITE tile.
    memset((char*)&score, 'l', sizeof(score));
    memset((char*)&top, 'l', sizeof(top));
    memset((char*)&level, 'l', sizeof(level));

    //Get the score/top/level
    sprintf((char*)&score, "%d", player.score);
    sprintf((char*)&top, "%d", player.score);
    sprintf((char*)&level, "%dl", catacomb_level_current()->level_number);

    //Set it
    memcpy(&menu[MENU_LEVEL_OFFSET], level, strlen(level));
    memcpy(&menu[MENU_SCORE_OFFSET], score, strlen(score));
    memcpy(&menu[MENU_TOP_OFFSET], top, strlen(top));

    for(byte i = 0; i < sizeof(player.items); ++i) {
        if(player.items[i] > 0) {
            byte tile = i == ITEM_KEY ? TILE_MENU_KEY : i == ITEM_POTION ? TILE_MENU_POT : TILE_MENU_SCROLL;
            for(byte n = 0; n < 10 && n < player.items[i]; n++) {
                menu[MENU_ITEM_OFFSET+(i*menu_size[0])+n] = tile;
            }
        }
    }

    //health progress bar
    for(byte i = 0; i < MAX_PLAYER_HEALTH; ++i) {
        if(i == 0 && player.health)
            menu[MENU_BODY_OFFSET] = TILE_MENU_PROGRESSL;
        else if(i == player.health-1)
            menu[MENU_BODY_OFFSET+i] = TILE_MENU_PROGRESSR;
        else if(i < player.health)
            menu[MENU_BODY_OFFSET+i] = TILE_MENU_PROGRESS;
        else
            menu[MENU_BODY_OFFSET+i] = TILE_MENU_BLACK;
    }

    //offset is used to make the progress bar red if we are at max power!!!
    int offset = player.shotpower == MAX_SHOT_POWER ? -4 : 0;
    for(byte i = 0; i < MAX_SHOT_POWER-1; ++i) {
        if(i == 0 && player.shotpower)
            menu[MENU_SHOTPOWER_OFFSET] = TILE_MENU_PROGRESSL+offset;
        else if(player.shotpower == MAX_SHOT_POWER && i == MAX_SHOT_POWER-2)
            menu[MENU_SHOTPOWER_OFFSET+i] = TILE_MENU_PROGRESSR+offset;
        else if(i == player.shotpower-1)
            menu[MENU_SHOTPOWER_OFFSET+i] = TILE_MENU_PROGRESSR+offset;
        else if(i < player.shotpower)
            menu[MENU_SHOTPOWER_OFFSET+i] = TILE_MENU_PROGRESS+offset;
        else
            menu[MENU_SHOTPOWER_OFFSET+i] = TILE_MENU_BLACK+offset;
    }

    gltexture_t* main = gl_find_gltexture("MAIN");
    for(byte x = 0; x < menu_size[0]; ++x) {
        for(byte y = 0; y < menu_size[1]; ++y) {
            char cur = menu[(y * menu_size[0]) + x];
            byte tile = cur > '`' ? TILE_MENU_TOPL+(cur-'a') : cur;
            gl_draw_tile_spritesheet(main, tile<<3, (x<<3)+player.position[0]+104, (y<<3)+player.position[1]-88);
        }
    }
    //draw the black borders
    for(byte y = 0; y < 200/TILE_HEIGHT; y++)
        gl_draw_tile_spritesheet(main, 0, player.position[0]+224, (y<<3)+player.position[1]-88);
    for(byte x = 0; x < 320/TILE_WIDTH; x++)
        gl_draw_tile_spritesheet(main, 0, (x<<3)+player.position[0]-88, player.position[1]+104);
}

int main(int argc, char* argv[])
{
#ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen( "CON", "w", stdout );
    FILE* ferr = freopen( "CON", "w", stderr );
#endif
    //graphics
    graphics_init();
    gl_draw_init();
    catacomb_graphics_init();

    //sounds
    sound_manager_init();
    catacomb_sounds_load("SOUNDS.CAT");

    catacomb_level_init();

    bool running = true;
    SDL_Event event;

    catacomb_level_change(1);
    player_reset();

    gltexture_t* tiles = gl_find_gltexture("ALLTILES");

    uint current_time = SDL_GetTicks();
    uint previous_time;
    float frame_time;
    while(running) {
        previous_time = current_time;
        current_time = SDL_GetTicks();
        frame_time = ((float)current_time - previous_time)/1000.f;

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
                    else if(event.key.keysym.sym == SDLK_x) {
                        player.health--;
                    }
                    else if(event.key.keysym.sym == SDLK_c) {
                        player.shotpower++;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
            }

            //handle other events here
            player_event(&event);
        }

        player_update(frame_time);
        catacomb_level_update(frame_time);

        //update
        //player_update() etc..

        //draw
        //player_draw() etc..

        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        glEnable(GL_TEXTURE_2D);

        catacomb_level_render();

        player_draw();
        game_menu_draw();

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

        //SDL_Delay(1000/16.0f);
        //SDL_ShowFPS();
    }


    catacomb_sounds_finish();
    sound_manager_finish();

    gl_draw_finish();
    graphics_finish();
    catacomb_sounds_finish();
    catacomb_graphics_finish();

    SDL_Quit();
    return 0;
}
