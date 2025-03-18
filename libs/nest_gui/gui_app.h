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
struct _GUI_App;

typedef struct _GUI_Window {
    struct _GUI_Window *parent;
    struct _GUI_App *app;
    SDL_Window *window;
    SDL_Renderer *renderer;
    GUI_Root *root_element;
    Nst_DynArray child_windows; // contains other GUI_Windows
    bool keep_open;
} GUI_Window;

typedef struct _GUI_App {
    bool initialized;
    GUI_Window *window;
    Nst_LList loaded_fonts;
    GUI_Element *focused_element;
} GUI_App;

GUI_Window *GUI_Window_New(GUI_Window *parent, GUI_App *app,
                           int width, int height);
void GUI_Window_Destroy(GUI_Window *window);

void GUI_Window_SetTitle(GUI_Window *window, const char *title);
const char *GUI_Window_GetTitle(GUI_Window *window);
void GUI_Window_SetPosition(GUI_Window *window, int x, int y);
void GUI_Window_GetPosition(GUI_Window *window, int *x, int *y);
void GUI_Window_SetSize(GUI_Window *window, int w, int h);
void GUI_Window_GetSize(GUI_Window *window, int *w, int *h);
void GUI_Window_SetResizable(GUI_Window *window, bool resizable);
bool GUI_Window_GetResizable(GUI_Window *window);
SDL_Rect GUI_Window_GetClipRect(GUI_Window *window);

bool GUI_Window_IsRunning(GUI_Window *window);
void GUI_Window_Close(GUI_Window *window);

bool GUI_App_Init(GUI_App *app, int window_width, int window_height);
bool GUI_App_Run(GUI_App *app);
bool GUI_App_IsRunning(GUI_App *app);
void GUI_App_Close(GUI_App *app);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_APP_H
