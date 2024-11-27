#include "gui_update.h"
#include "gui_constraint.h"

static bool update_window(GUI_Window *window);
static bool update_element(GUI_Element *element);
static bool update_constraints(Nst_MapObj *constrs);
static bool update_single_element(GUI_Element *element);
static bool update_children(Nst_VectorObj *children);

bool GUI_App_Update(GUI_App *app)
{
    return update_window(app->window);
}

static bool update_window(GUI_Window *window)
{
    if (!update_element(window->root_element))
        return false;
    SDL_RenderPresent(window->renderer);

    // updating windows in reverse allows them to be closed
    usize child_windows_len = window->child_windows.len;
    for (usize i = 0; i < child_windows_len; i++) {
        GUI_Window *child_window = (GUI_Window *)Nst_sbuffer_at(
            &window->child_windows,
            child_windows_len - i - 1);
        if (!GUI_Window_IsRunning(child_window)) {
            GUI_Window_Destroy(child_window);
        }
        if (!update_window(child_window))
            return false;
    }
    return true;
}

bool update_element(GUI_Element *element)
{
    if (!update_constraints(element->constraints_before))
        return false;
    if (!update_single_element(element))
        return false;
    if (!update_children(element->children))
        return false;
    if (!update_constraints(element->constraints_after))
        return false;
    return true;
}

static bool update_constraints(Nst_MapObj *constrs)
{
    for (i32 i = Nst_map_get_next_idx(-1, constrs);
         i != -1;
         i = Nst_map_get_next_idx(i, constrs))
    {
        GUI_Constraint *constr = (GUI_Constraint *)(constrs->nodes[i].value);
        if (!constr->update(constr))
            return false;
    }
    return true;
}

static bool update_single_element(GUI_Element *element)
{
    if (element->tick_update != nullptr)
        return element->tick_update(element);
    return true;
}

static bool update_children(Nst_VectorObj *children)
{
    for (i64 i = 0, n = children->len; i < n; i++) {
        GUI_Element *child = (GUI_Element *)(children->objs[i]);
        if (!update_element(child))
            return false;
    }
    return true;
}
