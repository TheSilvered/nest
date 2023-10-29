#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include "nest_gui.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void draw_rect(SDL_Renderer *renderer, SDL_Rect *rect);
SDL_Texture *draw_round_rect(SDL_Renderer *renderer, SDL_Rect rect,
                             int rtl, int rtr, int rbl, int rbr,
                             u8 r, u8 g, u8 b, u8 a);
SDL_Texture *draw_round_border_rect(SDL_Renderer *renderer, SDL_Rect rect,
                                    int border_thickness,
                                    int rtl, int rtr, int rbl, int rbr,
                                    u8 r_i, u8 g_i, u8 b_i, u8 a_i,
                                    u8 r_b, u8 g_b, u8 b_b, u8 a_b);
void draw_texture(GUI::App *app, int x, int y, SDL_Texture *texture,
                  SDL_Rect *clip);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_DRAW_H
