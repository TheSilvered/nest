#include "gui_stack_layout.h"
#include "gui_animation.h"
#include "gui_draw.h"

using namespace GUI;

i32 GUI::stack_layout_handle_event(SDL_Event *e, StackLayout *sl)
{
    if (e->type != SDL_USEREVENT
        || e->user.data1 != sl
        || e->user.code != GUI_UE_CHILD_ADDED)
        return 0;

    isize idx = (isize)e->user.data2;

    Element *child = (Element *)sl->children->objs[idx];
    Element *prev_child = nullptr;

    if (idx >= 1)
        prev_child = (Element *)sl->children->objs[idx - 1];

    switch (sl->sd) {
    case SD_TOP_BOTTOM:
        match_y(
            idx == 0 ? (Element *)sl : prev_child,
            child,
            idx == 0 ? P_TOP : BOTTOM,
            TOP);
        goto h_sa;
    case SD_BOTTOM_TOP:
        match_y(
            idx == 0 ? (Element *)sl : prev_child,
            child,
            idx == 0 ? P_BOTTOM : TOP,
            BOTTOM);
        goto h_sa;
    case SD_LEFT_RIGHT:
        match_x(
            idx == 0 ? (Element *)sl : prev_child,
            child,
            idx == 0 ? P_LEFT : RIGHT,
            LEFT);
        goto v_sa;
    case SD_RIGHT_LEFT:
        match_x(
            idx == 0 ? (Element *)sl : prev_child,
            child,
            idx == 0 ? P_RIGHT : LEFT,
            RIGHT);
        goto v_sa;
    }

h_sa:
    switch (sl->sa) {
    case SA_LEFT_TOP:
        match_x((Element *)sl, child, P_LEFT, LEFT);
        break;
    case SA_MIDDLE:
        match_x((Element *)sl, child, P_CENTER, CENTER);
        break;
    case SA_BOTTOM_RIGHT:
        match_x((Element *)sl, child, P_RIGHT, RIGHT);
        break;
    }
    return true;

v_sa:
    switch (sl->sa) {
    case SA_LEFT_TOP:
        match_y((Element *)sl, child, P_TOP, TOP);
        break;
    case SA_MIDDLE:
        match_y((Element *)sl, child, P_CENTER, CENTER);
        break;
    case SA_BOTTOM_RIGHT:
        match_y((Element *)sl, child, P_BOTTOM, BOTTOM);
        break;
    }
    return true;
}

bool GUI::stack_layout_update(StackLayout *sl)
{
    SDL_Rect clip = element_get_clip_rect((Element *)sl);
    SDL_Rect p_rect = element_get_padding_rect((Element *)sl);
    SDL_Color c = sl->app->bg_color3;
    SDL_Texture *texture = draw_round_rect(
        sl->app->renderer,
        p_rect,
        6, 6, 6, 6,
        c.r, c.g, c.b, 60);
    draw_texture(sl->app, p_rect.x, p_rect.y, texture, &clip);
    SDL_DestroyTexture(texture);
    return true;
}

Element *GUI::stack_layout_new(StackDir direction, StackAlign alignment,
                               int x, int y, int w, int h, App *app)
{
    StackLayout *new_sl = (StackLayout *)element_new(
        GUI_ET_STACK_LAYOUT,
        sizeof(StackLayout),
        x, y, w, h,
        app, nullptr);
    if (new_sl == nullptr)
        return nullptr;
    new_sl->sd = direction;
    new_sl->sa = alignment;
    new_sl->handle_event_func = (HandleEventFunc)stack_layout_handle_event;
    new_sl->frame_update_func = (UpdateFunc)stack_layout_update;
    element_set_padding((Element *)new_sl, 3, 3, 3, 3);

    return (Element *)new_sl;
}
