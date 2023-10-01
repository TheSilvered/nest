#ifndef GUI_UPDATE_H
#define GUI_UPDATE_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

bool update_elements(App *app);
bool tick_elements(App *app);
bool root_update(Element *el);

}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_UPDATE_H
