#ifndef GUI_EVENT_H
#define GUI_EVENT_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

bool handle_events(App *app);
i32 root_handle_event(SDL_Event *e, Element *el);
i32 default_event_handler(SDL_Event *e, Element *el);

}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_EVENT_H