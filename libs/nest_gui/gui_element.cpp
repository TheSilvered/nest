#include "gui_element.h"

GUI_Element *GUI_Element_New(usize size, GUI_Element *parent,
                             struct _GUI_Window *window, struct _GUI_App *app)
{
    GUI_Element *element = (GUI_Element *)_Nst_obj_alloc(size, element_type);

    Nst_GGC_OBJ_INIT(element);
    element->el_type = GUI_ET_NONE;
    element->rect = { 0, 0, 0, 0 };
    element->pad_u = 0;
    element->pad_d = 0;
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

    return element;
}
