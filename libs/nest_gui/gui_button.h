#ifndef GUI_BUTTON_H
#define GUI_BUTTON_H

#include "gui_label.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum _GUI_ClickState {
    GUI_CS_IDLE,
    GUI_CS_HOVER,
    GUI_CS_HOVER_IDLE,
    GUI_CS_CLICKED
} GUI_ClickState;

struct _GUI_Button;
typedef bool (*OnStateChange)(struct _GUI_Button *);

typedef struct _GUI_Button {
    GUI_ELEMENT_HEAD;
    GUI_Label *text;
    GUI_ClickState cs;
    i32 number_of_states;
    i32 current_state;
    bool disabled;
    SDL_Texture *textures[5];
    OnStateChange func;
    Nst_FuncObj *nest_func;
    SDL_Rect clickable_area;
} GUI_Button;

GUI_Element *gui_button_new(GUI_Label *text, GUI_App *app);
bool gui_button_update(GUI_Button *b);
i32 gui_button_handle_event(SDL_Event *e, GUI_Button *b);
void gui_button_destroy(GUI_Button *b);
bool gui_button_call_nest_func(GUI_Button *b);

#ifdef __cplusplus
}
#endif
#endif // !GUI_BUTTON_H