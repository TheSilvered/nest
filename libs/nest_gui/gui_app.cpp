#include <SDL_ttf.h>

#include "gui_app.h"
#include "gui_utils.h"
#include "gui_update.h"
#include "gui_events.h"

GUI_Window *GUI_Window_New(GUI_Window *parent, GUI_App *app,
                           int width, int height)
{
    GUI_Window *window = nullptr;
    SDL_Window *sdl_window = nullptr;
    SDL_Renderer *renderer = nullptr;

    window = Nst_malloc_c(1, GUI_Window);
    if (window == nullptr)
        goto cleanup;

    sdl_window = SDL_CreateWindow(
        "Nest",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width, height,
        0);
    if (sdl_window == nullptr) {
        GUI_ThrowSDLError();
        goto cleanup;
    }
    renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == nullptr) {
        GUI_ThrowSDLError();
        goto cleanup;
    }

    window->parent = parent;
    window->app = app;
    window->window = sdl_window;
    window->renderer = renderer;
    window->root_element = GUI_Root_New(window, app);
    if (!Nst_sbuffer_init(&window->child_windows, sizeof(GUI_Window *), 0))
        goto cleanup;
    window->keep_open = true;

    return window;

cleanup:
    if (window != nullptr)
        Nst_free(window);
    if (sdl_window != nullptr)
        SDL_DestroyWindow(sdl_window);
    return nullptr;
}

void GUI_Window_Destroy(GUI_Window *window)
{
    SDL_DestroyRenderer(window->renderer);
    SDL_DestroyWindow(window->window);
    Nst_dec_ref(NstOBJ(window->root_element));

    for (usize i = 0, n = window->child_windows.len; i < n; i++) {
        GUI_Window *child = (GUI_Window *)Nst_sbuffer_at(
            &window->child_windows,
            i);
        GUI_Window_Destroy(child);
    }
    Nst_sbuffer_destroy(&window->child_windows);

    Nst_free(window);
}

void GUI_Window_SetTitle(GUI_Window *window, const i8 *title)
{
    SDL_SetWindowTitle(window->window, title);
}

const i8 *GUI_Window_GetTitle(GUI_Window *window)
{
    return SDL_GetWindowTitle(window->window);
}

void GUI_Window_SetPosition(GUI_Window *window, int x, int y)
{
    SDL_SetWindowPosition(window->window, x, y);
}

void GUI_Window_GetPosition(GUI_Window *window, int *x, int *y)
{
    SDL_GetWindowPosition(window->window, x, y);
}

void GUI_Window_SetSize(GUI_Window *window, int w, int h)
{
    SDL_SetWindowSize(window->window, w, h);
}

void GUI_Window_GetSize(GUI_Window *window, int *w, int *h)
{
    SDL_GetWindowSize(window->window, w, h);
}

void GUI_Window_SetResizable(GUI_Window *window, bool resizable)
{
    SDL_SetWindowResizable(window->window, resizable ? SDL_TRUE : SDL_FALSE);
}

bool GUI_Window_GetResizable(GUI_Window *window)
{
    return bool(SDL_GetWindowFlags(window->window) & SDL_WINDOW_RESIZABLE);
}

SDL_Rect GUI_Window_GetClipRect(GUI_Window *window)
{
    int w, h;
    GUI_Window_GetSize(window, &w, &h);
    return SDL_Rect{ 0, 0, w, h };
}

bool GUI_Window_IsRunning(GUI_Window *window)
{
    return window->keep_open;
}

void GUI_Window_Close(GUI_Window *window)
{
    window->keep_open = false;
}

bool GUI_App_Init(GUI_App *app, int window_width, int window_height)
{
    app->window = GUI_Window_New(nullptr, app, window_width, window_height);
    if (app->window == nullptr)
        return false;
    app->focused_element = nullptr;
    Nst_llist_init(&app->loaded_fonts);
    app->initialized = true;
    return true;
}

bool GUI_App_IsRunning(GUI_App *app)
{
    return !app->initialized || GUI_Window_IsRunning(app->window);
}

bool GUI_App_Run(GUI_App *app)
{
    bool return_value = true;
    while (GUI_App_IsRunning(app)) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (!GUI_App_HandleEvent(app, &event)) {
                return_value = false;
                goto cleanup;
            }
        }
        if (!GUI_App_Update(app)) {
            return_value = false;
            goto cleanup;
        }
    }

cleanup:
    GUI_Window_Destroy(app->window);
    Nst_llist_empty(&app->loaded_fonts, (Nst_LListDestructor)TTF_CloseFont);
    app->initialized = false;
    return return_value;
}

void GUI_App_Close(GUI_App *app)
{
    GUI_Window_Close(app->window);
}
