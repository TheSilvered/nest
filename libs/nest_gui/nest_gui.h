#ifndef NEST_GUI_H
#define NEST_GUI_H

#include <SDL_ttf.h>
#include "nest.h"
#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

typedef struct _App {
    Element *root;
    Element *focused_element;
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

    FontObj *builtin_fonts[48];
    Nst_Obj *opened_fonts;
} App;

void set_sdl_error();
int imin(int n1, int n2);
int imax(int n1, int n2);
void set_focused_element(Element *el);
Element *get_focused_element();
void remove_focused_element();
App *get_global_app();

}

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

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
Nst_FUNC_SIGN(get_root_);
Nst_FUNC_SIGN(set_func_);
Nst_FUNC_SIGN(get_builtin_font_);
Nst_FUNC_SIGN(open_font_);
Nst_FUNC_SIGN(match_x_);
Nst_FUNC_SIGN(match_y_);
Nst_FUNC_SIGN(match_pos_);
Nst_FUNC_SIGN(match_w_);
Nst_FUNC_SIGN(match_h_);
Nst_FUNC_SIGN(match_size_);
Nst_FUNC_SIGN(perc_pos_);
Nst_FUNC_SIGN(diff_pos_);
Nst_FUNC_SIGN(perc_size_);
Nst_FUNC_SIGN(diff_size_);
Nst_FUNC_SIGN(min_x_);
Nst_FUNC_SIGN(min_y_);
Nst_FUNC_SIGN(min_pos_);
Nst_FUNC_SIGN(max_x_);
Nst_FUNC_SIGN(max_y_);
Nst_FUNC_SIGN(max_pos_);
Nst_FUNC_SIGN(min_w_);
Nst_FUNC_SIGN(min_h_);
Nst_FUNC_SIGN(min_size_);
Nst_FUNC_SIGN(max_w_);
Nst_FUNC_SIGN(max_h_);
Nst_FUNC_SIGN(max_size_);
Nst_FUNC_SIGN(fill_w_);
Nst_FUNC_SIGN(fill_h_);
Nst_FUNC_SIGN(get_text_);
Nst_FUNC_SIGN(set_text_);
Nst_FUNC_SIGN(_debug_view_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
