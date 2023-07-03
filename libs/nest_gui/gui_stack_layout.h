#ifndef GUI_STACK_LAYOUT_H
#define GUI_STACK_LAYOUT_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef enum _GUI_StackDir
{
    GUI_SD_TOP_BOTTOM,
    GUI_SD_LEFT_RIGHT,
    GUI_SD_BOTTOM_TOP,
    GUI_SD_RIGHT_LEFT
}
GUI_StackDir;

typedef enum _GUI_StackAlign
{
    GUI_SA_LEFT_TOP,
    GUI_SA_MIDDLE,
    GUI_SA_BOTTOM_RIGTH
}
GUI_StackAlign;

typedef struct _GUI_StackLayout
{
    GUI_ELEMENT_HEAD;
    GUI_StackDir sd;
    GUI_StackAlign sa;
}
GUI_StackLayout;

bool gui_stack_layout_on_child_added(GUI_StackLayout *sl, usize idx);

GUI_Element *gui_stack_layout_new(GUI_StackDir direction,
                                  GUI_StackAlign alignment,
                                  int x, int y, int w, int h,
                                  GUI_App *app);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_STACK_LAYOUT_H