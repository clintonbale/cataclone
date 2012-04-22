#include "menu.h"
#include "catacomb/catacomb_tiles.h"
#include "draw.h"
#include "graphics.h"

menu_t* menu_new(ushort x, ushort y, ushort w, ushort h, update_ptr func) {
    menu_t* new_menu = NULL;

    new_menu = malloc(sizeof(menu_t));
    if(!new_menu) error("Out of memory!");

    new_menu->data = malloc(w*h);
    if(!new_menu->data) error("Out of memory!");

    new_menu->size[0] = w;
    new_menu->size[1] = h;
    new_menu->position[0] = x;
    new_menu->position[1] = y;
    new_menu->update = func;
    new_menu->visible = true;

    byte tile = 0;
    //build the borders and the guts
    ushort index = 0;
    for(y = 0; y < h; ++y) {
        for(x = 0; x < w; ++x) {
            if(y == 0 && x == 0)
                tile = TILE_MENU_TOPL;
            else if(y == 0 && x == w-1)
                tile = TILE_MENU_TOPR;
            else if(y == h-1 && x == 0)
                tile = TILE_MENU_BOTTOML;
            else if(y == h-1 && x == w-1)
                tile = TILE_MENU_BOTTOMR;
            else if(y == 0)
                tile = TILE_MENU_TOP;
            else if(x == 0)
                tile = TILE_MENU_LEFT;
            else if(x == w-1)
                tile = TILE_MENU_RIGHT;
            else if(y == h-1)
                tile = TILE_MENU_BOTTOM;
            else
                tile = TILE_MENU_WHITE;
            new_menu->data[index++] = tile;
        }
    }
    debug("Menu: %p | Data: %p | {%d,%d} | {%d,%d}", new_menu, new_menu->data, new_menu->position[0], new_menu->position[1],w,h);
    return new_menu;
}

void menu_free(menu_t* menu) {
    if(menu) {
        if(menu->data) {
            free(menu->data);
            menu->data = (byte*)0;
        }
        free(menu);
        menu = (menu_t*)0;
    }
}

void menu_tick(const menu_t* menu, float gt) {
    if(!menu || !menu->data)
        error("NULL menu.");
    if(menu->update)
        menu->update(menu, gt);
    if(!menu->visible)
        return;


    gltexture_t* main_tiles = gl_find_gltexture("MAIN");

    //Dereference the members first to save time.
    const ushort w = menu->size[0], h = menu->size[1];
    const ushort x = menu->position[0], y = menu->position[1];
    const byte* data = menu->data;

    glPushMatrix(); //save the camera state
    glLoadIdentity();
    glOrtho(0, graphics_viewport_width(), graphics_viewport_height(), 0, 0.0f, 1.0f);

    for(ushort i = 0; i < (w*h); ++i) {
        gl_draw_tile_spritesheet(main_tiles, data[i]<<3, ((i%w)+x)*TILE_WIDTH, ((i/w)+y)*TILE_HEIGHT);
    }

    glPopMatrix(); //restore the camera state
}

void menu_add_text(const menu_t* menu, ushort x, ushort y, const char* text) {
    if(!menu || !menu->data)
        error("NULL menu.");

    memcpy(&menu->data[(y*menu->size[0])+x], text, strlen(text));
}

menu_t* menu_create_side_panel(update_ptr func) {
    menu_t* panel = menu_new(SIDE_PANEL_X, SIDE_PANEL_Y, SIDE_PANEL_WIDTH, SIDE_PANEL_HEIGHT, func);

    menu_add_text(panel, 4, 1, "LEVEL");
    menu_add_text(panel, 1, 3, "SCORE:");
    menu_add_text(panel, 1, 5, "TOP  :");
    menu_add_text(panel, 1, 7, "K:");
    menu_add_text(panel, 1, 8, "P:");
    menu_add_text(panel, 1, 9, "B:");
    menu_add_text(panel, 1, 10, "N:");
    menu_add_text(panel, 2, 13, "SHOT POWER");
    menu_add_text(panel, 5, 16, "BODY");

    for(byte x = 1; x < 14; x++) {
        panel->data[(14*panel->size[0])+x] = TILE_MENU_BLACK;
        panel->data[(17*panel->size[0])+x] = TILE_MENU_BLACK;
    }

    return panel;
}
