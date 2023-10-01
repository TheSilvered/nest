#include "gui_update.h"
#include "gui_element.h"
#include "gui_draw.h"
#include "gui_animation.h"

using namespace GUI;

static void draw_element_bounds(Element *el)
{
    SDL_Renderer *renderer = el->app->renderer;

    SDL_Rect r = element_get_padding_rect(el);
    SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
    draw_rect(renderer, &r);

    r = el->rect;
    SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
    draw_rect(renderer, &r);
}

static bool update_element(Element *el, bool show_bounds)
{
    if (IS_HIDDEN(el))
        return true;

    Nst_MapObj *animations = el->animations;
    for (i32 i = Nst_map_get_next_idx(-1, animations);
         i != -1;
         i = Nst_map_get_next_idx(i, animations))
    {
        AniObj *ani = (AniObj *)(animations->nodes[i].value);
        if (ani_is_running(ani))
            ani_update(ani);
    }

    if (el->frame_update_func != nullptr) {
        if (!el->frame_update_func(el))
            return false;
    }

    if (show_bounds)
        draw_element_bounds(el);

    Nst_SeqObj *children = el->children;
    for (usize i = 0, n = children->len; i < n; i++) {
        if (!update_element((Element *)children->objs[i], show_bounds))
            return false;
    }
    return true;
}

static bool tick_element(Element *el)
{
    if (el->tick_update_func != nullptr) {
        if (!el->tick_update_func(el))
            return false;
    }

    Nst_SeqObj *children = el->children;
    for (usize i = 0, n = children->len; i < n; i++) {
        if (!tick_element((Element *)children->objs[i]))
            return false;
    }
    return true;
}

bool GUI::update_elements(App *app)
{
    SDL_GetWindowSize(app->window, &app->clip_window.w, &app->clip_window.h);
    return update_element(app->root, app->show_bounds);
}

bool GUI::tick_elements(App *app)
{
    return tick_element(app->root);
}

bool GUI::root_update(Element *el)
{
    SDL_Renderer *renderer = el->app->renderer;
    SDL_Color *bg_color = &el->app->bg_color;
    SDL_SetRenderDrawColor(renderer, bg_color->r, bg_color->g, bg_color->b, bg_color->a);
    SDL_RenderClear(renderer);
    return true;
}
