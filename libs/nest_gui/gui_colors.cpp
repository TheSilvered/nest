#include "gui_colors.h"

static SDL_Color colors[GUI_COL_NUM];

void GUI_InitColors()
{
    colors[GUI_COL_BODY_BG]         = {  32,  32,  32, 255 };
    colors[GUI_COL_BG]              = {  47,  47,  47, 255 };
    colors[GUI_COL_STROKE]          = {  67,  67,  67, 255 };
    colors[GUI_COL_HOVER_BG]        = {  57,  57,  57, 255 };
    colors[GUI_COL_HOVER_STROKE]    = {  87,  87,  87, 255 };
    colors[GUI_COL_HL_BG]           = {  19,  66, 133, 255 };
    colors[GUI_COL_HL_STROKE]       = {  83, 138, 218, 255 };
    colors[GUI_COL_HL_HOVER_BG]     = {  39,  88, 157, 255 };
    colors[GUI_COL_HL_HOVER_STROKE] = {  39,  88, 157, 255 };
    colors[GUI_COL_INPUT_BG]        = {  21,  21,  21, 255 };
    colors[GUI_COL_FG]              = { 255, 255, 255, 255 };
    colors[GUI_COL_DISABLED_FG]     = { 147, 147, 174, 255 };
    colors[GUI_COL_HL_DISABLED_FG]  = { 144, 170, 205, 255 };
}

SDL_Color GUI_GetColor(GUI_Col idx)
{
    return colors[idx];
}

void GUI_SetColor(GUI_Col idx, SDL_Color color)
{
    colors[idx] = color;
}

void GUI_SetDrawColor(SDL_Renderer *renderer, GUI_Col idx)
{
    SDL_SetRenderDrawColor(
        renderer,
        colors[idx].r,
        colors[idx].g,
        colors[idx].b,
        colors[idx].a);
}
