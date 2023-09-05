#ifndef NEST_GUI_H
#define NEST_GUI_H

#include <SDL_ttf.h>
#include "nest.h"
#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _GUI_FontObj {
    Nst_OBJ_HEAD;
    TTF_Font *font;
} GUI_FontObj;

typedef struct _GUI_App {
    GUI_Element *root;
    GUI_Element *focused_element;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect clip_window;
    bool keep_open;
    bool show_bounds;
    bool element_reached;

    SDL_Color fg_color;
    SDL_Color fg_color2;
    SDL_Color fg_color3;
    SDL_Color bg_color3;
    SDL_Color bg_color2;
    SDL_Color bg_color;
    SDL_Color bg_dark;

    GUI_FontObj *builtin_fonts[48];
    Nst_Obj *opened_fonts;
} GUI_App;

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_Obj *gui_font_new(TTF_Font *font);
void set_sdl_error();
int imin(int n1, int n2);
int imax(int n1, int n2);
void set_focused_element(GUI_Element *el);
GUI_Element *get_focused_element();
void remove_focused_element();

Nst_FUNC_SIGN(init_sdl_and_ttf_);
Nst_FUNC_SIGN(loop_);
Nst_FUNC_SIGN(label_);
Nst_FUNC_SIGN(button_);
Nst_FUNC_SIGN(stack_layout_);
Nst_FUNC_SIGN(set_window_);
Nst_FUNC_SIGN(set_pos_);
Nst_FUNC_SIGN(set_rel_pos_);
Nst_FUNC_SIGN(get_pos_);
Nst_FUNC_SIGN(set_size_);
Nst_FUNC_SIGN(set_rel_size_);
Nst_FUNC_SIGN(get_size_);
Nst_FUNC_SIGN(set_overflow_);
Nst_FUNC_SIGN(get_overflow_);
Nst_FUNC_SIGN(set_margins_);
Nst_FUNC_SIGN(get_margins_);
Nst_FUNC_SIGN(set_padding_);
Nst_FUNC_SIGN(get_padding_);
Nst_FUNC_SIGN(show_);
Nst_FUNC_SIGN(hide_);
Nst_FUNC_SIGN(is_hidden_);
Nst_FUNC_SIGN(set_auto_height_);
Nst_FUNC_SIGN(get_auto_height_);
Nst_FUNC_SIGN(set_font_);
Nst_FUNC_SIGN(get_font_);
Nst_FUNC_SIGN(add_child_);
Nst_FUNC_SIGN(remove_child_);
Nst_FUNC_SIGN(get_root_);
Nst_FUNC_SIGN(set_func_);
Nst_FUNC_SIGN(get_builtin_font_);
Nst_FUNC_SIGN(open_font_);
Nst_FUNC_SIGN(_debug_view_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
