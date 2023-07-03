#include "gui_element.h"
#include "nest_gui.h"

GUI_Element *gui_element_new(GUI_ElementType t,
                             usize size,
                             int x, int y,
                             int w, int h,
                             struct _GUI_App *app)
{
    GUI_Element *obj = (GUI_Element *)_nst_obj_alloc(
        size,
        gui_element_type,
        (Nst_ObjDestructor)gui_element_destroy);

    if ( obj == nullptr )
    {
        return nullptr;
    }
    obj->rect = { x, y, w, h };
    obj->clip_content = false;
    obj->clip_parent = false;

    obj->el_type = t;

    obj->parent = nullptr;
    obj->children = SEQ(nst_vector_new(0));
    if ( obj->children == nullptr )
    {
        nst_free(obj);
        return nullptr;
    }
    obj->app = app;

    obj->handle_event_func = nullptr;
    obj->frame_update_func = nullptr;
    obj->tick_update_func = nullptr;
    obj->on_child_added_func = nullptr;

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
    nst_inc_ref(parent);

    if ( NST_OBJ_IS_TRACKED(obj) )
    {
        nst_ggc_track_obj(GGC_OBJ(parent));
    }
}

int gui_element_get_content_x(GUI_Element *obj, GUI_RelPosX pos, GUI_RelRect r)
{
    SDL_Rect rect;

    switch ( r )
    {
    case GUI_RECT_MARGIN:
        rect = gui_element_get_margin_rect(obj);
        break;
    case GUI_RECT_ELEMENT:
        rect = obj->rect;
        break;
    default:
        rect = gui_element_get_padding_rect(obj);
        break;
    }

    switch ( pos )
    {
    case GUI_RIGHT:
        return rect.x + rect.w;
    case GUI_MIDDLE:
        return rect.x + (rect.w / 2);
    default:
        return rect.x;
    }
}

int gui_element_get_content_y(GUI_Element *obj, GUI_RelPosY pos, GUI_RelRect r)
{
    SDL_Rect rect;

    switch ( r )
    {
    case GUI_RECT_MARGIN:
        rect = gui_element_get_margin_rect(obj);
        break;
    case GUI_RECT_ELEMENT:
        rect = obj->rect;
        break;
    default:
        rect = gui_element_get_padding_rect(obj);
        break;
    }

    switch ( pos )
    {
    case GUI_RIGHT:
        return rect.y + rect.h;
    case GUI_MIDDLE:
        return rect.y + (rect.h / 2);
    default:
        return rect.y;
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
                             GUI_RelRect rel_pos_rect,
                             GUI_RelPosX from_x, GUI_RelPosY from_y,
                             GUI_RelPosX to_x, GUI_RelPosY to_y)
{
    NST_FLAG_SET(obj, GUI_FLAG_REL_POS);
    obj->rel_pos.rel_pos_rect = rel_pos_rect;
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
                              GUI_RelRect rel_size_rect,
                              i32 min_w, i32 min_h,
                              i32 max_w, i32 max_h,
                              f64 scale_x, f64 scale_y,
                              i32 diff_x, i32 diff_y)
{
    NST_FLAG_SET(obj, GUI_FLAG_REL_SIZE);
    obj->rel_size.rel_size_rect = rel_size_rect;
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
        int x = gui_element_get_content_x(obj->rel_pos.element, from_x, obj->rel_pos.rel_pos_rect);
        gui_element_set_x(obj, obj->rel_pos.to_x, x);
    }
    if ( from_y >= GUI_TOP && from_y <= GUI_BOTTOM )
    {
        int y = gui_element_get_content_y(obj->rel_pos.element, from_y, obj->rel_pos.rel_pos_rect);
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
    GUI_Element *rs_element = obj->rel_size.element;

    if ( rs_element == nullptr )
    {
        SDL_GetWindowSize(obj->app->window, &p_w, &p_h);
    }
    else
    {
        p_w = rs_element->rect.w;
        p_h = rs_element->rect.h;

        if ( obj->rel_size.rel_size_rect == GUI_RECT_MARGIN )
        {
            p_w += rs_element->margin_left + rs_element->margin_right;
            p_h += rs_element->margin_top  + rs_element->margin_bottom;
        }
        else if ( obj->rel_size.rel_size_rect == GUI_RECT_PADDING )
        {
            p_w -= rs_element->padding_left + rs_element->padding_right;
            p_h -= rs_element->padding_top  + rs_element->padding_bottom;
        }
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

bool gui_element_add_child(GUI_Element *parent, GUI_Element *child)
{
    if ( !nst_vector_append(parent->children, child) )
    {
        return false;
    }
    gui_element_set_parent(child, parent);

    if ( parent->on_child_added_func != nullptr )
    {
        return parent->on_child_added_func(parent, parent->children->len - 1);
    }
    return true;
}

void gui_element_clip_parent(GUI_Element *element, bool clip)
{
    element->clip_parent = clip;
}

void gui_element_clip_content(GUI_Element *element, bool clip)
{
    for ( usize i = 0, n = element->children->len; i < n; i++ )
    {
        GUI_Element *child = (GUI_Element *)element->children->objs[i];
        gui_element_clip_parent(child, clip);
    }
    element->clip_content = clip;
}

TTF_Font *get_font(struct _GUI_App *app,
                   GUI_FontSize     size,
                   GUI_FontStyle    style,
                   GUI_FontWeight   weight)
{
    TTF_Font *font;
    TTF_Font **font_var;

    switch ( size )
    {
    case GUI_FSZ_BIG:
        switch ( weight )
        {
        case GUI_FW_BOLD:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->bold_italic_big;
                break;
            default:
                font_var = &app->bold_big;
                break;
            }
            break;
        default:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->italic_big;
                break;
            default:
                font_var = &app->regular_big;
                break;
            }
            break;
        }
        break;
    case GUI_FSZ_SMALL:
        switch ( weight )
        {
        case GUI_FW_BOLD:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->bold_italic_small;
                break;
            default:
                font_var = &app->bold_small;
                break;
            }
            break;
        default:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->italic_small;
                break;
            default:
                font_var = &app->regular_small;
                break;
            }
            break;
        }
        break;
    default:
        switch ( weight )
        {
        case GUI_FW_BOLD:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->bold_italic_medium;
                break;
            default:
                font_var = &app->bold_medium;
                break;
            }
            break;
        default:
            switch( style )
            {
            case GUI_FST_ITALIC:
                font_var = &app->italic_medium;
                break;
            default:
                font_var = &app->regular_medium;
                break;
            }
            break;
        }
        break;
    }
    font = *font_var;

    if ( font != nullptr )
    {
        return font;
    }

    Nst_StrObj *font_path;

    switch ( weight )
    {
    case GUI_FW_BOLD:
        switch( style )
        {
        case GUI_FST_ITALIC:
            font_path = _nst_get_import_path((i8 *)"font/osbi.ttf", 13);
            break;
        default:
            font_path = _nst_get_import_path((i8 *)"font/osbr.ttf", 13);
            break;
        }
        break;
    default:
        switch( style )
        {
        case GUI_FST_ITALIC:
            font_path = _nst_get_import_path((i8 *)"font/osri.ttf", 13);
            break;
        default:
            font_path = _nst_get_import_path((i8 *)"font/osrr.ttf", 13);
            break;
        }
        break;
    }
    int ptsize;

    if ( font_path == nullptr )
    {
        return nullptr;
    }

    switch ( size )
    {
    case GUI_FSZ_BIG:
        ptsize = 18;
        break;
    case GUI_FSZ_SMALL:
        ptsize = 13;
        break;
    default:
        ptsize = 15;
        break;
    }

    font = TTF_OpenFont(font_path->value, ptsize);
    *font_var = font;
    return font;
}
