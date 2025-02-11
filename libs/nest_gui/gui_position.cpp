#include "gui_position.h"

void GUI_PointToSides(GUI_Point point, GUI_XSide *x_side, GUI_YSide *y_side)
{
    *x_side = GUI_XSide(short(point) >> 3);
    *y_side = GUI_YSide(short(point) &  7);
}

GUI_Point GUI_SidesToPoint(GUI_XSide x_side, GUI_YSide y_side)
{
    return GUI_Point((short(x_side) << 3) | short(y_side));
}

int GUI_Element_GetXSide(GUI_Element *element, GUI_XSide x_side)
{
    int x = element->rect.x;
    int w = element->rect.w;

    switch (x_side) {
    case GUI_L:
        return x;
    case GUI_C:
        return x + w / 2;
    case GUI_R:
        return x + w;
    case GUI_LP:
        return x - element->pad_l;
    case GUI_CP:
        return (2 * x - element->pad_l + w + element->pad_r) / 2;
    case GUI_RP:
        return x + w + element->pad_r;
    default:
        return 0;
    }
}

int GUI_Element_GetYSide(GUI_Element *element, GUI_YSide y_side)
{
    int y = element->rect.y;
    int h = element->rect.h;

    switch (y_side) {
    case GUI_T:
        return y;
    case GUI_M:
        return y + h / 2;
    case GUI_B:
        return y + h;
    case GUI_TP:
        return y - element->pad_t;
    case GUI_MP:
        return (2 * y - element->pad_t + h + element->pad_b) / 2;
    case GUI_BP:
        return y + h + element->pad_b;
    default:
        return 0;
    }
}

void GUI_Element_SetXSide(GUI_Element *element, GUI_XSide x_side, int value)
{
    int w = element->rect.w;

    switch (x_side) {
    case GUI_L:
        element->rect.x = value;
        break;
    case GUI_C:
        element->rect.x = value - w / 2;
        break;
    case GUI_R:
        element->rect.x = value - w;
        break;
    case GUI_LP:
        element->rect.x = value + element->pad_l;
        break;
    case GUI_CP:
        element->rect.x = value + (element->pad_l - w - element->pad_r) / 2;
        break;
    case GUI_RP:
        element->rect.x = value - w - element->pad_r;
        break;
    }
}

void GUI_Element_SetYSide(GUI_Element *element, GUI_YSide y_side, int value)
{
    int h = element->rect.h;

    switch (y_side) {
    case GUI_T:
        element->rect.y = value;
        break;
    case GUI_M:
        element->rect.y = value - h / 2;
        break;
    case GUI_B:
        element->rect.y = value - h;
        break;
    case GUI_TP:
        element->rect.y = value + element->pad_t;
        break;
    case GUI_MP:
        element->rect.y = value + (element->pad_t - h - element->pad_b) / 2;
        break;
    case GUI_BP:
        element->rect.y = value - h - element->pad_b;
        break;
    }
}

void GUI_Element_GetPoint(GUI_Element *element, GUI_Point point,
    int *x, int *y)
{
    GUI_XSide x_side;
    GUI_YSide y_side;
    GUI_PointToSides(point, &x_side, &y_side);

    *x = GUI_Element_GetXSide(element, x_side);
    *y = GUI_Element_GetYSide(element, y_side);
}

void GUI_Element_SetPoint(GUI_Element *element, GUI_Point point,
    int x, int y)
{
    GUI_XSide x_side;
    GUI_YSide y_side;
    GUI_PointToSides(point, &x_side, &y_side);

    GUI_Element_SetXSide(element, x_side, x);
    GUI_Element_SetYSide(element, y_side, y);
}

void GUI_PointPos_InitAbs(GUI_PointPos *pos, int x, int y)
{
    pos->abs.x = x;
    pos->abs.y = y;
    pos->is_abs = true;
}

void GUI_PointPos_InitRel(GUI_PointPos *pos, GUI_Element *element,
                          GUI_Point point)
{
    Nst_inc_ref(element);
    pos->rel.element = element;
    pos->rel.point = point;
    pos->is_abs = false;
}

void GUI_PointPos_Destroy(GUI_PointPos *pos)
{
    if (!pos->is_abs)
        Nst_dec_ref(pos->rel.element);
}


void GUI_PointPos_Traverse(GUI_PointPos *pos)
{
    if (!pos->is_abs)
        Nst_ggc_obj_reachable(OBJ(pos->rel.element));
}

void GUI_PointPos_Value(GUI_PointPos *pos, int *x, int *y)
{
    if (pos->is_abs) {
        *x = pos->abs.x;
        *y = pos->abs.y;
        return;
    }
    GUI_Element_GetPoint(pos->rel.element, pos->rel.point, x, y);
}

void GUI_XSidePos_InitAbs(GUI_XSidePos *pos, int x)
{
    pos->abs.x = x;
    pos->is_abs = true;
}

void GUI_XSidePos_InitRel(GUI_XSidePos *pos, GUI_Element *element,
                          GUI_XSide x_side)
{
    Nst_inc_ref(element);
    pos->rel.element = element;
    pos->rel.x_side = x_side;
    pos->is_abs = false;
}

void GUI_XSidePos_Destroy(GUI_XSidePos *pos)
{
    if (!pos->is_abs)
        Nst_dec_ref(pos->rel.element);
}

void GUI_XSidePos_Traverse(GUI_XSidePos *pos)
{
    if (!pos->is_abs)
        Nst_ggc_obj_reachable(OBJ(pos->rel.element));
}

int GUI_XSidePos_Value(GUI_XSidePos *pos)
{
    if (pos->is_abs)
        return pos->abs.x;
    return GUI_Element_GetXSide(pos->rel.element, pos->rel.x_side);
}

void GUI_YSidePos_InitAbs(GUI_YSidePos *pos, int y)
{
    pos->abs.y = y;
    pos->is_abs = true;
}

void GUI_YSidePos_InitRel(GUI_YSidePos *pos, GUI_Element *element,
                          GUI_YSide y_side)
{
    Nst_inc_ref(pos->rel.element);
    pos->rel.element = element;
    pos->rel.y_side = y_side;
    pos->is_abs = false;
}

void GUI_YSidePos_Destroy(GUI_YSidePos *pos)
{
    if (!pos->is_abs)
        Nst_dec_ref(pos->rel.element);
}

void GUI_YSidePos_Traverse(GUI_YSidePos *pos)
{
    if (!pos->is_abs)
        Nst_ggc_obj_reachable(OBJ(pos->rel.element));
}

int GUI_YSidePos_Value(GUI_YSidePos *pos)
{
    if (pos->is_abs)
        return pos->abs.y;
    return GUI_Element_GetYSide(pos->rel.element, pos->rel.y_side);
}
