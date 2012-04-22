#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"

typedef void(*update_ptr)(const void*,float);
typedef struct {
    vec2_t      position;
    vec2_t      size;
    byte*       data;
    update_ptr  update;
} menu_t;

//Puts a new menu on the menu stack.
//menus on top are drawn first.
menu_t* menu_push(const menu_t* m);
menu_t* menu_pop(void);
menu_t* menu_peek(void);

menu_t* menu_new(ushort x, ushort y, ushort w, ushort h, update_ptr func);
void    menu_finish();
void    menu_free(menu_t* menu);

void    menu_render_all(void);
void    menu_update_all(float gt);

void    menu_render(const menu_t* menu);
void    menu_add_text(const menu_t* menu, ushort x, ushort y, const char* text);

//handle creating all basic menus
menu_t* menu_create_side_panel(update_ptr func);
menu_t* menu_create_message_box(const char* msg);

#endif // _MENU_H_
