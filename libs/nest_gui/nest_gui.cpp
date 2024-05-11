#include <SDL.h>
#include <SDL_ttf.h>
#include "nest_gui.h"

#define FUNC_COUNT 2

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(init_sdl_and_ttf_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(loop_, 0);

#if __LINE__ - FUNC_COUNT != 16
#error
#endif

    GUI_init_utils();

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    GUI_quit_utils();
    TTF_Quit();
    SDL_Quit();
}

Nst_FUNC_SIGN(init_sdl_and_ttf_)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);
    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        GUI_throw_sdl_error();
        return NULL;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(loop_)
{
    Nst_UNUSED(args);
    Nst_UNUSED(arg_num);

    SDL_Window *window = SDL_CreateWindow(
        "Test Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        600,
        400,
        0
    );

    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                running = false;
        }
    }

    SDL_DestroyWindow(window);
    Nst_RETURN_NULL;
}
