#include "gui_stack_layout.h"

bool gui_stack_layout_on_child_added(GUI_StackLayout *sl, usize idx, Nst_OpErr *err)
{
    GUI_Element *child = (GUI_Element *)sl->children->objs[idx];
    GUI_Element *parent = (GUI_Element *)sl;
    GUI_Element *prev_child = nullptr;

    if ( idx >= 1 )
    {
        prev_child = (GUI_Element *)sl->children->objs[idx - 1];
    }

    GUI_StackDir inv_sd = GUI_StackDir(sl->sd - 2);

    if ( inv_sd == -1 )
    {
        inv_sd = (GUI_StackDir)3;
    }
    else if ( inv_sd == -2 )
    {
        inv_sd = (GUI_StackDir)2;
    }

    switch ( sl->sd )
    {
    case GUI_SD_TOP_BOTTOM:
    case GUI_SD_BOTTOM_TOP:
        if ( idx == 0 )
        {
            gui_element_set_rel_pos(
                child, parent,
                GUI_RECT_MARGIN,
                (GUI_RelPosX)sl->sa, (GUI_RelPosY)sl->sd,
                (GUI_RelPosX)sl->sa, (GUI_RelPosY)sl->sd);
            break;
        }
        gui_element_set_rel_pos(
            child, prev_child,
            GUI_RECT_MARGIN,
            (GUI_RelPosX)sl->sa, (GUI_RelPosY)inv_sd,
            (GUI_RelPosX)sl->sa, (GUI_RelPosY)sl->sd);
        break;
    case GUI_SD_LEFT_RIGHT:
    case GUI_SD_RIGHT_LEFT:
        if ( idx == 0 )
        {
            gui_element_set_rel_pos(
                child, parent,
                GUI_RECT_MARGIN,
                GUI_RelPosX(sl->sd - 1), (GUI_RelPosY)sl->sa,
                GUI_RelPosX(sl->sd - 1), (GUI_RelPosY)sl->sa);
            break;
        }
        gui_element_set_rel_pos(
            child, prev_child,
            GUI_RECT_MARGIN,
            GUI_RelPosX(inv_sd - 1), (GUI_RelPosY)sl->sa,
            GUI_RelPosX(sl->sd - 1), (GUI_RelPosY)sl->sa);
        break;
    }
    return true;
}

GUI_Element *gui_stack_layout_new(GUI_StackDir direction,
                                  GUI_StackAlign alignment,
                                  int x, int y, int w, int h,
                                  GUI_App *app, Nst_OpErr *err)
{
    GUI_StackLayout *new_sl = (GUI_StackLayout *)gui_element_new(
        GUI_ET_STACK_LAYOUT,
        sizeof(GUI_StackLayout),
        x, y, w, h,
        app, err);
    if ( new_sl == nullptr )
    {
        return nullptr;
    }
    new_sl->sd = direction;
    new_sl->sa = alignment;
    new_sl->on_child_added_func = (OnChildAdded)gui_stack_layout_on_child_added;
    return (GUI_Element *)new_sl;
}
