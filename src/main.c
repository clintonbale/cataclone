#include <SDL.h>
#include <SDL_opengl.h>

#include "common.h"
#include "graphics.h"
#include "draw.h"
#include "player.h"
#include "sound_manager.h"
#include "menu.h"

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

bool show_logo_screen();
void draw_black_bars();
void main_panel_update(const void*,float);

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

    running = show_logo_screen();

    menu_push(menu_create_side_panel(main_panel_update));

    gltexture_t* tiles = gl_find_gltexture("ALLTILES");

    uint current_time = SDL_GetTicks();
    uint previous_time;
    float frame_time;

    glEnable(GL_TEXTURE_2D);
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
                    else if(event.key.keysym.sym == SDLK_x)
                        player.health--;
                    else if(event.key.keysym.sym == SDLK_c)
                        player.shotpower++;
                    else if(event.key.keysym.sym == SDLK_z)
                        player.items[ITEM_KEY]++;
                    else if(event.key.keysym.sym == SDLK_v)
                        menu_push(menu_create_message_box("QUIT (Y/N)? "));
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
            }

            //handle other events here
            player_event(&event);
        }

        //Updating...
        player_update(frame_time);
        catacomb_level_update(frame_time);
        menu_update_all(frame_time);

        //Rendering...
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        catacomb_level_render();
        player_draw();

        menu_render_all();
        draw_black_bars();

/* draws all tiles on the top of the screen for debugging */
        int x = 0, y = 0;
        for(int i = 0; i < tiles->width/8; i++) {
            gl_draw_tile_spritesheet(tiles, i<<3, x, y);
            x+=8;
        }


        SDL_GL_SwapBuffers();
        //SDL_ShowFPS();
    }
    glDisable(GL_TEXTURE_2D);

    //Cleanup...
    menu_finish();
    catacomb_sounds_finish();
    sound_manager_finish();

    gl_draw_finish();
    graphics_finish();
    catacomb_sounds_finish();
    catacomb_graphics_finish();

    SDL_Quit();
    return 0;
}

#define MENU_PLAYER_HEALTH_Y 17
#define MENU_PLAYER_SHOTPW_Y 14
#define MENU_PROGRESS_WIDTH  13
#define MENU_ITEM_OFFSET 108

void main_panel_update(const void* ptr, float gt) {
    //Buffer for all int to string conversions
    static char buffer[16];
    //Store the last updated stuff so we dont have
    //to update everything all the itme.
    static byte last_health = 0;
    static byte last_shotpower = 0;
    static byte last_items[4] = {0,0,0,0};
    const menu_t* menu = (const menu_t*)ptr;

    sprintf(&buffer[0], "%-2d", catacomb_level_current()->level_number);
    menu_add_text(menu, 10, 1, buffer);

    sprintf(&buffer[0], "%-7d", player.score);
    menu_add_text(menu, 7, 3, buffer);

    //TODO: Get the top highscore.
    sprintf(&buffer[0], "%-7d", player.score);
    menu_add_text(menu, 7, 5, buffer);

    if(player.health != last_health) {
        byte* health_offset = &menu->data[MENU_PLAYER_HEALTH_Y*menu->size[0]];
        //Clear existing progress
        memset(health_offset+1, TILE_MENU_BLACK, 13);
        for(byte i = 2; i < player.health && i < MAX_PLAYER_HEALTH; ++i) {
            *(health_offset+i) = TILE_MENU_PROGRESS;
        }
        //Only draw the edges if we have health
        if(player.health > 0) {
            *(health_offset+1) = TILE_MENU_PROGRESSL;
            //Only draw the right edge if we're at the right
            if(player.health > 1)
                //Write the right edge & don't overflow!
                *(health_offset+(player.health>MAX_PLAYER_HEALTH?MAX_PLAYER_HEALTH:player.health)) = TILE_MENU_PROGRESSR;
        }
        last_health = player.health;
    }
    if(player.shotpower != last_shotpower) {
        byte* shotpower_offset = &menu->data[MENU_PLAYER_SHOTPW_Y*menu->size[0]];
        //Get the correct offset based on what the health is
        //If the health is at max, offset the tile index by -4
        //so that we can use the red tiles instead.
        int offset = player.shotpower >= MAX_SHOT_POWER ? -4 : 0;

        memset(shotpower_offset+1, TILE_MENU_BLACK, MENU_PROGRESS_WIDTH);
        for(byte i = 2; i < player.shotpower && i < MAX_PLAYER_HEALTH; ++i) {
            *(shotpower_offset+i) = TILE_MENU_PROGRESS+offset;
        }

        if(player.shotpower > 0) {
            *(shotpower_offset+1) = TILE_MENU_PROGRESSL+offset;
            if(player.shotpower > 1)
                //Write the right edge & don't overflow!
                *(shotpower_offset+(player.shotpower>MAX_PLAYER_HEALTH?MAX_PLAYER_HEALTH:player.shotpower)) = TILE_MENU_PROGRESSR+offset;
        }
        last_shotpower = player.shotpower;
    }
    //TODO: This double for may need some optimizing.
    for(byte i = 0; i < sizeof(player.items); ++i) {
        if(player.items[i] != last_items[i]) {
            byte tile = i == ITEM_KEY ? TILE_MENU_KEY : i == ITEM_POTION ? TILE_MENU_POT : TILE_MENU_SCROLL;
            for(byte n = 0; n < MAX_PLAYER_ITEMS; ++n) {
                menu->data[MENU_ITEM_OFFSET+(i*menu->size[0])+n] = player.items[i] > n ? tile : TILE_MENU_WHITE;
            }
            last_items[i] = player.items[i];
        }
    }
}

void draw_black_bars() {
    gltexture_t* main_tiles = gl_find_gltexture("MAIN");

    glPushMatrix(); //save the camera state
    glLoadIdentity();
    glOrtho(0, graphics_viewport_width(), graphics_viewport_height(), 0, 0.0f, 1.0f);

    const uint bar_x = graphics_viewport_width()-TILE_WIDTH;
    const uint bar_y = graphics_viewport_height()-TILE_HEIGHT;

    //Loop through and draw
    for(ushort x = 0; x < 320; x += TILE_WIDTH) {
        gl_draw_tile_spritesheet(main_tiles, 0, x, bar_y);
    }
    for(ushort y = 0; y < 200; y += TILE_HEIGHT) {
        gl_draw_tile_spritesheet(main_tiles, 0, bar_x, y);
    }

    glPopMatrix(); //restore the camera state
}

bool show_logo_screen() {
    bool selected = false;
    bool keep_running = true;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    //only need to draw it once.
    gl_draw_image(0, 0, graphics_viewport_width(), graphics_viewport_height(), gl_find_gltexture("TITLE")->texnum);
    glDisable(GL_TEXTURE_2D);

    SDL_GL_SwapBuffers();

    SDL_Event event;
    while(!selected) {
        //Sleep so we dont eat the cpu.
        SDL_Delay(100);
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                {
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            keep_running = false;
                            break;
                        case SDLK_c:
                            graphics_set_mode(GFX_MODE_CGA);
                            break;
                        default:
                            graphics_set_mode(GFX_MODE_EGA);
                            break;
                    }
                    selected = true;
                }
                break;
                default: break;
            }
        }
    }

    //clear before we leave
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    return keep_running;
}
