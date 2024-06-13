#include "gui_constraints.h"
#include "gui_obj_types.h"

static void fill_between_x_dstr(GUI_FillBetweenXData *data);
static void fill_between_x_trav(GUI_FillBetweenXData *data);
static bool fill_between_x_update(GUI_Constraint *constr);
static void fill_between_y_dstr(GUI_FillBetweenYData *data);
static void fill_between_y_trav(GUI_FillBetweenYData *data);
static bool fill_between_y_update(GUI_Constraint *constr);

static GUI_Constraint *new_constraint(GUI_Element *element,
                                      GUI_ConstraintUpdate update)
{
    GUI_Constraint *constr = Nst_obj_alloc(GUI_Constraint, constr_type);
    if (constr == nullptr)
        return nullptr;

    Nst_GGC_OBJ_INIT(constr);
    constr->element = element;
    constr->data_dstr = nullptr;
    constr->data_trav = nullptr;
    constr->update = update;

    return constr;
}

void GUI_Constraint_Destroy(GUI_Constraint *constr)
{
    if (constr->data_dstr != nullptr)
        constr->data_dstr(constr->data._any);
}

void GUI_Constraint_Traverse(GUI_Constraint *constr)
{
    if (constr->data_trav != nullptr)
        constr->data_trav(constr->data._any);
}

bool GUI_Constraint_Update(GUI_Constraint *constr)
{
    return constr->update(constr);
}

bool GUI_Element_AddConstraintBefore(GUI_Constraint *constr)
{
    return Nst_vector_append(constr->element->constraints_before, constr);
}

bool GUI_Element_AddConstraintAfter(GUI_Constraint *constr)
{
    return Nst_vector_append(constr->element->constraints_after, constr);
}

GUI_Constraint *GUI_FillBetweenX_New(GUI_Element *element,
                                     GUI_XSidePos left, GUI_XSidePos right,
                                     bool pad_l, bool pad_r)
{
    GUI_FillBetweenXData *data = Nst_malloc_c(1, GUI_FillBetweenXData);
    if (data == nullptr)
        return nullptr;

    GUI_Constraint *constr = new_constraint(element, fill_between_x_update);
    if (constr == nullptr)
        return nullptr;

    constr->data_dstr = (GUI_ConstraintDataDstr)fill_between_x_dstr;
    constr->data_trav = (GUI_ConstraintDataTrav)fill_between_x_trav;
    constr->data.fill_between_x = data;

    data->left = left;
    data->right = right;
    data->pad_l = pad_l;
    data->pad_r = pad_r;

    return constr;
}

GUI_Constraint *GUI_FillBetweenY_New(GUI_Element *element,
                                     GUI_YSidePos top, GUI_YSidePos bottom,
                                     bool pad_t, bool pad_b)
{
    GUI_FillBetweenYData *data = Nst_malloc_c(1, GUI_FillBetweenYData);
    if (data == nullptr)
        return nullptr;

    GUI_Constraint *constr = new_constraint(element, fill_between_y_update);
    if (constr == nullptr)
        return nullptr;

    constr->data_dstr = (GUI_ConstraintDataDstr)fill_between_y_dstr;
    constr->data_trav = (GUI_ConstraintDataTrav)fill_between_y_trav;
    constr->data.fill_between_y = data;

    data->top = top;
    data->bottom = bottom;
    data->pad_t = pad_t;
    data->pad_b = pad_b;

    return constr;
}

static void fill_between_x_dstr(GUI_FillBetweenXData *data)
{
    GUI_XSidePos_Destroy(&data->left);
    GUI_XSidePos_Destroy(&data->right);
}

static void fill_between_x_trav(GUI_FillBetweenXData *data)
{
    GUI_XSidePos_Traverse(&data->left);
    GUI_XSidePos_Traverse(&data->right);
}

static bool fill_between_x_update(GUI_Constraint *constr)
{
    GUI_Element *element = constr->element;
    int left  = GUI_XSidePos_Value(&constr->data.fill_between_x->left);
    int right = GUI_XSidePos_Value(&constr->data.fill_between_x->right);
    bool pad_l = constr->data.fill_between_x->pad_l;
    bool pad_r = constr->data.fill_between_x->pad_r;

    if (left > right) {
        int temp = left;
        left = right;
        right = temp;
    }
    int width = right - left;
    if (pad_l)
        width -= element->pad_l;
    if (pad_r)
        width -= element->pad_r;
    if (width < 0)
        width = 0;

    GUI_Element_SetXSide(element, pad_l ? GUI_LP : GUI_L, left);
    GUI_Element_SetWidth(element, width);
    return true;
}

static void fill_between_y_dstr(GUI_FillBetweenYData *data)
{
    GUI_YSidePos_Destroy(&data->top);
    GUI_YSidePos_Destroy(&data->bottom);
}

static void fill_between_y_trav(GUI_FillBetweenYData *data)
{
    GUI_YSidePos_Traverse(&data->top);
    GUI_YSidePos_Traverse(&data->bottom);
}

static bool fill_between_y_update(GUI_Constraint *constr)
{
    GUI_Element *element = constr->element;
    int top    = GUI_YSidePos_Value(&constr->data.fill_between_y->top);
    int bottom = GUI_YSidePos_Value(&constr->data.fill_between_y->bottom);
    bool pad_t = constr->data.fill_between_y->pad_t;
    bool pad_b = constr->data.fill_between_y->pad_b;

    if (top > bottom) {
        int temp = top;
        top = bottom;
        bottom = temp;
    }
    int height = bottom - top;
    if (pad_t)
        height -= element->pad_t;
    if (pad_b)
        height -= element->pad_b;
    if (height < 0)
        height = 0;

    GUI_Element_SetYSide(element, pad_t ? GUI_TP : GUI_T, top);
    GUI_Element_SetHeight(element, height);
    return true;
}
