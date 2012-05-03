#ifndef _MENU_H_
#define _MENU_H_

#include "common.h"
#include <SDL.h>

typedef void(*update_ptr)(const void*,float);
typedef void(*event_ptr)(const void*,SDL_Event*);
typedef struct {
    vec2_t      position;
    vec2_t      size;
    byte*       data;
    void*       extra;
    update_ptr  update;
    event_ptr   event;
} menu_t;

menu_t* menu_push(const menu_t* m);
menu_t* menu_pop(void);
menu_t* menu_peek(void);

menu_t* menu_new(ushort x, ushort y, ushort w, ushort h, update_ptr u_func, event_ptr e_func);
void    menu_clear();
void    menu_free(menu_t* menu);

void    menu_render_all(void);
void    menu_event_all(SDL_Event* evt);
void    menu_update_all(float gt);

void    menu_render(const menu_t* menu);
void    menu_add_text(const menu_t* menu, ushort x, ushort y, const char* text);

bool    menu_dialog_visible(void);

//
// Basic menu functions
//
menu_t* menu_create_side_panel(update_ptr func);
menu_t* menu_create_simple_box(const char* msg, update_ptr u_func, event_ptr e_func);

#endif // _MENU_H_
