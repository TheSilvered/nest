#include "gui_element.h"
#include "gui_events.h"

void GUI_Event_PushUserEvent(GUI_Element *element, GUI_UserEvent event_id)
{
    GUI_Event_PushUserEventEx(element, event_id, 0, nullptr);
}

void GUI_Event_PushUserEventEx(GUI_Element *element, GUI_UserEvent event_id,
                               Uint32 window_id, void *data2)
{
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.windowID = window_id;
    event.user.code = event_id;
    event.user.timestamp = SDL_GetTicks();
    event.user.data1 = (void *)element;
    event.user.data2 = data2;
    SDL_PushEvent(&event);
}

bool GUI_App_HandleEvent(GUI_App *app, SDL_Event *event)
{


    if (event->type == SDL_QUIT)
        GUI_App_Close(app);
    return true;
}
