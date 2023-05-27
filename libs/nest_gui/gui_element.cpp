#include "gui_element.h"
#include "nest_gui.h"

GUI_Element *gui_element_new(GUI_ElementType t,
                             usize size,
                             int x, int y,
                             int w, int h,
                             struct _GUI_App *app,
                             Nst_OpErr *err)
{
    GUI_Element *obj = (GUI_Element *)nst_obj_alloc(
        size,
        gui_element_type,
        gui_element_destroy,
        err);
    if ( obj == nullptr )
    {
        return nullptr;
    }
    obj->rect.x = x;
    obj->rect.y = y;
    obj->rect.w = w;
    obj->rect.h = h;
    obj->el_type = GUI_ET_BASE;

    obj->parent = nullptr;
    obj->children = SEQ(nst_vector_new(0, err));
    if ( obj->children == nullptr )
    {
        nst_free(obj);
        return nullptr;
    }
    obj->app = app;

    obj->handle_event_func = nullptr;
    obj->frame_update_func = nullptr;
    obj->tick_update_func = nullptr;

    gui_element_set_margin(obj, 0, 0, 0, 0);
    gui_element_set_padding(obj, 0, 0, 0, 0);

    NST_GGC_OBJ_INIT(obj, gui_element_track, gui_element_traverse);

    return obj;
}

void gui_element_destroy(GUI_Element *obj)
{
    nst_dec_ref(obj->children);
    if ( obj->parent != nullptr )
    {
        nst_dec_ref(obj->parent);
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_POS) && obj->rel_pos.element != nullptr )
    {
        nst_dec_ref(obj->rel_pos.element);
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_SIZE) && obj->rel_size.element != nullptr )
    {
        nst_dec_ref(obj->rel_size.element);
    }
}

void gui_element_track(GUI_Element *obj)
{
    nst_ggc_track_obj(GGC_OBJ(obj->children));
    if ( obj->parent != nullptr )
    {
        nst_ggc_track_obj(GGC_OBJ(obj->parent));
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_POS) && obj->rel_pos.element != nullptr )
    {
        nst_ggc_track_obj(GGC_OBJ(obj->rel_pos.element));
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_SIZE) && obj->rel_size.element != nullptr )
    {
        nst_ggc_track_obj(GGC_OBJ(obj->rel_size.element));
    }
}

void gui_element_traverse(GUI_Element *obj)
{
    _nst_seq_traverse(obj->children);
    NST_FLAG_SET(obj->children, NST_FLAG_GGC_REACHABLE);
    if ( obj->parent != nullptr )
    {
        NST_FLAG_SET(obj->parent, NST_FLAG_GGC_REACHABLE);
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_POS) && obj->rel_pos.element != nullptr )
    {
        NST_FLAG_SET(obj->rel_pos.element, NST_FLAG_GGC_REACHABLE);
    }
    if ( NST_FLAG_HAS(obj, GUI_FLAG_REL_SIZE) && obj->rel_size.element != nullptr )
    {
        NST_FLAG_SET(obj->rel_size.element, NST_FLAG_GGC_REACHABLE);
    }
}

void gui_element_set_margin(GUI_Element *obj,
                            i32 margin_top,
                            i32 margin_left,
                            i32 margin_bottom,
                            i32 margin_right)
{
    obj->margin_top = margin_top;
    obj->margin_left = margin_left;
    obj->margin_bottom = margin_bottom;
    obj->margin_right = margin_right;
}

void gui_element_set_padding(GUI_Element *obj,
                             i32 padding_top,
                             i32 padding_left,
                             i32 padding_bottom,
                             i32 padding_right)
{
    obj->padding_top = padding_top;
    obj->padding_left = padding_left;
    obj->padding_bottom = padding_bottom;
    obj->padding_right = padding_right;
}

void gui_element_set_parent(GUI_Element *obj, GUI_Element *parent)
{
    if ( obj->parent != nullptr )
    {
        nst_dec_ref(obj->parent);
    }

    obj->parent = parent;

    if ( NST_OBJ_IS_TRACKED(obj) )
    {
        nst_ggc_track_obj(GGC_OBJ(parent));
    }
}

int gui_element_get_content_x(GUI_Element *obj, GUI_RelPosX pos)
{
    switch ( pos )
    {
    case GUI_LEFT:
        return obj->rect.x + obj->padding_left;
    case GUI_MIDDLE:
        return obj->rect.x
             + obj->padding_left
             + ((obj->rect.w - obj->padding_left - obj->padding_right) >> 1);
    default:
        return obj->rect.x + obj->rect.w - obj->padding_right;
    }
}

int gui_element_get_content_y(GUI_Element *obj, GUI_RelPosY pos)
{
    switch ( pos )
    {
    case GUI_TOP:
        return obj->rect.y + obj->padding_top;
    case GUI_CENTER:
        return obj->rect.y
             + obj->padding_top
             + ((obj->rect.h - obj->padding_top - obj->padding_bottom) >> 1);
    default:
        return obj->rect.y + obj->rect.h - obj->padding_bottom;
    }
}

void gui_element_set_x(GUI_Element *obj, GUI_RelPosX pos, int x)
{
    switch ( pos )
    {
    case GUI_LEFT:
        obj->rect.x = x + obj->margin_left;
        break;
    case GUI_MIDDLE:
        obj->rect.x = x
            + obj->margin_left
            - ((obj->rect.w + obj->margin_left + obj->margin_right) >> 1);
        break;
    case GUI_RIGHT:
        obj->rect.x = x - obj->rect.w - obj->margin_right;
        break;
    default:
        obj->rect.x = x;
    }
}

void gui_element_set_y(GUI_Element *obj, GUI_RelPosY pos, int y)
{
    switch ( pos )
    {
    case GUI_TOP:
        obj->rect.y = y + obj->margin_top;
        break;
    case GUI_CENTER:
        obj->rect.y = y
            + obj->margin_top
            - ((obj->rect.h + obj->margin_top + obj->margin_bottom) >> 1);
        break;
    case GUI_BOTTOM:
        obj->rect.y = y - obj->rect.h - obj->margin_bottom;
        break;
    default:
        obj->rect.y = y;
    }
}

void gui_element_set_rel_pos(GUI_Element *obj,
                             GUI_Element *element,
                             GUI_RelPosX from_x, GUI_RelPosY from_y,
                             GUI_RelPosX to_x, GUI_RelPosY to_y)
{
    NST_FLAG_SET(obj, GUI_FLAG_REL_POS);
    obj->rel_pos.from_x = from_x;
    obj->rel_pos.from_y = from_y;
    obj->rel_pos.to_x = to_x;
    obj->rel_pos.to_y = to_y;
    obj->rel_pos.element = element;

    if ( element != nullptr )
    {
        nst_inc_ref(element);
        if ( NST_OBJ_IS_TRACKED(obj) )
        {
            nst_ggc_track_obj(GGC_OBJ(element));
        }
    }
    else
    {
        obj->rel_pos.element = obj->app->root;
        nst_inc_ref(obj->app->root);
        if ( NST_OBJ_IS_TRACKED(obj) )
        {
            nst_ggc_track_obj(GGC_OBJ(obj->app->root));
        }
    }
}

void gui_element_set_rel_size(GUI_Element *obj,
                              GUI_Element *element,
                              i32 min_w, i32 min_h,
                              i32 max_w, i32 max_h,
                              f64 scale_x, f64 scale_y,
                              i32 diff_x, i32 diff_y)
{
    NST_FLAG_SET(obj, GUI_FLAG_REL_SIZE);
    obj->rel_size.min_w = min_w;
    obj->rel_size.min_h = min_h;
    obj->rel_size.max_w = max_w;
    obj->rel_size.max_h = max_h;
    obj->rel_size.scale_x = scale_x;
    obj->rel_size.scale_y = scale_y;
    obj->rel_size.diff_x = diff_x;
    obj->rel_size.diff_y = diff_y;
    obj->rel_size.element = element;

    if ( element != nullptr )
    {
        nst_inc_ref(element);
        if ( NST_OBJ_IS_TRACKED(obj) )
        {
            nst_ggc_track_obj(GGC_OBJ(element));
        }
    }
}

void gui_element_update_pos(GUI_Element *obj)
{
    if ( !NST_FLAG_HAS(obj, GUI_FLAG_REL_POS) )
    {
        return;
    }

    GUI_RelPosX from_x = obj->rel_pos.from_x;
    GUI_RelPosY from_y = obj->rel_pos.from_y;

    if ( from_x >= GUI_LEFT && from_x <= GUI_RIGHT )
    {
        int x = gui_element_get_content_x(obj->rel_pos.element, from_x);
        gui_element_set_x(obj, obj->rel_pos.to_x, x);
    }
    if ( from_y >= GUI_TOP && from_y <= GUI_BOTTOM )
    {
        int y = gui_element_get_content_y(obj->rel_pos.element, from_y);
        gui_element_set_y(obj, obj->rel_pos.to_y, y);
    }
}

void gui_element_update_size(GUI_Element *obj)
{
    if ( !NST_FLAG_HAS(obj, GUI_FLAG_REL_SIZE) )
    {
        return;
    }

    int p_w, p_h;
    if ( obj->rel_size.element == nullptr )
    {
        SDL_GetWindowSize(obj->app->window, &p_w, &p_h);
    }
    else
    {
        p_w = obj->rel_size.element->rect.w;
        p_h = obj->rel_size.element->rect.h;
    }

    if ( obj->rel_size.diff_x != 0 )
    {
        p_w += obj->rel_size.diff_x;
    }
    else if ( obj->rel_size.scale_x != 0.0 )
    {
        p_w = int(double(p_w) * obj->rel_size.scale_x);
    }
    else
    {
        p_w = obj->rect.w;
    }

    if ( obj->rel_size.diff_y != 0 )
    {
        p_h += obj->rel_size.diff_y;
    }
    else if ( obj->rel_size.scale_y != 0.0 )
    {
        p_h = int(double(p_h) * obj->rel_size.scale_y);
    }
    else
    {
        p_h = obj->rect.h;
    }

    i32 min_w = obj->rel_size.min_w;
    i32 min_h = obj->rel_size.min_h;
    i32 max_w = obj->rel_size.max_w;
    i32 max_h = obj->rel_size.max_h;

    if ( p_w < min_w && min_w > 0 )
    {
        p_w = min_w;
    }
    if ( p_w > max_w && max_w > 0 )
    {
        p_w = max_w;
    }

    if ( p_h < min_h && min_h > 0 )
    {
        p_h = min_h;
    }
    if ( p_h > max_h && max_h > 0 )
    {
        p_h = max_h;
    }

    if ( p_w < 0 ) p_w = 0;
    if ( p_h < 0 ) p_h = 0;

    obj->rect.w = p_w;
    obj->rect.h = p_h;
}

SDL_Rect gui_element_get_margin_rect(GUI_Element *obj)
{
    SDL_Rect r = obj->rect;
    r.x = r.x - obj->margin_left;
    r.y = r.y - obj->margin_top;
    r.w += obj->margin_left + obj->margin_right;
    r.h += obj->margin_top + obj->margin_bottom;
    if ( r.w < 0 || r.h < 0 )
    {
        r.w = 0;
        r.h = 0;
    }
    return r;
}

SDL_Rect gui_element_get_padding_rect(GUI_Element *obj)
{
    SDL_Rect r = obj->rect;
    r.x = r.x + obj->padding_left;
    r.y = r.y + obj->padding_top;
    r.w -= obj->padding_left + obj->padding_right;
    r.h -= obj->padding_top + obj->padding_bottom;
    if ( r.w < 0 || r.h < 0 )
    {
        r.w = 0;
        r.h = 0;
    }
    return r;
}

bool gui_element_add_child(GUI_Element *parent, GUI_Element *child, Nst_OpErr *err)
{
    nst_vector_append(parent->children, child, err);
    if ( NST_ERROR_OCCURRED )
    {
        return false;
    }
    gui_element_set_parent(child, parent);
    return true;
}
