#ifndef NEST_GUI_H
#define NEST_GUI_H

#include "nest.h"
#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _GUI_App
{
    GUI_Element *root;
    SDL_Window *window;
    SDL_Renderer *renderer;
    bool keep_open;
}
GUI_App;

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
EXPORT void free_lib();

NST_FUNC_SIGN(init_);
NST_FUNC_SIGN(handle_events_);
NST_FUNC_SIGN(set_window_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
