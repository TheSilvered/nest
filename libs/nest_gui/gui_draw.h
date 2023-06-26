#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void draw_rect(SDL_Renderer *renderer, SDL_Rect *rect);
SDL_Texture *draw_round_rect(SDL_Rect *rect,
                             int rtl, int rtr, int rbl, int rbr,
                             u8 r, u8 g, u8 b, u8 a);
void draw_texture(GUI_App *app, int x, int y, SDL_Texture *texture, SDL_Rect *clip);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_DRAW_H
