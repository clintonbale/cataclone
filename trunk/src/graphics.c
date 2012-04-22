#include "graphics.h"
#include "common.h"

#include <SDL.h>
#include <SDL_opengl.h>

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 200
#define DEFAULT_VSYNC (1)
#define DEFAULT_MULTISAMPLES 0
#define DEFAULT_MULTISAMPLESBUFFS 0
#define DEFAULT_VIDEOMODE GFX_MODE_EGA

static SDL_Surface* screen;

static uint window_height;
static uint window_width;

static int window_flags;
static bool window_vsync;
static int window_multisamples;
static int window_multisamplesbuffs;

static graphics_mode_t graphics_mode;

const SDL_Surface* graphics_get_screen() {
    return screen;
}

void graphics_viewport_start() {
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

void graphics_init() {
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

    graphics_mode = DEFAULT_VIDEOMODE;

    graphics_viewport_set_title("Catacomb");
    graphics_viewport_start();
}

void graphics_finish() {
    SDL_FreeSurface(screen);
}

graphics_mode_t graphics_get_mode(void) {
    return graphics_mode;
}

void graphics_set_mode(graphics_mode_t newMode) {
    graphics_mode = newMode;
    //TODO: Reload tiles?
}

void graphics_set_multisamples(int multisamples) {
    window_multisamples = multisamples > 0 ? 1 : 0;
}

int graphics_get_multisamples(void) {
    return window_multisamples;
}

void graphics_viewport_set_title(const char* title) {
    SDL_WM_SetCaption(title, NULL);
}

uint graphics_viewport_height() {
    return window_height;
}

uint graphics_viewport_width() {
    return window_width;
}
