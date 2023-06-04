#include "gui_update.h"
#include "gui_element.h"
#include "gui_draw.h"

static void draw_element_bounds(GUI_Element *el)
{
    SDL_Renderer *renderer = el->app->renderer;
    SDL_Rect r = gui_element_get_margin_rect(el);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    draw_rect(renderer, &r);

    r = gui_element_get_padding_rect(el);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    draw_rect(renderer, &r);

    r = el->rect;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    draw_rect(renderer, &r);
}

static bool update_element(GUI_Element *el, bool show_bounds, Nst_OpErr *err)
{
    gui_element_update_pos(el);
    gui_element_update_size(el);

    if ( el->frame_update_func != nullptr )
    {
        if ( !el->frame_update_func(el, err) )
        {
            return false;
        }
    }

    if ( show_bounds )
    {
        draw_element_bounds(el);
    }

    Nst_SeqObj *children = el->children;
    for ( usize i = 0, n = children->len; i < n; i++ )
    {
        if ( !update_element((GUI_Element *)children->objs[i], show_bounds, err) )
        {
            return false;
        }
    }
    return true;
}

static bool tick_element(GUI_Element *el, Nst_OpErr *err)
{
    if ( el->tick_update_func != nullptr )
    {
        if ( !el->tick_update_func(el, err) )
        {
            return false;
        }
    }

    Nst_SeqObj *children = el->children;
    for ( usize i = 0, n = children->len; i < n; i++ )
    {
        if ( !tick_element((GUI_Element *)children->objs[i], err) )
        {
            return false;
        }
    }
    return true;
}

bool update_elements(GUI_App *app, Nst_OpErr *err)
{
    return update_element(app->root, app->show_bounds, err);
}

bool tick_elements(GUI_App *app, Nst_OpErr *err)
{
    return tick_element(app->root, err);
}

bool root_update(GUI_Element *el, Nst_OpErr *err)
{
    SDL_Renderer *renderer = el->app->renderer;
    SDL_Color *bg_color = &el->app->bg_color;
    SDL_SetRenderDrawColor(renderer, bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    SDL_RenderClear(renderer);
    return true;
}
