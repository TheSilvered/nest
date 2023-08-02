#include "gui_event.h"

static i32 handle_event(SDL_Event *e, GUI_Element *el)
{
    Nst_SeqObj *children = el->children;
    for (usize i = 0, n = children->len; i < n; i++) {
        i32 res = handle_event(e, (GUI_Element *)children->objs[n - i - 1]);
        if (res)
            return res;
    }
    if (el->handle_event_func != nullptr)
        return el->handle_event_func(e, el);
    return 0;
}

bool handle_events(GUI_App *app)
{
    SDL_Event e;
    app->element_reached = false;
    while (SDL_PollEvent(&e)) {
        if (handle_event(&e, app->root) < 0)
            return false;
    }
    return true;
}

i32 default_event_handler(SDL_Event *e, GUI_Element *el)
{
    if (Nst_FLAG_HAS(el, GUI_FLAG_IS_HIDDEN))
        return 0;

    switch (e->type) {
    case SDL_MOUSEMOTION: {
        SDL_Point p = { e->motion.x, e->motion.y };
        if (SDL_PointInRect(&p, &el->rect)) {
            el->app->element_reached = true;
            return 1;
        }
        return 0;
    }
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP: {
        SDL_Point p = { e->button.x, e->button.y };
        if (SDL_PointInRect(&p, &el->rect)) {
            el->app->element_reached = true;
            return 1;
        }
        return 0;
    }
    }
    return 0;
}

i32 root_handle_event(SDL_Event *e, GUI_Element *el)
{
    switch (e->type) {
    case SDL_QUIT:
        el->app->keep_open = false;
        return 1;
    default:
        return default_event_handler(e, el);
    }
    return 0;
}
