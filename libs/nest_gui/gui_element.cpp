#include "gui_app.h"
#include "gui_element.h"
#include "gui_obj_types.h"
#include "gui_constraint.h"
#include "gui_colors.h"

GUI_Element *GUI_Element_New(usize size, GUI_Element *parent,
                             struct _GUI_Window *window, struct _GUI_App *app)
{
    GUI_Element *element = (GUI_Element *)_Nst_obj_alloc(size, element_type);

    Nst_GGC_OBJ_INIT(element);
    element->el_type = GUI_ET_NONE;
    element->rect = { 0, 0, 0, 0 };
    element->pad_t = 0;
    element->pad_b = 0;
    element->pad_l = 0;
    element->pad_r = 0;
    element->parent = parent;
    element->children = VECTOR(Nst_vector_new(0));
    element->window = window;
    element->app = app;
    element->constraints_before = MAP(Nst_map_new());
    element->constraints_after = MAP(Nst_map_new());
    element->event_handler = nullptr;
    element->frame_update = nullptr;
    element->tick_update = nullptr;
    element->el_destructor = nullptr;

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
    element->rect.w = w;
    element->rect.h = h;
}

void GUI_Element_GetSize(GUI_Element *element, int *w, int *h)
{
    *w = element->rect.w;
    *h = element->rect.h;
}

void GUI_Element_SetWidth(GUI_Element *element, int w)
{
    element->rect.w = w;
}

int GUI_Element_GetWidth(GUI_Element *element)
{
    return element->rect.w;
}

void GUI_Element_SetHeight(GUI_Element *element, int h)
{
    element->rect.h = h;
}

int GUI_Element_GetHeight(GUI_Element *element)
{
    return element->rect.h;
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
