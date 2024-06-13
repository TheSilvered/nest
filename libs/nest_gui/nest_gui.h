#ifndef NEST_GUI_H
#define NEST_GUI_H

#include "nest.h"
#include "gui_app.h"
#include "gui_draw.h"
#include "gui_element.h"
#include "gui_utils.h"
#include "gui_obj_types.h"

#ifdef Nst_WIN
#pragma warning(disable: 4995)
#endif

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP bool NstC lib_init();
NstEXP Nst_DeclrList *NstC get_func_ptrs();
NstEXP void NstC free_lib();

Nst_FUNC_SIGN(init_sdl_and_ttf_);
Nst_FUNC_SIGN(loop_);
Nst_FUNC_SIGN(window_init_);
Nst_FUNC_SIGN(window_set_title_);
Nst_FUNC_SIGN(window_get_title_);
Nst_FUNC_SIGN(window_set_position_);
Nst_FUNC_SIGN(window_get_position_);
Nst_FUNC_SIGN(window_set_size_);
Nst_FUNC_SIGN(window_get_size_);
Nst_FUNC_SIGN(window_set_resizable_);
Nst_FUNC_SIGN(window_get_resizable_);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NEST_GUI_H
