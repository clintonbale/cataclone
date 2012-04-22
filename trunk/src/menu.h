#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

#define SIDE_PANEL_X 24
#define SIDE_PANEL_Y 0
#define SIDE_PANEL_WIDTH 15
#define SIDE_PANEL_HEIGHT 24

#define MAX_MENUS 5

typedef void(*update_ptr)(const void*,float);
typedef struct {
    vec2_t      position;
    vec2_t      size;
    byte*       data;
    bool        visible;
    update_ptr  update;
} menu_t;

//TODO: Make use of active_menus, loop through it in MAIN
menu_t* active_menus[MAX_MENUS];

menu_t* menu_new(ushort x, ushort y, ushort w, ushort h, update_ptr func);
void    menu_free(menu_t* menu);

void    menu_tick(const menu_t* menu, float gt);
void    menu_add_text(const menu_t* menu, ushort x, ushort y, const char* text);

//handle creating all basic menus
menu_t* menu_create_side_panel(update_ptr func);
menu_t* menu_create_message_box(const char* msg);

#endif // _MENU_H_
