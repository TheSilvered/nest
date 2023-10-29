#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_label.h"

#ifdef __cplusplus
extern "C" {
#endif

namespace GUI {

typedef enum _ClickState {
    GUI_CS_IDLE,
    GUI_CS_HOVER,
    GUI_CS_HOVER_IDLE,
    GUI_CS_CLICKED
} ClickState;

struct _Button;
typedef bool (*OnStateChange)(struct _Button *);

typedef struct _Button {
    GUI_ELEMENT_HEAD;
    Label *text;
    ClickState cs;
    i32 number_of_states;
    i32 current_state;
    bool disabled;
    SDL_Texture *textures[5];
    OnStateChange func;
    Nst_FuncObj *nest_func;
    SDL_Rect clickable_area;
} Button;

Element *button_new(Label *text, App *app);
bool button_update(Button *b);
i32 button_handle_event(SDL_Event *e, Button *b);
void button_destroy(Button *b);
bool button_call_nest_func(Button *b);

}

#ifdef __cplusplus
}
#endif
#endif // !GUI_BUTTON_H