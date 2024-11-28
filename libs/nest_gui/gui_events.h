#ifndef GUI_EVENTS_H
#define GUI_EVENTS_H

#include <SDL.h>
#include "gui_app.h"

#ifndef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _GUI_Event : Sint32 {
    GUI_E_RESIZE,
    GUI_E_IMPORTANT,
    GUI_E_STATE,
    GUI_E_PADDING
} GUI_UserEvent;

void GUI_Event_PushUserEvent(GUI_Element *element, GUI_UserEvent event_id);
void GUI_Event_PushUserEventEx(GUI_Element *element, GUI_UserEvent event_id,
                               Uint32 window_id, void *data2);

bool GUI_App_HandleEvent(GUI_App *app, SDL_Event *event);

#ifndef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_EVENTS_H
