#include <SDL.h>

#include "gui_app.h"
#include "gui_element.h"
#include "gui_obj_types.h"
#include "gui_constraint.h"
#include "gui_colors.h"
#include "gui_events.h"

GUI_Element *GUI_Element_New(usize size, GUI_Element *parent,
                             struct _GUI_Window *window, struct _GUI_App *app)
{
    GUI_Element *element = (GUI_Element *)_Nst_obj_alloc(size, element_type);

    element->el_type = GUI_ET_NONE;
    element->rect = { 0, 0, 0, 0 };
    element->pad_t = 0;
    element->pad_b = 0;
    element->pad_l = 0;
    element->pad_r = 0;
    element->parent = parent;
    element->children = Nst_vector_new(0);
    element->window = window;
    element->app = app;
    element->constraints_before = Nst_map_new();
    element->constraints_after = Nst_map_new();
    element->event_handler = nullptr;
    element->frame_update = nullptr;
    element->tick_update = nullptr;
    element->el_destructor = nullptr;
    element->state = GUI_ES_ENABLED;
    element->prev_state = GUI_ES_NONE;
    element->important = false;

    Nst_GGC_OBJ_INIT(element);

    return element;
}

void GUI_Element_Destroy(GUI_Element *element)
{
    if (element->el_destructor != nullptr)
        element->el_destructor(element);
    Nst_dec_ref(element->constraints_before);
    Nst_dec_ref(element->constraints_after);
    Nst_dec_ref(element->children);
}

void GUI_Element_Traverse(GUI_Element *element)
{
    Nst_ggc_obj_reachable(element->constraints_before);
    Nst_ggc_obj_reachable(element->constraints_after);
    Nst_ggc_obj_reachable(element->children);
}

void GUI_Element_SetSize(GUI_Element *element, int w, int h)
{
    if (element->rect.w != w || element->rect.h != h)
        GUI_Event_PushUserEvent(element, GUI_E_RESIZE);
    element->rect.w = w;
    element->rect.h = h;
}

void GUI_Element_GetSize(GUI_Element *element, int *w, int *h)
{
    if (w != nullptr)
        *w = element->rect.w;
    if (h != nullptr)
        *h = element->rect.h;
}

void GUI_Element_SetWidth(GUI_Element *element, int w)
{
    if (element->rect.w != w)
        GUI_Event_PushUserEvent(element, GUI_E_RESIZE);
    element->rect.w = w;
}

int GUI_Element_GetWidth(GUI_Element *element)
{
    return element->rect.w;
}

void GUI_Element_SetHeight(GUI_Element *element, int h)
{
    if (element->rect.h != h)
        GUI_Event_PushUserEvent(element, GUI_E_RESIZE);
    element->rect.h = h;
}

int GUI_Element_GetHeight(GUI_Element *element)
{
    return element->rect.h;
}

bool GUI_Element_IsImportant(GUI_Element *element)
{
    return element->important;
}

void GUI_Element_SetImportant(GUI_Element *element, bool important)
{
    if (element->important != important)
        GUI_Event_PushUserEvent(element, GUI_E_IMPORTANT);
    element->important = important;
}

void GUI_Element_SetPadding(GUI_Element *element, i32 t, i32 b, i32 l, i32 r)
{
    i32 pad_t = element->pad_t;
    i32 pad_b = element->pad_b;
    i32 pad_l = element->pad_l;
    i32 pad_r = element->pad_r;
    element->pad_t = t;
    element->pad_b = b;
    element->pad_l = l;
    element->pad_r = r;
    if (t != pad_t || b != pad_b || l != pad_l || r != pad_r)
        GUI_Event_PushUserEvent(element, GUI_E_PADDING);
}

void GUI_Element_GetPadding(GUI_Element *element,
                            i32 *t, i32 *b, i32 *l, i32 *r)
{
    if (t != nullptr)
        *t = element->pad_t;
    if (b != nullptr)
        *b = element->pad_b;
    if (l != nullptr)
        *l = element->pad_l;
    if (r != nullptr)
        *r = element->pad_r;
}

void GUI_Element_SetPaddingTop(GUI_Element *element, i32 pad_top)
{
    if (element->pad_t != pad_top)
        GUI_Event_PushUserEvent(element, GUI_E_PADDING);
    element->pad_t = pad_top;
}

i32 GUI_Element_GetPaddingTop(GUI_Element *element)
{
    return element->pad_t;
}

void GUI_Element_SetPaddingBottom(GUI_Element *element, i32 pad_bottom)
{
    if (element->pad_b != pad_bottom)
        GUI_Event_PushUserEvent(element, GUI_E_PADDING);
    element->pad_b = pad_bottom;
}

i32 GUI_Element_GetPaddingBottom(GUI_Element *element)
{
    return element->pad_b;
}

void GUI_Element_SetPaddingLeft(GUI_Element *element, i32 pad_left)
{
    if (element->pad_l != pad_left)
        GUI_Event_PushUserEvent(element, GUI_E_PADDING);
    element->pad_l = pad_left;
}

i32 GUI_Element_GetPaddingLeft(GUI_Element *element)
{
    return element->pad_l;
}


void GUI_Element_SetPaddingRight(GUI_Element *element, i32 pad_right)
{
    if (element->pad_r != pad_right)
        GUI_Event_PushUserEvent(element, GUI_E_PADDING);
    element->pad_r = pad_right;
}

i32 GUI_Element_GetPaddingRight(GUI_Element *element)
{
    return element->pad_r;
}

bool GUI_Element_IsDisabled(GUI_Element *element)
{
    GUI_ElementState state = element->state;
    return state == GUI_ES_DISABLED || state == GUI_ES_NONE;
}

void GUI_Element_Disable(GUI_Element *element)
{
    GUI_ElementState initial_state = element->state;
    element->state = GUI_ES_DISABLED;
    if (initial_state == GUI_ES_DISABLED || initial_state == GUI_ES_READONLY)
        element->prev_state = initial_state;
    else
        element->prev_state = GUI_ES_NONE;

    if (element->state != initial_state)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);

    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_Disable(child);
    }
}

void GUI_Element_DisableAll(GUI_Element *element)
{
    if (element->state != GUI_ES_DISABLED)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);
    element->state = GUI_ES_DISABLED;
    element->prev_state = GUI_ES_NONE;
    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_DisableAll(child);
    }
}

bool GUI_Element_IsReadonly(GUI_Element *element)
{
    return element->state == GUI_ES_READONLY;
}

void GUI_Element_EnableReadonly(GUI_Element *element)
{
    GUI_ElementState initial_state = element->state;

    if (element->prev_state == GUI_ES_NONE)
        element->state = GUI_ES_READONLY;
    else {
        element->state = element->prev_state;
        element->prev_state = GUI_ES_NONE;
    }
    if (element->state != initial_state)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);

    if (initial_state == GUI_ES_READONLY)
        element->prev_state = GUI_ES_READONLY;
    else
        element->prev_state = GUI_ES_NONE;

    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_EnableReadonly(child);
    }
}

void GUI_Element_EnableReadonlyAll(GUI_Element *element)
{
    if (element->state != GUI_ES_READONLY)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);
    element->state = GUI_ES_READONLY;
    element->prev_state = GUI_ES_NONE;
    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_EnableReadonlyAll(child);
    }
}

bool GUI_Element_IsEnabled(GUI_Element *element)
{
    return element->state == GUI_ES_ENABLED;
}

void GUI_Element_Enable(GUI_Element *element)
{
    GUI_ElementState initial_state = element->state;

    if (element->prev_state == GUI_ES_NONE)
        element->state = GUI_ES_READONLY;
    else {
        element->state = element->prev_state;
        element->prev_state = GUI_ES_NONE;
    }
    if (element->state != initial_state)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);

    if (initial_state == GUI_ES_READONLY || initial_state == GUI_ES_DISABLED)
        element->prev_state = initial_state;
    else
        element->prev_state = GUI_ES_NONE;

    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_Enable(child);
    }
}

void GUI_Element_EnableAll(GUI_Element *element)
{
    if (element->state != GUI_ES_ENABLED)
        GUI_Event_PushUserEvent(element, GUI_E_STATE);
    element->state = GUI_ES_ENABLED;
    element->prev_state = GUI_ES_NONE;
    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_EnableAll(child);
    }
}

void GUI_Element_ForgetPrevState(GUI_Element *element)
{
    element->prev_state = GUI_ES_NONE;
    for (usize i = 0, n = Nst_seq_len(element->children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(element->children, i);
        GUI_Element_ForgetPrevState(child);
    }
}

static bool root_update(GUI_Element *root)
{
    GUI_Window *window = root->window;
    GUI_SetDrawColor(window->renderer, GUI_COL_BODY_BG);
    SDL_RenderClear(window->renderer);
    return true;
}

GUI_Element *GUI_Root_New(struct _GUI_Window *window, struct _GUI_App *app)
{
    GUI_Element *element = GUI_Element_New(
        sizeof(GUI_Element),
        nullptr,
        window,
        app);
    if (element == nullptr)
        return nullptr;

    element->el_type = GUI_ET_ROOT;
    element->tick_update = root_update;

    GUI_Element_AddConstraintBefore(GUI_MatchWindowSize_New(element));

    return element;
}
