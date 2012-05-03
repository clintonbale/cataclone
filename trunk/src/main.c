#include <SDL.h>
#include <SDL_opengl.h>

#include "common.h"
#include "window.h"
#include "render.h"
#include "player.h"
#include "sound_manager.h"
#include "menu.h"
#include "memory.h"

#include "catacomb/catacomb_defs.h"
#include "catacomb/catacomb_graphics.h"
#include "catacomb/catacomb_level.h"
#include "catacomb/catacomb_scores.h"
#include "catacomb/catacomb_sound.h"

bool show_logo_screen();
void draw_black_bars();

void main_panel_update(const void*,float);
void panel_blob_update(const void*,float);
void sound_panel_update(const void*, float);

void quit_panel_event(const void*,SDL_Event*);
void reset_panel_event(const void*,SDL_Event*);
void sound_panel_event(const void*,SDL_Event*);

void init_all();
void finish_all();

float get_frame_time(void);
void display_fps();

bool running;

int main(int argc, char* argv[])
{
#ifdef _WIN32
    FILE* ctt = fopen("CON", "w" );
    FILE* fout = freopen("CON", "w", stdout );
    FILE* ferr = freopen("CON", "w", stderr );
#endif
    //init everything
    window_init();
    sound_manager_init();

    //inits pics (for start menu)
    catacomb_graphics_init_pics();

    graphics_mode_t mode = GFX_MODE_EGA;
    //show logo screen
    running = show_logo_screen(&mode);

    //init the tiles (based on loading screen selection) and level
    catacomb_sounds_load("SOUNDS.CAT");
    catacomb_graphics_init_tiles(mode);
    catacomb_level_init();
    catacomb_level_change(1);

    //Must be initialized after textures are loaded.
    player_init();

    //load the side menu
    menu_push(menu_create_side_panel(main_panel_update));

    bool showfps = false;
    SDL_Event event;

    glEnable(GL_TEXTURE_2D);
    while(running) {
        float frame_time = get_frame_time();
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYUP:
                    switch(event.key.keysym.sym) {
                        case SDLK_F12:
                            if(!(showfps = !showfps)) window_set_title("");
                            break;
                        case SDLK_SPACE: //TEMPORARY, for debugging.
                            catacomb_level_next();
                            player_start();
                            break;
                        default: break;
                    }
                    break;
                case SDL_KEYDOWN:
                    //Don't create a new menu if one is already visible.
                    if(menu_dialog_visible())
                        break;
                    //TODO: Refactor menu creation code to a different function?
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            //Clear all buttons pressed from the player. So they dont move while in menu.
                            player_clear_input();
                            menu_push(menu_create_simple_box("QUIT (Y/N)?*", panel_blob_update, quit_panel_event));
                            continue;
                        case SDLK_F1:
                            //TODO: Show help menu!
                            continue;
                        case SDLK_F2:
                            player_clear_input();
                            menu_push(menu_create_simple_box("SOUND* ON  OFF ", sound_panel_update, sound_panel_event));
                            continue;
                        case SDLK_F3:
                            //TODO: Show control menu
                            continue;
                        case SDLK_F4:
                            player_clear_input();
                            menu_push(menu_create_simple_box("RESET GAME (Y/N)?*", panel_blob_update, reset_panel_event));
                            continue;
                        default: break;
                    }
                    break;
                case SDL_QUIT:
                    running = false;
                    break;
                default: break;
            }

            menu_event_all(&event);

            //don't update the player events if a menu is open.
            if(!menu_dialog_visible())
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

        int x = 0;
        for(int i = 0; i < r_find_texture("MAIN")->width/8; i++) {
            r_draw_tile(r_find_texture("MAIN"), i<<3, x, 0);
            x+=8;
        }

        SDL_GL_SwapBuffers();
        if(showfps) display_fps();
    }
    glDisable(GL_TEXTURE_2D);
    finish_all();

#ifdef _DEBUG
    memory_report();
#endif
    SDL_Quit();
    return 0;
}

void finish_all() {
    window_finish();
    sound_manager_finish();
    menu_clear();

    catacomb_graphics_finish();
    catacomb_sounds_finish();
    catacomb_level_finish();
}

float get_frame_time(void) {
    static uint current_time = 0, previous_time = 0;
    static float frame_time = 0.0f;

    previous_time = current_time;
    current_time = SDL_GetTicks();
    frame_time = ((float)current_time - previous_time)/1000.f;

    return frame_time;
}

void display_fps() {
    static char fpsstr[16];
    static uint fps_lasttime = 0, fps_ticks, fps_delta;
    static float fps_frametime, fps_framerate;

    fps_ticks = SDL_GetTicks();
    fps_delta = fps_ticks - fps_lasttime;
    fps_lasttime = fps_ticks;

    fps_frametime = 0.2f * fps_delta + (1.0 - 0.2f) * fps_frametime;
    fps_framerate = 1000.0/fps_frametime;

    sprintf((char*)&fpsstr, "FPS: %.2f", fps_framerate);
    window_set_title(fpsstr);
}

#define SOUND_BLOB_OFFSET 6

#define EXTRA_SELECTED_INDEX 3
#define EXTRA_BLOB_LOCATION_Y 2
#define EXTRA_BLOB_LOCATION_X 1
#define EXTRA_BLOB_ANIMATION 0

void sound_panel_update(const void* ptr, float gt) {
    const menu_t* menu = (const menu_t*)ptr;
    byte* extra = (byte*)menu->extra;
    extra[EXTRA_SELECTED_INDEX] = sounds_enabled;

    //update the blob!
    panel_blob_update(ptr, gt);

    //TODO: Support CGA mode, clfean up the code?
    if(extra[EXTRA_SELECTED_INDEX]) {
        menu->data[menu->size.x+7] = T_PINK;
        menu->data[menu->size.x+8] = 'o';
        menu->data[menu->size.x+9] = 'n';
        menu->data[menu->size.x+10] = T_PINK;

        menu->data[menu->size.x+11] = T_MNU_WHITE;
        menu->data[menu->size.x+12] = 'O';
        menu->data[menu->size.x+13] = 'F';
        menu->data[menu->size.x+14] = 'F';
        menu->data[menu->size.x+15] = T_MNU_WHITE;
    }
    else {
        menu->data[menu->size.x+7] = T_MNU_WHITE;
        menu->data[menu->size.x+8] = 'O';
        menu->data[menu->size.x+9] = 'N';
        menu->data[menu->size.x+10] = T_MNU_WHITE;

        menu->data[menu->size.x+11] = T_PINK;
        menu->data[menu->size.x+12] = 'o';
        menu->data[menu->size.x+13] = 'f';
        menu->data[menu->size.x+14] = 'f';
        menu->data[menu->size.x+15] = T_PINK;
    }
}

void sound_panel_event(const void* ptr, SDL_Event* evt) {
    const menu_t* menu = (const menu_t*)ptr;
    byte* extra = (byte*)menu->extra;

    if(evt->type == SDL_KEYDOWN) {
        switch(evt->key.keysym.sym) {
            case SDLK_LEFT:
            case SDLK_RIGHT:
                extra[EXTRA_SELECTED_INDEX] = !extra[EXTRA_SELECTED_INDEX];
                //keep sounds_enabled up to date so when we load it the next time it's correct!
                sounds_enabled = extra[EXTRA_SELECTED_INDEX];
                break;
            default:
                debug("Sounds: %s", sounds_enabled ? "ENABLED" : "DISABLED");
                menu_free(menu_pop());
                break;
        }
    }
}

void reset_panel_event(const void* ptr, SDL_Event* evt) {
    if(evt->type == SDL_KEYDOWN) {
        if(evt->key.keysym.sym == SDLK_y) {
            catacomb_level_change(1);
            player_init();
            //TODO: Show highscores, load and play demo screen.
        }
        menu_free(menu_pop());
    }
}

void quit_panel_event(const void* ptr, SDL_Event* evt) {
    if(evt->type == SDL_KEYDOWN) {
        if(evt->key.keysym.sym == SDLK_y)
            running = false;
        menu_free(menu_pop());
    }
#ifdef _DEBUG
    //So we dont have to press y every time...
    running = false;
#endif
}

void panel_blob_update(const void* ptr, float gt) {
    static float elapsed = 1.0f;
    elapsed += gt;

    const menu_t* menu = (const menu_t*)ptr;
    if(!menu->extra) error("Erroneous menu_t.");
    byte* extra = (byte*)menu->extra;

    if(extra[EXTRA_BLOB_LOCATION_X] == 0 && extra[EXTRA_BLOB_LOCATION_Y] == 0) {
        for(ushort i = 0; i < (menu->size.x * menu->size.y); ++i) {
            if(menu->data[i]=='*') {
                extra[EXTRA_BLOB_LOCATION_X] = i % menu->size.x;
                extra[EXTRA_BLOB_LOCATION_Y] = i / menu->size.x;
                menu->data[i] = T_MNU_WHITE; // so we don't see the star.
                break;
            }
        }
    }

    if(elapsed > 0.15f) {
        elapsed = 0.0f;
        menu->data[(extra[EXTRA_BLOB_LOCATION_Y]*menu->size.x)+extra[EXTRA_BLOB_LOCATION_X]] = T_MNU_BLOB+extra[EXTRA_BLOB_ANIMATION];
        extra[EXTRA_BLOB_ANIMATION] = (extra[EXTRA_BLOB_ANIMATION]+1)%4;
    }
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
        byte* health_offset = &menu->data[MENU_PLAYER_HEALTH_Y*menu->size.x];
        //Clear existing progress
        memset(health_offset+1, T_BLACK, 13);
        for(byte i = 2; i < player.health && i < MAX_PLAYER_HEALTH; ++i) {
            *(health_offset+i) = T_MNU_PROG_M;
        }
        //Only draw the edges if we have health
        if(player.health > 0) {
            *(health_offset+1) = T_MNU_PROG_L;
            //Only draw the right edge if we're at the right
            if(player.health > 1)
                //Write the right edge & don't overflow!
                *(health_offset+(player.health>MAX_PLAYER_HEALTH?MAX_PLAYER_HEALTH:player.health)) = T_MNU_PROG_R;
        }
        last_health = player.health;
    }
    if(player.shotpower != last_shotpower) {
        byte* shotpower_offset = &menu->data[MENU_PLAYER_SHOTPW_Y*menu->size.x];
        //Get the correct offset based on what the health is
        //If the health is at max, offset the tile index by -4
        //so that we can use the red tiles instead.
        int offset = player.shotpower >= MAX_SHOT_POWER ? -4 : 0;

        memset(shotpower_offset+1, T_BLACK, MENU_PROGRESS_WIDTH);
        for(byte i = 2; i < player.shotpower && i < MAX_PLAYER_HEALTH; ++i) {
            *(shotpower_offset+i) = T_MNU_PROG_M+offset;
        }

        if(player.shotpower > 0) {
            *(shotpower_offset+1) = T_MNU_PROG_L+offset;
            if(player.shotpower > 1)
                //Write the right edge & don't overflow!
                *(shotpower_offset+(player.shotpower>MAX_PLAYER_HEALTH?MAX_PLAYER_HEALTH:player.shotpower)) = T_MNU_PROG_R+offset;
        }
        last_shotpower = player.shotpower;
    }
    //TODO: This double for may need some optimizing.
    for(byte i = 0; i < sizeof(player.items); ++i) {
        if(player.items[i] != last_items[i]) {
            byte tile = i == ITEM_KEY ? T_MNU_KEY : i == ITEM_POTION ? T_MNU_POTION : T_MNU_SCROLL;
            for(byte n = 0; n < MAX_PLAYER_ITEMS; ++n) {
                menu->data[MENU_ITEM_OFFSET+(i*menu->size.x)+n] = player.items[i] > n ? tile : T_MNU_WHITE;
            }
            last_items[i] = player.items[i];
        }
    }
}

void draw_black_bars() {
    //TODO: somehow keep a global reference of all loaded textures?
    texture_t* tex_main = r_find_texture("MAIN");

    glPushMatrix(); //save the camera state
    glLoadIdentity();
    glOrtho(0, window_viewport_width(), window_viewport_height(), 0, 0.0f, 1.0f);

    const uint bar_x = window_viewport_width()-TILE_WIDTH;
    const uint bar_y = window_viewport_height()-TILE_HEIGHT;

    //Loop through and draw
    for(ushort x = 0; x < 320; x += TILE_WIDTH) {
        r_draw_tile(tex_main, 0, x, bar_y);
    }
    for(ushort y = 0; y < 200; y += TILE_HEIGHT) {
        r_draw_tile(tex_main, 0, bar_x, y);
    }

    glPopMatrix(); //restore the camera state
}

bool show_logo_screen(graphics_mode_t* mode) {
    bool selected = false;
    bool keep_running = true;

    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glEnable(GL_TEXTURE_2D);
    //only need to draw it once.
    r_draw_image(0, 0, window_viewport_width(), window_viewport_height(), r_find_texture("TITLE")->texnum);
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
                            *mode = GFX_MODE_CGA;
                            break;
                        default:
                            *mode = GFX_MODE_EGA;
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
