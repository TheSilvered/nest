#ifndef GUI_EVENT_H
#define GUI_EVENT_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool handle_events(GUI_App *app, Nst_OpErr *err);
i32 root_handle_event(SDL_Event *e, GUI_Element *el, Nst_OpErr *err);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_EVENT_H