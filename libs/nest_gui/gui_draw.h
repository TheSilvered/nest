#ifndef GUI_DRAW_H
#define GUI_DRAW_H

#include "nest.h"
#include <SDL.h>

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void GUI_DrawRect(SDL_Renderer *renderer, SDL_Rect *rect);
SDL_Texture *GUI_DrawRoundRect(SDL_Renderer *renderer, SDL_Rect rect,
                               int rtl, int rtr, int rbl, int rbr,
                               u8 r, u8 g, u8 b, u8 a);
SDL_Texture *GUI_DrawRoundBorderRect(SDL_Renderer *renderer, SDL_Rect rect,
                                     int border_thickness,
                                     int rtl, int rtr, int rbl, int rbr,
                                     u8 r_i, u8 g_i, u8 b_i, u8 a_i,
                                     u8 r_b, u8 g_b, u8 b_b, u8 a_b);
void GUI_DrawTexture(SDL_Renderer *renderer, int x, int y,
                     SDL_Texture *texture, SDL_Rect *clip);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_DRAW_H
