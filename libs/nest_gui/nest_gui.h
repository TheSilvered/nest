#ifndef NEST_GUI_H
#define NEST_GUI_H

#include <SDL_ttf.h>
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
    bool show_bounds;

    TTF_Font *regular_small;
    TTF_Font *italic_small;
    TTF_Font *bold_small;
    TTF_Font *regular_medium;
    TTF_Font *italic_medium;
    TTF_Font *bold_medium;
    TTF_Font *regular_big;
    TTF_Font *italic_big;
    TTF_Font *bold_big;

    SDL_Color fg_color;
    SDL_Color bg_color;
    SDL_Color fg_dimmed_color;
    SDL_Color bg_light_color;
}
GUI_App;

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
EXPORT void free_lib();

NST_FUNC_SIGN(init_);
NST_FUNC_SIGN(loop_);
NST_FUNC_SIGN(set_window_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
