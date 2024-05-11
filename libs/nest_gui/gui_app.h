#ifndef GUI_APP_H
#define GUI_APP_H

#include "nest.h"
#include <SDL.h>

#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef GUI_Element GUI_Root;
struct _GUI_Window;

typedef struct _GUI_Window {
    GUI_Window *parent;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GUI_Root *root_element;
    Nst_Buffer *child_windows; // contains other GUI_Windows
    SDL_Rect clip;
    bool keep_open;
} GUI_Window;

typedef struct _GUI_App {
    GUI_Window *main_window;
    Nst_Buffer *loaded_fonts;
    GUI_Element *focused_element;
} GUI_App;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_APP_H
