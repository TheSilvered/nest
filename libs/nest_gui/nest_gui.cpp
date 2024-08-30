#include <SDL.h>
#include <SDL_ttf.h>
#include "nest_gui.h"

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(init_sdl_and_ttf_, 0),
    Nst_FUNCDECLR(loop_, 0),
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
        Nst_set_call_error_c("the app is not initialized");
        return true;
    }
    return false;
}

Nst_Obj *NstC init_sdl_and_ttf_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);
    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        GUI_ThrowSDLError();
        return NULL;
    }
    Nst_RETURN_NULL;
}

Nst_Obj *NstC loop_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);

    if (not_initialized())
        return nullptr;

    while (GUI_Window_IsRunning(app.window)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                GUI_Window_Close(app.window);
        }
        GUI_App_Update(&app);
    }

    GUI_App_Quit(&app);

    Nst_RETURN_NULL;
}

Nst_Obj *NstC window_init_(usize arg_num, Nst_Obj **args)
{
    i64 w, h;
    if (!Nst_extract_args("i i", arg_num, args, &w, &h))
        return nullptr;

    if (app.initialized) {
        Nst_set_call_error_c("the app was already initialized");
        return nullptr;
    }

    if (!GUI_App_Init(&app, int(w), int(h)))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC window_set_title_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *title;
    if (!Nst_extract_args("s", arg_num, args, &title))
        return nullptr;
    if (not_initialized())
        return nullptr;

    GUI_Window_SetTitle(app.window, (const i8*)(title->value));
    Nst_RETURN_NULL;
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
    Nst_RETURN_NULL;
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
    Nst_RETURN_NULL;
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
    Nst_RETURN_NULL;
}

Nst_Obj *NstC window_get_resizable_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    Nst_RETURN_BOOL(GUI_Window_GetResizable(app.window));
}
