#include "gui_event.h"

static i32 handle_event(SDL_Event *e, GUI_Element *el, Nst_OpErr *err)
{
    Nst_SeqObj *children = el->children;
    for ( usize i = 0, n = children->len; i < n; i++ )
    {
        i32 res = handle_event(e, (GUI_Element *)children->objs[i], err);
        if ( res )
        {
            return res;
        }
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
        if ( handle_event(&e, app->root, err) == -1 )
        {
            return false;
        }
    }
    return true;
}

i32 root_handle_event(SDL_Event *e, GUI_Element *el, Nst_OpErr *err)
{
    if ( e->type == SDL_QUIT )
    {
        el->app->keep_open = false;
        return true;
    }
    return false;
}
