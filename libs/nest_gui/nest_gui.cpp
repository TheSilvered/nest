#include <SDL.h>
#include <SDL_ttf.h>
#include "nest_gui.h"

#define FUNC_COUNT 11

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static GUI_App app;
Nst_TypeObj *element_type;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(init_sdl_and_ttf_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(loop_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_init_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_set_title_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_get_title_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_set_position_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_get_position_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_set_size_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_get_size_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_set_resizable_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(window_get_resizable_, 0);

#if __LINE__ - FUNC_COUNT != 18
#error
#endif

    GUI_InitUtils();
    app.initialized = false;

    element_type = Nst_cont_type_new(
        "GUIElement",
        (Nst_ObjDstr)GUI_Element_Destroy,
        (Nst_ObjTrav)GUI_Element_Traverse);

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    GUI_QuitUtils();
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

Nst_FUNC_SIGN(init_sdl_and_ttf_)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);
    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        GUI_ThrowSDLError();
        return NULL;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(loop_)
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
    }

    GUI_App_Quit(&app);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_init_)
{
    i64 w, h;
    Nst_DEF_EXTRACT("i i", &w, &h);

    if (app.initialized) {
        Nst_set_call_error_c("the app was already initialized");
        return nullptr;
    }

    if (!GUI_App_Init(&app, int(w), int(h)))
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_set_title_)
{
    Nst_StrObj *title;
    Nst_DEF_EXTRACT("s", &title);
    if (not_initialized())
        return nullptr;

    GUI_Window_SetTitle(app.window, (const i8*)(title->value));
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_get_title_)
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
    return Nst_string_new_allocated(title_copy, title_len);
}

Nst_FUNC_SIGN(window_set_position_)
{
    i64 x, y;
    Nst_DEF_EXTRACT("i i", &x, &y);
    if (not_initialized())
        return nullptr;

    GUI_Window_SetPosition(app.window, int(x), int(y));
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_get_position_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    int x, y;
    GUI_Window_GetPosition(app.window, &x, &y);

    return Nst_array_create_c("ii", x, y);
}

Nst_FUNC_SIGN(window_set_size_)
{
    i64 w, h;
    Nst_DEF_EXTRACT("i i", &w, &h);
    if (not_initialized())
        return nullptr;

    GUI_Window_SetSize(app.window, int(w), int(h));
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_get_size_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    int w, h;
    GUI_Window_GetSize(app.window, &w, &h);

    return Nst_array_create_c("ii", w, h);
}

Nst_FUNC_SIGN(window_set_resizable_)
{
    bool resizable;
    Nst_DEF_EXTRACT("y", &resizable);
    if (not_initialized())
        return nullptr;

    GUI_Window_SetResizable(app.window, resizable);
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(window_get_resizable_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (not_initialized())
        return nullptr;

    Nst_RETURN_BOOL(GUI_Window_GetResizable(app.window));
}
