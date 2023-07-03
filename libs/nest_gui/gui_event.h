#ifndef GUI_EVENT_H
#define GUI_EVENT_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool handle_events(GUI_App *app);
i32 root_handle_event(SDL_Event *e, GUI_Element *el);
i32 default_event_handler(SDL_Event *e, GUI_Element *el);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_EVENT_H