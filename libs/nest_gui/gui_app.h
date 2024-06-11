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
    struct _GUI_Window *parent;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GUI_Root *root_element;
    Nst_SizedBuffer child_windows; // contains other GUI_Windows
    SDL_Rect clip;
    bool keep_open;
} GUI_Window;

typedef struct _GUI_App {
    bool initialized;
    GUI_Window *window;
    Nst_LList loaded_fonts;
    GUI_Element *focused_element;
} GUI_App;

GUI_Window *GUI_Window_New(GUI_Window *parent, int width, int height);
void GUI_Window_Destroy(GUI_Window *window);

void GUI_Window_SetTitle(GUI_Window *window, const i8 *title);
const i8 *GUI_Window_GetTitle(GUI_Window *window);
void GUI_Window_SetPosition(GUI_Window *window, int x, int y);
void GUI_Window_GetPosition(GUI_Window *window, int *x, int *y);
void GUI_Window_SetSize(GUI_Window *window, int w, int h);
void GUI_Window_GetSize(GUI_Window *window, int *w, int *h);
void GUI_Window_SetResizable(GUI_Window *window, bool resizable);
bool GUI_Window_GetResizable(GUI_Window *window);

bool GUI_Window_IsRunning(GUI_Window *window);
void GUI_Window_Close(GUI_Window *window);

bool GUI_App_Init(GUI_App *app, int window_width, int window_height);
void GUI_App_Quit(GUI_App *app);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_APP_H
