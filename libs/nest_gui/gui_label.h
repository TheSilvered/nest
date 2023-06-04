#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _GUI_Label
{
    GUI_ELEMENT_HEAD;
    Nst_Buffer text;
    TTF_Font *font;
    SDL_Texture *texture;
    SDL_Color color;
    int alignment;
    int texture_render_width;
    int texture_w;
    int texture_h;
    bool clip_text;
    bool auto_height;
}
GUI_Label;

GUI_Element *gui_label_new(Nst_StrObj *text,
                           TTF_Font   *font,
                           SDL_Color  color,
                           int x, int y, int w, int h,
                           GUI_App *app,
                           Nst_OpErr *err);
bool gui_label_update(GUI_Label *l, Nst_OpErr *err);
void gui_label_destroy(GUI_Label *l);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_LABEL_H