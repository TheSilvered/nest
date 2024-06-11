#include <SDL.h>
#include <cstring>
#include "gui_utils.h"

Nst_StrObj *sdl_error_name = NULL;

bool GUI_InitUtils(void)
{
    sdl_error_name = STR(Nst_string_new_c_raw("SDL Error", false));
    if (sdl_error_name == NULL)
        return false;
    return true;
}

void GUI_QuitUtils(void)
{
    if (sdl_error_name != NULL) {
        Nst_dec_ref(sdl_error_name);
        sdl_error_name = NULL;
    }
}

void GUI_ThrowSDLError(void)
{
    const i8 *error = SDL_GetError();
    usize error_len = strlen(error);
    i8 *error_copy = (i8 *)Nst_calloc(1, error_len + 1, (void *)error);
    if (error_copy == NULL)
        return;

    Nst_Obj *error_obj = Nst_string_new_allocated(error_copy, error_len);
    if (error_obj == NULL)
        return;
    Nst_set_error(Nst_inc_ref(sdl_error_name), error_obj);
}

int GUI_MaxInt2(int a, int b)
{
    return a > b ? a : b;
}

int GUI_MinInt2(int a, int b)
{
    return a < b ? a : b;
}
