#ifndef NEST_GUI_H
#define NEST_GUI_H

#include "nest.h"
#include "gui_app.h"
#include "gui_draw.h"
#include "gui_element.h"
#include "gui_utils.h"
#include "gui_obj_types.h"
#include "gui_update.h"
#include "gui_colors.h"

#ifdef Nst_WIN
#pragma warning(disable: 4995)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP Nst_Declr *NstC lib_init();
NstEXP void NstC lib_quit();

Nst_Obj *NstC init_sdl_and_ttf_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC loop_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_init_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_set_title_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_get_title_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_set_position_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_get_position_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_set_size_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_get_size_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_set_resizable_(usize arg_num, Nst_Obj **args);
Nst_Obj *NstC window_get_resizable_(usize arg_num, Nst_Obj **args);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
