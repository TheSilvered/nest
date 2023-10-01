#ifndef GUI_LABEL_H
#define GUI_LABEL_H

#include <SDL_ttf.h>
#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

typedef struct _Label {
    GUI_ELEMENT_HEAD;
    Nst_Buffer text;
    FontObj *font;
    SDL_Texture *texture;
    SDL_Color color;
    int alignment;
    int texture_render_width;
    bool auto_height;
} Label;

Element *label_new(Nst_StrObj *text, FontObj *font, SDL_Color color,
                   int x, int y, int w, int h, App *app);
bool label_update(Label *l);
void label_destroy(Label *l);

void label_change_color(Label *l, SDL_Color new_color);
void label_append_text(Label *l, Nst_StrObj *str, bool change_size);
void label_append_c_text(Label *l, i8 *text, bool change_size);
void label_set_text(Label *l, Nst_StrObj *str, bool change_size);
void label_set_c_text(Label *l, i8 *text, bool change_size);
void label_set_font(Label *l, FontObj *font, bool change_size);

}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_LABEL_H