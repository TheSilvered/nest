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
    GUI_Element *focused_element;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect clip_window;
    bool keep_open;
    bool show_bounds;

    TTF_Font *regular_small;
    TTF_Font *italic_small;
    TTF_Font *bold_small;
    TTF_Font *bold_italic_small;

    TTF_Font *regular_medium;
    TTF_Font *italic_medium;
    TTF_Font *bold_medium;
    TTF_Font *bold_italic_medium;

    TTF_Font *regular_big;
    TTF_Font *italic_big;
    TTF_Font *bold_big;
    TTF_Font *bold_italic_big;

    SDL_Color fg_color;
    SDL_Color bg_color;
    SDL_Color fg_dimmed_color;
    SDL_Color bg_light_color;
}
GUI_App;

EXPORT bool lib_init();
EXPORT Nst_DeclrList *get_func_ptrs();
EXPORT void free_lib();

void set_sdl_error(Nst_OpErr *err);
int imin(int n1, int n2);
int imax(int n1, int n2);
void set_focused_element(GUI_Element *el);
GUI_Element *get_focused_element();
void remove_focused_element();

NST_FUNC_SIGN(init_);
NST_FUNC_SIGN(loop_);
NST_FUNC_SIGN(set_window_);
NST_FUNC_SIGN(label_);
NST_FUNC_SIGN(button_);
NST_FUNC_SIGN(stack_layout_);
NST_FUNC_SIGN(set_position_);
NST_FUNC_SIGN(set_rel_position_);
NST_FUNC_SIGN(set_size_);
NST_FUNC_SIGN(set_rel_size_);
NST_FUNC_SIGN(set_margins_);
NST_FUNC_SIGN(set_padding_);
NST_FUNC_SIGN(show_overflow_);
NST_FUNC_SIGN(auto_height_);
NST_FUNC_SIGN(add_child_);
NST_FUNC_SIGN(get_root_);
NST_FUNC_SIGN(set_func_);
NST_FUNC_SIGN(_debug_view_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
