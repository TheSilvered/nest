#ifndef GUI_UPDATE_H
#define GUI_UPDATE_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool update_elements(GUI_App *app, Nst_OpErr *err);
bool tick_elements(GUI_App *app, Nst_OpErr *err);
bool root_update(GUI_Element *el, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_UPDATE_H