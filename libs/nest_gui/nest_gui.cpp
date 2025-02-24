#include <SDL.h>
#include <SDL_ttf.h>
#include "nest_gui.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(app_run_, 0),
    Nst_FUNCDECLR(app_is_running_, 0),
    Nst_FUNCDECLR(app_close_, 0),
    Nst_FUNCDECLR(window_init_, 2),
    Nst_FUNCDECLR(window_set_title_, 1),
    Nst_FUNCDECLR(window_get_title_, 0),
    Nst_FUNCDECLR(window_set_position_, 2),
    Nst_FUNCDECLR(window_get_position_, 0),
    Nst_FUNCDECLR(window_set_size_, 2),
    Nst_FUNCDECLR(window_get_size_, 0),
    Nst_FUNCDECLR(window_set_resizable_, 1),
    Nst_FUNCDECLR(window_get_resizable_, 0),
    Nst_DECLR_END
};
static GUI_App app;

Nst_Declr *lib_init()
{
    if (!GUI_InitUtils()) {
        return nullptr;
    }
    if (!GUI_InitTypes()) {
        GUI_QuitUtils();
        return nullptr;
    }
    GUI_InitColors();

    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        GUI_ThrowSDLError();
        return nullptr;
    }

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

    app.initialized = false;
    return obj_list_;
}

void lib_quit()
{
    GUI_QuitUtils();
    GUI_QuitTypes();
    TTF_Quit();
    SDL_Quit();
}

static bool not_initialized() {
    if (!app.initialized) {
        Nst_error_setc_call("the app is not initialized");
        return true;
    }
    return false;
}

Nst_Obj *NstC app_run_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);

    if (not_initialized())
        return nullptr;

    if (!GUI_App_Run(&app))
        return nullptr;

    return Nst_null_ref();
}

Nst_Obj *NstC app_is_running_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);

    Nst_RETURN_BOOL(GUI_App_IsRunning(&app));
}

Nst_Obj *NstC app_close_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);

    if (not_initialized())
        return nullptr;

    GUI_App_Close(&app);
    return Nst_null_ref();
}

Nst_Obj *NstC window_init_(usize arg_num, Nst_Obj **args)
{
    i64 w, h;
    if (!Nst_extract_args("i i", arg_num, args, &w, &h))
        return nullptr;

    if (app.initialized) {
        Nst_error_setc_call("the app was already initialized");
        return nullptr;
    }

    if (!GUI_App_Init(&app, int(w), int(h)))
        return nullptr;
    return Nst_null_ref();
}

Nst_Obj *NstC window_set_title_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *title;
    if (!Nst_extract_args("s", arg_num, args, &title))
        return nullptr;
    if (not_initialized())
        return nullptr;

    GUI_Window_SetTitle(app.window, (const i8*)Nst_str_value(title));
    return Nst_null_ref();
}

Nst_Obj *NstC window_get_title_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    const i8 *title = GUI_Window_GetTitle(app.window);
    usize title_len = strlen(title);
    i8 *title_copy = (i8 *)Nst_calloc(1, title_len + 1, (void *)title);
    if (title_copy == nullptr)
        return nullptr;
    return Nst_str_new_allocated(title_copy, title_len);
}

Nst_Obj *NstC window_set_position_(usize arg_num, Nst_Obj **args)
{
    i64 x, y;
    if (!Nst_extract_args("i i", arg_num, args, &x, &y))
        return nullptr;
    if (not_initialized())
        return nullptr;

    GUI_Window_SetPosition(app.window, int(x), int(y));
    return Nst_null_ref();
}

Nst_Obj *NstC window_get_position_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    int x, y;
    GUI_Window_GetPosition(app.window, &x, &y);

    return Nst_array_create_c("ii", x, y);
}

Nst_Obj *NstC window_set_size_(usize arg_num, Nst_Obj **args)
{
    i64 w, h;
    if (!Nst_extract_args("i i", arg_num, args, &w, &h))
        return nullptr;
    if (not_initialized())
        return nullptr;

    GUI_Window_SetSize(app.window, int(w), int(h));
    return Nst_null_ref();
}

Nst_Obj *NstC window_get_size_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    int w, h;
    GUI_Window_GetSize(app.window, &w, &h);

    return Nst_array_create_c("ii", w, h);
}

Nst_Obj *NstC window_set_resizable_(usize arg_num, Nst_Obj **args)
{
    bool resizable;
    if (!Nst_extract_args("y", arg_num, args, &resizable))
        return nullptr;
    if (not_initialized())
        return nullptr;

    GUI_Window_SetResizable(app.window, resizable);
    return Nst_null_ref();
}

Nst_Obj *NstC window_get_resizable_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    Nst_RETURN_BOOL(GUI_Window_GetResizable(app.window));
}
