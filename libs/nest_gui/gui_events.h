#ifndef GUI_EVENTS_H
#define GUI_EVENTS_H

#include <SDL.h>

#ifndef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _GUI_Event : Sint32 {
    GUI_E_RESIZE,
    GUI_E_IMPORTANT,
    GUI_E_STATE,
    GUI_E_PADDING
} GUI_Event;

#ifndef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_EVENTS_H
