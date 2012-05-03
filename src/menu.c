#include "menu.h"
#include "catacomb/catacomb_defs.h"
#include "render.h"
#include "window.h"
#include "memory.h"
#include <SDL_opengl.h>

#define MAX_MENUS 4
static menu_t* menu_stack[MAX_MENUS] = {(menu_t*)0};
static byte    num_menus = 0;


/*
===============
menu_push

Pushes a menu_t* on the top of the stack
===============
*/
menu_t* menu_push(const menu_t* menu) {
    if(num_menus+1 > MAX_MENUS) error("%s", "Cannot add new menu, the menu_stack is full.");
    return menu_stack[num_menus++] = (menu_t*)menu;
}


/*
===============
menu_pop
===============
*/
menu_t* menu_pop(void) {
    if(!num_menus) error("%s", "No elements in the menu_stack.");
    return menu_stack[--num_menus];
}


/*
===============
menu_peek

Gets the menu_t* on the top of the stack without popping
===============
*/
menu_t* menu_peek(void) {
    if(!num_menus) error("%s", "No elements in the menu_stack.");
    return menu_stack[num_menus-1];
}


/*
===============
menu_new

Creates a new empty menu
===============
*/
menu_t* menu_new(ushort x, ushort y, ushort w, ushort h, update_ptr u_func, event_ptr e_func) {
    menu_t* new_menu = NULL;

    new_menu = memory_alloc(sizeof(menu_t));
    new_menu->data = memory_alloc(w*h);

    new_menu->size.x = w;
    new_menu->size.y = h;
    new_menu->position.x = x;
    new_menu->position.y = y;
    new_menu->extra = (void*)0;
    new_menu->update = u_func ? u_func : 0;
    new_menu->event = e_func ? e_func : 0;

    byte tile = 0;
    //
    // Build the borders and the guts
    //
    ushort index = 0;
    for(y = 0; y < h; ++y) {
        for(x = 0; x < w; ++x) {
            if(y == 0 && x == 0)
                tile = T_MNU_BORDER_TL;
            else if(y == 0 && x == w-1)
                tile = T_MNU_BORDER_TR;
            else if(y == h-1 && x == 0)
                tile = T_MNU_BORDER_BL;
            else if(y == h-1 && x == w-1)
                tile = T_MNU_BORDER_BR;
            else if(y == 0)
                tile = T_MNU_BORDER_T;
            else if(x == 0)
                tile = T_MNU_BORDER_L;
            else if(x == w-1)
                tile = T_MNU_BORDER_R;
            else if(y == h-1)
                tile = T_MNU_BORDER_B;
            else
                tile = T_MNU_WHITE;
            new_menu->data[index++] = tile;
        }
    }
    debug("New menu: 0x%p", new_menu);
    return new_menu;
}


/*
===============
menu_new

Clears all the menus in the stack.
===============
*/
void menu_clear() {
    for(byte i = 0; i < num_menus; ++i)
        if(menu_stack[i]) menu_free(menu_stack[i]);
}


/*
===============
menu_free
===============
*/
void menu_free(menu_t* menu) {
    if(menu) {
        debug("Freeing menu: 0x%p", menu);
        memory_free(menu->data);
        memory_free(menu->extra);
        memory_free(menu);
    }
}


/*
===============
menu_render_all

Renders all the menus. Menus on top are drawn first.
===============
*/
void menu_render_all() {
    //draw bottom to top of stack
    for(short i = num_menus-1; i >= 0; --i) {
        if(menu_stack[i])
            menu_render(menu_stack[i]);
    }
}


/*
===============
menu_event_all
===============
*/
void menu_event_all(SDL_Event* evt) {
    for(byte i = 0; i < num_menus; ++i)
        if(menu_stack[i] && menu_stack[i]->event)
            menu_stack[i]->event(menu_stack[i], evt);
}


/*
===============
menu_update_all
===============
*/
void menu_update_all(float gt) {
    for(byte i = 0; i < num_menus; ++i) {
        if(menu_stack[i] && menu_stack[i]->update)
            menu_stack[i]->update(menu_stack[i], gt);
    }
}


/*
===============
menu_render

Renders a menu using absolute screen position.
===============
*/
void menu_render(const menu_t* menu) {
    if(!menu || !menu->data)
        error("NULL menu.");

    texture_t* tex_main = r_find_texture("MAIN");

    //
    // Dereference the members first
    //
    const ushort w = menu->size.x, h = menu->size.y;
    const ushort x = menu->position.x, y = menu->position.y;
    const byte* data = menu->data;

    //
    // Save the camera state
    //
    glPushMatrix(); //
    glLoadIdentity();
    glOrtho(0, window_viewport_width(), window_viewport_height(), 0, 0.0f, 1.0f);

    for(ushort i = 0; i < (w*h); ++i) {
        r_draw_tile(tex_main, data[i]<<3, ((i%w)+x)*TILE_WIDTH, ((i/w)+y)*TILE_HEIGHT);
    }

    //
    // Restore the camera state
    //
    glPopMatrix();
}


/*
===============
menu_add_text

Adds a text string to a menu.
===============
*/
void menu_add_text(const menu_t* menu, ushort x, ushort y, const char* text) {
    if(!menu || !menu->data)
        error("NULL menu.");

    memcpy(&menu->data[(y*menu->size.x)+x], text, strlen(text));
}


/*
===============
menu_dialog_visible

Returns true if there is more than one dialog visible
===============
*/
bool menu_dialog_visible(void) {
    return num_menus >= 2;
}


/*
===============
menu_create_side_panel

Creates the main side panel for Catacomb.
===============
*/
#define SIDE_PANEL_X 24
#define SIDE_PANEL_Y 0
#define SIDE_PANEL_WIDTH 15
#define SIDE_PANEL_HEIGHT 24

menu_t* menu_create_side_panel(update_ptr func) {
    menu_t* panel = menu_new(SIDE_PANEL_X, SIDE_PANEL_Y, SIDE_PANEL_WIDTH, SIDE_PANEL_HEIGHT, func, NULL);

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
        panel->data[(14*panel->size.x)+x] = T_BLACK;
        panel->data[(17*panel->size.x)+x] = T_BLACK;
    }

    return panel;
}

/*
===============
menu_create_simple_box

Creates a simple box with some text.
TODO: Parse newline tokens, remove magic numbers.
===============
*/
menu_t* menu_create_simple_box(const char* msg, update_ptr u_func, event_ptr e_func) {
    ushort width = strlen(msg)+2;
    ushort x = 12-width/2;
    ushort y = 12-3/2;

    menu_t* panel = menu_new(x, y, width, 3, u_func, e_func);
    panel->extra = memory_calloc(sizeof(int32_t)); //current animation

    menu_add_text(panel, 1, 1, msg);

    return panel;
}
