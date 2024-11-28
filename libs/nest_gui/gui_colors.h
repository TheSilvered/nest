#ifndef GUI_COLORS_H
#define GUI_COLORS_H

#include "SDL.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef enum _GUI_Col {
    GUI_COL_BODY_BG,
    GUI_COL_BG,
    GUI_COL_STROKE,
    GUI_COL_HOVER_BG,
    GUI_COL_HOVER_STROKE,
    GUI_COL_HL_BG,
    GUI_COL_HL_STROKE,
    GUI_COL_HL_HOVER_BG,
    GUI_COL_HL_HOVER_STROKE,
    GUI_COL_INPUT_BG,
    GUI_COL_FG,
    GUI_COL_DISABLED_FG,
    GUI_COL_HL_DISABLED_FG,

    GUI_COL_NUM
} GUI_Col;

void GUI_InitColors();

SDL_Color GUI_GetColor(GUI_Col idx);
void GUI_SetColor(GUI_Col idx, SDL_Color color);
void GUI_SetDrawColor(SDL_Renderer *renderer, GUI_Col idx);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GUI_COLORS_H
