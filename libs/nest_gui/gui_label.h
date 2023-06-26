#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include <SDL_ttf.h>
#include "nest_gui.h"
#define NO_ALIGN_IMPL
#ifndef TTF_WRAPPED_ALIGN_LEFT

#define TTF_WRAPPED_ALIGN_LEFT 0
#define TTF_WRAPPED_ALIGN_CENTER 1
#define TTF_WRAPPED_ALIGN_RIGHT 2

SDL_Surface *render_wrapped_aligned(TTF_Font *font,
                                    i8       *text,
                                    SDL_Color color,
                                    int       wrap_lenght,
                                    int       alignment);

#endif

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