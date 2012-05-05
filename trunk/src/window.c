#include "window.h"
#include "common.h"

#include <SDL.h>
#include <SDL_opengl.h>

#define DEFAULT_WIDTH               320
#define DEFAULT_HEIGHT              200
#define DEFAULT_VSYNC               0
#define DEFAULT_MULTISAMPLES        0
#define DEFAULT_MULTISAMPLESBUFFS   0

static SDL_Surface* screen;

static uint window_height;
static uint window_width;

static int window_flags;
static bool window_vsync;
static int window_multisamples;
static int window_multisamplesbuffs;


/*
===============
window_start

Creates the screen and sets up the window.
===============
*/
static void window_start() {
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, window_vsync);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, window_multisamples);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, window_multisamplesbuffs);

    screen = SDL_SetVideoMode(window_width, window_height, 32, window_flags );
    if(!screen) {
        error("Could not create SDL window: %s", SDL_GetError());
    }

    glViewport(0,0, window_width, window_height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0f, window_width, window_height, 0.0f, 0.0f, 1.0f);
}


/*
===============
window_init

Initializes parameters, sets tiltle, and inits SDL.
===============
*/
void window_init() {
    int err = SDL_Init(SDL_INIT_VIDEO);
    if(err == -1) {
        error("Cannot initialize SDL video!");
    }

    window_height = DEFAULT_HEIGHT;
    window_width = DEFAULT_WIDTH;
    window_flags = SDL_OPENGL;
    window_vsync = DEFAULT_VSYNC;
    window_multisamples = DEFAULT_MULTISAMPLES;
    window_multisamplesbuffs = DEFAULT_MULTISAMPLESBUFFS;

    window_set_title("Catacomb");
    window_start();
}


/*
===============
window_finish
===============
*/
void window_finish() {
    SDL_FreeSurface(screen);
}


/*
===============
window_set_title
===============
*/
void window_set_title(const char* title) {
    SDL_WM_SetCaption(title, NULL);
}


/*
===============
window_viewport_height
===============
*/
uint window_viewport_height() {
    return window_height;
}


/*
===============
window_viewport_width
===============
*/
uint window_viewport_width() {
    return window_width;
}
