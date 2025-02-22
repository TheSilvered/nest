#include "gui_obj_types.h"
#include "gui_constraint.h"
#include "gui_element.h"

Nst_Obj *element_type = nullptr;
Nst_Obj *constr_type = nullptr;

bool GUI_InitTypes()
{
    element_type = Nst_cont_type_new(
        "GUIElement",
        (Nst_ObjDstr)GUI_Element_Destroy,
        (Nst_ObjTrav)GUI_Element_Traverse);
    constr_type = Nst_cont_type_new(
        "GUIConstraint",
        (Nst_ObjDstr)GUI_Constraint_Destroy,
        (Nst_ObjTrav)GUI_Constraint_Traverse);

    if (Nst_error_occurred()) {
        GUI_QuitTypes();
        return false;
    }
    return true;
}

void GUI_QuitTypes()
{
    Nst_ndec_ref(element_type);
    Nst_ndec_ref(constr_type);
}
