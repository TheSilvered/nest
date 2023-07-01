#include "gui_event.h"

static i32 handle_event(SDL_Event   *e,
                        GUI_Element *el,
                        bool         avoid_focused_element,
                        Nst_OpErr   *err)
{
    Nst_SeqObj *children = el->children;
    for ( usize i = 0, n = children->len; i < n; i++ )
    {
        i32 res = handle_event(e, (GUI_Element *)children->objs[n - i - 1], true, err);
        if ( res )
        {
            return res;
        }
    }

    if ( !avoid_focused_element && el == get_focused_element() )
    {
        return 0;
    }
    if ( el->handle_event_func != nullptr  )
    {
        return el->handle_event_func(e, el, err);
    }
    return 0;
}

bool handle_events(GUI_App *app, Nst_OpErr *err)
{
    SDL_Event e;
    while ( SDL_PollEvent(&e) )
    {
        if ( get_focused_element() != nullptr )
        {
            i32 res = handle_event(&e, get_focused_element(), false, err);

            if ( res < 0 )
            {
                return false;
            }
            else if ( res > 0 )
            {
                continue;
            }
        }
        if ( handle_event(&e, app->root, true, err) < 0 )
        {
            return false;
        }
    }
    return true;
}

i32 default_event_handler(SDL_Event *e, GUI_Element *el, Nst_OpErr *err)
{
    switch ( e->type )
    {
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
    {
        SDL_Point p = { e->button.x, e->button.y };
        return SDL_PointInRect(&p, &el->rect);
    }
    }
    return 0;
}

i32 root_handle_event(SDL_Event *e, GUI_Element *el, Nst_OpErr *err)
{
    switch ( e->type )
    {
    case SDL_QUIT:
        el->app->keep_open = false;
        return 1;
    default:
        return default_event_handler(e, el, err);
    }
    return 0;
}
