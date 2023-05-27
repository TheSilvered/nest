#include <SDL.h>
#include <SDL_ttf.h>
#include <cstring>
#include "nest_gui.h"
#include "gui_event.h"
#include "gui_update.h"

#define FUNC_COUNT 3

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *sdl_error_str;
static GUI_App app;
Nst_TypeObj *gui_element_type;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

    func_list_[idx++] = NST_MAKE_FUNCDECLR(init_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(loop_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(set_window_, 6);

#if __LINE__ - FUNC_COUNT != 23
#error
#endif

    sdl_error_str = STR(nst_string_new_c_raw("SDL Error", false, nullptr));
    gui_element_type = nst_type_new("GUI Element", 11, nullptr);

    app.root = nullptr;
    app.window = nullptr;
    app.renderer = nullptr;

    app.keep_open = false;
    app.show_bounds = false;

    app.regular_small = nullptr;
    app.italic_small = nullptr;
    app.bold_small = nullptr;
    app.regular_medium = nullptr;
    app.italic_medium = nullptr;
    app.bold_medium = nullptr;
    app.regular_big = nullptr;
    app.italic_big = nullptr;
    app.bold_big = nullptr;

    app.bg_color = { 0, 0, 0, 255 };
    app.fg_color = { 255, 255, 255, 255 };
    app.bg_light_color = { 60, 60, 60, 255 };
    app.fg_dimmed_color = { 160, 160, 160, 255 };

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    nst_dec_ref(sdl_error_str);
    nst_dec_ref(gui_element_type);

    if ( app.window != nullptr )
    {
        SDL_DestroyWindow(app.window);
        SDL_DestroyRenderer(app.renderer);
        nst_dec_ref(app.root);
    }

    TTF_Quit();
    SDL_Quit();
}

void set_sdl_error(Nst_OpErr *err)
{
    const i8 *sdl_error = SDL_GetError();
    usize len = strlen(sdl_error);
    i8 *new_error = (i8 *)nst_calloc(
        1,
        sizeof(i8) * (len + 1),
        (void *)sdl_error, // copies the string if the allocation succeded
        err);
    if ( new_error == nullptr )
    {
        return;
    }
    Nst_StrObj *msg = STR(nst_string_new(new_error, len, true, err));
    if ( msg == nullptr )
    {
        nst_free(new_error);
        return;
    }
    NST_SET_ERROR(sdl_error_str, msg);
}

NST_FUNC_SIGN(init_)
{
    if ( SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING )
    {
        NST_RETURN_NULL;
    }

    if ( SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init() )
    {
        set_sdl_error(err);
        return nullptr;
    }
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(loop_)
{
    while ( app.keep_open )
    {
        if ( !handle_events(&app, err) )
        {
            return nullptr;
        }

        update_elements(&app, err);
        tick_elements(&app, err);

        SDL_RenderPresent(app.renderer);
    }

    NST_RETURN_NULL;
}

NST_FUNC_SIGN(set_window_)
{
    if ( app.window != nullptr )
    {
        NST_SET_RAW_CALL_ERROR("'set_window' was called more than once");
        return nullptr;
    }

    Nst_StrObj *title;
    Nst_Int w, h;
    Nst_Obj *flags_obj;
    Nst_Obj *pos_x_obj, *pos_y_obj;
    int flags, pos_x, pos_y;

    NST_DEF_EXTRACT("sii?i?i?i", &title, &w, &h, &flags_obj, &pos_x_obj, &pos_y_obj);
    flags = NST_DEF_VAL(flags_obj, int(AS_INT(flags_obj)), 0);
    pos_x = NST_DEF_VAL(pos_x_obj, int(AS_INT(pos_x_obj)), SDL_WINDOWPOS_CENTERED);
    pos_y = NST_DEF_VAL(pos_y_obj, int(AS_INT(pos_y_obj)), SDL_WINDOWPOS_CENTERED);

    app.window = SDL_CreateWindow(title->value, pos_x, pos_y, int(w), int(h), flags);
    if ( app.window == nullptr )
    {
        set_sdl_error(err);
        return nullptr;
    }
    app.renderer = SDL_CreateRenderer(app.window, -1, 0);
    if ( app.renderer == nullptr )
    {
        set_sdl_error(err);
        SDL_DestroyWindow(app.window);
        return nullptr;
    }

    GUI_Element *root = gui_element_new(
        GUI_ET_BASE,
        sizeof(GUI_Element),
        10, 10,
        int(w), int(h),
        &app, err);
    if ( root == nullptr )
    {
        SDL_DestroyWindow(app.window);
        SDL_DestroyRenderer(app.renderer);
        return nullptr;
    }
    root->handle_event_func = root_handle_event;
    root->frame_update_func = root_update;
    app.root = root;
    gui_element_set_parent(root, root);
    gui_element_set_rel_size(
        root, nullptr,
        -1, -1,
        -1, -1,
        0.0, 0.0,
        -20, -20);

    gui_element_set_padding(root, 10, 10, 10, 10);
    gui_element_set_margin(root, 10, 10, 10, 10);

    GUI_Element *test_child = gui_element_new(
        GUI_ET_BASE,
        sizeof(GUI_Element),
        0, 0,
        100, 100,
        &app, err);
    if ( test_child != nullptr )
    {
        gui_element_set_rel_pos(
            test_child,
            root,
            GUI_MIDDLE,
            GUI_CENTER,
            GUI_MIDDLE,
            GUI_CENTER);
        gui_element_set_rel_size(
            test_child,
            root,
            -1, -1,
            -1, -1,
            0.0, 0.0,
            0, 0);

        gui_element_set_margin(test_child, 5, 5, 5, 5);
        gui_element_set_padding(test_child, 5, 5, 5, 5);

        gui_element_add_child(root, test_child, err);
        nst_dec_ref(test_child);
    }

    app.keep_open = true;
    NST_RETURN_NULL;
}
