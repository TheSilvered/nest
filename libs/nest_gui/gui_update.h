#ifndef GUI_UPDATE_H
#define GUI_UPDATE_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool update_elements(GUI_App *app);
bool tick_elements(GUI_App *app);
bool root_update(GUI_Element *el);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_UPDATE_H
