#ifndef GUI_STACK_LAYOUT_H
#define GUI_STACK_LAYOUT_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

typedef enum _StackDir {
    SD_TOP_BOTTOM,
    SD_LEFT_RIGHT,
    SD_BOTTOM_TOP,
    SD_RIGHT_LEFT
} StackDir;

typedef enum _StackAlign {
    SA_LEFT_TOP,
    SA_MIDDLE,
    SA_BOTTOM_RIGHT
} StackAlign;

typedef struct _StackLayout {
    GUI_ELEMENT_HEAD;
    StackDir sd;
    StackAlign sa;
} StackLayout;

i32 stack_layout_handle_event(SDL_Event *e, StackLayout *sl);
bool stack_layout_update(StackLayout *sl);

Element *stack_layout_new(StackDir direction, StackAlign alignment,
                          int x, int y, int w, int h, App *app);

}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_STACK_LAYOUT_H