#include "gui_update.h"
#include "gui_constraint.h"

static bool update_window(GUI_Window *window);
static bool update_element(GUI_Element *element);
static bool update_constraints(Nst_Obj *constrs);
static bool update_single_element(GUI_Element *element);
static bool update_children(Nst_Obj *children);

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
        GUI_Window *child_window = (GUI_Window *)Nst_da_at(
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

static bool update_constraints(Nst_Obj *constrs)
{
    GUI_Constraint *constr = nullptr;
    for (isize i = Nst_map_next(-1, constrs, nullptr, (Nst_Obj **)(&constr));
         i != -1;
         i = Nst_map_next(i, constrs, nullptr, (Nst_Obj **)(&constr)))
    {
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

static bool update_children(Nst_Obj *children)
{
    for (i64 i = 0, n = Nst_seq_len(children); i < n; i++) {
        GUI_Element *child = (GUI_Element *)Nst_seq_getnf(children, i);
        if (!update_element(child))
            return false;
    }
    return true;
}
