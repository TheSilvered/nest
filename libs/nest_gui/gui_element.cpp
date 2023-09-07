#include "gui_element.h"
#include "nest_gui.h"

GUI_Element *gui_element_new(GUI_ElementType t, usize size, int x, int y,
                             int w, int h, struct _GUI_App *app,
                             void (*el_destructor)(void *))
{
    GUI_Element *obj = (GUI_Element *)_Nst_obj_alloc(size, gui_element_type);
    if (obj == nullptr)
        return nullptr;
    Nst_GGC_OBJ_INIT(obj);

    obj->rect = { x, y, w, h };
    obj->clip_content = false;
    obj->clip_parent = false;

    obj->el_type = t;

    obj->parent = nullptr;
    obj->children = SEQ(Nst_vector_new(0));
    if (obj->children == nullptr) {
        Nst_free(obj);
        return nullptr;
    }
    obj->app = app;

    obj->handle_event_func = nullptr;
    obj->frame_update_func = nullptr;
    obj->tick_update_func = nullptr;
    obj->el_destructor = el_destructor;

    gui_element_set_margin(obj, 0, 0, 0, 0);
    gui_element_set_padding(obj, 0, 0, 0, 0);

    return obj;
}

void gui_element_destroy(GUI_Element *obj)
{
    if (obj->el_destructor != nullptr)
        obj->el_destructor(obj);

    Nst_dec_ref(obj->children);
    if (obj->parent != nullptr)
        Nst_dec_ref(obj->parent);
    if (Nst_HAS_FLAG(obj, GUI_FLAG_REL_POS) && obj->rel_pos.element != nullptr)
        Nst_dec_ref(obj->rel_pos.element);
    if (Nst_HAS_FLAG(obj, GUI_FLAG_REL_SIZE)
        && obj->rel_size.element != nullptr)
    {
        Nst_dec_ref(obj->rel_size.element);
    }
}

void gui_element_traverse(GUI_Element *obj)
{
    Nst_ggc_obj_reachable(obj->children);
    if (obj->parent != nullptr)
        Nst_ggc_obj_reachable(obj->parent);
    if (Nst_HAS_FLAG(obj, GUI_FLAG_REL_POS) && obj->rel_pos.element != nullptr)
        Nst_ggc_obj_reachable(obj->rel_pos.element);
    if (Nst_HAS_FLAG(obj, GUI_FLAG_REL_SIZE)
        && obj->rel_size.element != nullptr)
    {
        Nst_ggc_obj_reachable(obj->rel_size.element);
    }
}

void gui_element_set_margin(GUI_Element *obj, i32 margin_top, i32 margin_left,
                            i32 margin_bottom, i32 margin_right)
{
    if (margin_top >= 0)
        obj->margin_top = margin_top;
    if (margin_left >= 0)
        obj->margin_left = margin_left;
    if (margin_bottom >= 0)
        obj->margin_bottom = margin_bottom;
    if (margin_right >= 0)
        obj->margin_right = margin_right;
}

void gui_element_set_padding(GUI_Element *obj, i32 padding_top,
                             i32 padding_left, i32 padding_bottom,
                             i32 padding_right)
{
    if (padding_top >= 0)
        obj->padding_top = padding_top;
    if (padding_left >= 0)
        obj->padding_left = padding_left;
    if (padding_bottom >= 0)
        obj->padding_bottom = padding_bottom;
    if (padding_right >= 0)
        obj->padding_right = padding_right;
}

void gui_element_set_parent(GUI_Element *obj, GUI_Element *parent)
{
    if (obj->parent != nullptr)
        Nst_dec_ref(obj->parent);

    obj->parent = parent;
    Nst_inc_ref(parent);
}

int gui_element_get_content_x(GUI_Element *obj, GUI_RelPosX pos, GUI_RelRect r)
{
    SDL_Rect rect;

    switch (r) {
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

    switch (pos) {
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

    switch (r) {
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

    switch (pos) {
    case GUI_RIGHT:
        return rect.y + rect.h;
    case GUI_MIDDLE:
        return rect.y + (rect.h / 2);
    default:
        return rect.y;
    }
}

static void set_margin_pos(GUI_Element *obj,
                           GUI_RelPosX pos_x, int x,
                           GUI_RelPosY pos_y, int y)
{
    switch (pos_x) {
    case GUI_LEFT:
        obj->rect.x = x + obj->margin_left;
        break;
    case GUI_MIDDLE:
        obj->rect.x = x
            + obj->margin_left
            - (obj->rect.w + obj->margin_left + obj->margin_right) / 2;
        break;
    case GUI_RIGHT:
        obj->rect.x = x - obj->rect.w - obj->margin_right;
        break;
    default: break;
    }

    switch (pos_y) {
    case GUI_TOP:
        obj->rect.y = y + obj->margin_top;
        break;
    case GUI_CENTER:
        obj->rect.y = y
            + obj->margin_top
            - (obj->rect.h + obj->margin_top + obj->margin_bottom) / 2;
        break;
    case GUI_BOTTOM:
        obj->rect.y = y - obj->rect.h - obj->margin_bottom;
        break;
    default: break;
    }
}

static void set_padding_pos(GUI_Element *obj,
                            GUI_RelPosX pos_x, int x,
                            GUI_RelPosY pos_y, int y)
{
    switch (pos_x) {
    case GUI_LEFT:
        obj->rect.x = x - obj->padding_left;
        break;
    case GUI_MIDDLE:
        obj->rect.x = x
            - obj->padding_left
            - (obj->rect.w - obj->padding_left - obj->padding_right) / 2;
        break;
    case GUI_RIGHT:
        obj->rect.x = x - obj->rect.w - obj->padding_left;
        break;
    default: break;
    }

    switch (pos_y) {
    case GUI_TOP:
        obj->rect.y = y - obj->padding_top;
        break;
    case GUI_CENTER:
        obj->rect.y = y
            - obj->padding_top
            - (obj->rect.h - obj->padding_top - obj->padding_bottom) / 2;
        break;
    case GUI_BOTTOM:
        obj->rect.y = y - obj->rect.h - obj->padding_top;
        break;
    default: break;
    }
}

static void set_element_pos(GUI_Element *obj,
                            GUI_RelPosX pos_x, int x,
                            GUI_RelPosY pos_y, int y)
{
    switch (pos_x) {
    case GUI_LEFT:
        obj->rect.x = x;
        break;
    case GUI_MIDDLE:
        obj->rect.x = x - obj->rect.w / 2;
        break;
    case GUI_RIGHT:
        obj->rect.x = x - obj->rect.w;
        break;
    default: break;
    }

    switch (pos_y) {
    case GUI_TOP:
        obj->rect.y = y;
        break;
    case GUI_MIDDLE:
        obj->rect.y = y - obj->rect.h / 2;
        break;
    case GUI_RIGHT:
        obj->rect.y = y - obj->rect.y;
        break;
    default: break;
    }
}

void gui_element_set_pos(GUI_Element *obj,
                         GUI_RelPosX pos_x, int x,
                         GUI_RelPosY pos_y, int y,
                         GUI_RelRect rect)
{
    int prev_x = obj->rect.x;
    int prev_y = obj->rect.y;

    if (rect == GUI_RECT_MARGIN)
        set_margin_pos(obj, pos_x, x, pos_y, y);
    else if (rect == GUI_RECT_PADDING)
        set_padding_pos(obj, pos_x, x, pos_y, y);
    else
        set_element_pos(obj, pos_x, x, pos_y, y);

    if (prev_x != obj->rect.x || prev_y != obj->rect.y) {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = GUI_UE_MOVED;
        event.user.data1 = obj;
        SDL_PushEvent(&event);
    }
}

void gui_element_set_size(GUI_Element *obj, int w, int h, GUI_RelRect rect)
{
    int prev_w = obj->rect.w;
    int prev_h = obj->rect.h;

    if (w >= 0) {
        if (rect == GUI_RECT_MARGIN)
            obj->rect.w = imax(w, obj->margin_left + obj->margin_right)
                        - obj->margin_left - obj->margin_right;
        else if (rect == GUI_RECT_PADDING)
            obj->rect.w = w + obj->padding_left + obj->padding_right;
        else
            obj->rect.w = w;
    }

    if (h >= 0) {
        if (rect == GUI_RECT_MARGIN)
            obj->rect.h = imax(h, obj->margin_top + obj->margin_bottom)
                - obj->margin_top - obj->margin_bottom;
        else if (rect == GUI_RECT_PADDING)
            obj->rect.h = h + obj->padding_top + obj->padding_bottom;
        else
            obj->rect.h = h;
    }

    if (prev_w != obj->rect.w || prev_h != obj->rect.h) {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = GUI_UE_RESIZED;
        event.user.data1 = obj;
        SDL_PushEvent(&event);
    }
}

void gui_element_set_rel_pos(GUI_Element *obj, GUI_Element *element,
                             GUI_RelRect from_rect, GUI_RelRect to_rect,
                             GUI_RelPosX from_x, GUI_RelPosY from_y,
                             GUI_RelPosX to_x, GUI_RelPosY to_y)
{
    Nst_SET_FLAG(obj, GUI_FLAG_REL_POS);
    obj->rel_pos.from_rect = from_rect;
    obj->rel_pos.to_rect = to_rect;
    obj->rel_pos.from_x = from_x;
    obj->rel_pos.from_y = from_y;
    obj->rel_pos.to_x = to_x;
    obj->rel_pos.to_y = to_y;
    obj->rel_pos.element = element;

    if (element != nullptr)
        Nst_inc_ref(element);
    else {
        obj->rel_pos.element = obj->app->root;
        Nst_inc_ref(obj->app->root);
    }
}

void gui_element_set_rel_size(GUI_Element *obj, GUI_Element *element,
                              GUI_RelRect from_rect, GUI_RelRect to_rect,
                              i32 min_w, i32 min_h, i32 max_w, i32 max_h,
                              f64 scale_x, f64 scale_y, i32 diff_x, i32 diff_y)
{
    Nst_SET_FLAG(obj, GUI_FLAG_REL_SIZE);
    obj->rel_size.from_rect = from_rect;
    obj->rel_size.to_rect = to_rect;
    obj->rel_size.min_w = min_w;
    obj->rel_size.min_h = min_h;
    obj->rel_size.max_w = max_w;
    obj->rel_size.max_h = max_h;
    obj->rel_size.scale_x = scale_x;
    obj->rel_size.scale_y = scale_y;
    obj->rel_size.diff_x = diff_x;
    obj->rel_size.diff_y = diff_y;
    obj->rel_size.element = element;

    if (element != nullptr)
        Nst_inc_ref(element);
}

void gui_element_update_pos(GUI_Element *obj)
{
    if (!Nst_HAS_FLAG(obj, GUI_FLAG_REL_POS))
        return;

    GUI_RelPosX from_x = obj->rel_pos.from_x;
    GUI_RelPosY from_y = obj->rel_pos.from_y;
    GUI_RelPosX to_x = obj->rel_pos.to_x;
    GUI_RelPosY to_y = obj->rel_pos.to_y;

    int x, y;
    if (from_x >= GUI_LEFT && from_x <= GUI_RIGHT) {
        x = gui_element_get_content_x(
            obj->rel_pos.element,
            from_x,
            obj->rel_pos.from_rect);
    } else {
        x = obj->rect.x;
        to_x = (GUI_RelPosX)-1;
    }

    if (from_y >= GUI_TOP && from_y <= GUI_BOTTOM) {
        y = gui_element_get_content_y(
            obj->rel_pos.element,
            from_y,
            obj->rel_pos.from_rect);
    } else {
        y = obj->rect.y;
        to_y = (GUI_RelPosY)-1;
    }
    gui_element_set_pos(obj, to_x, x, to_y, y, obj->rel_pos.to_rect);
}

void gui_element_update_size(GUI_Element *obj)
{
    if (!Nst_HAS_FLAG(obj, GUI_FLAG_REL_SIZE))
        return;

    int p_w, p_h;
    GUI_Element *rs_element = obj->rel_size.element;

    if (rs_element == nullptr)
        SDL_GetWindowSize(obj->app->window, &p_w, &p_h);
    else {
        p_w = rs_element->rect.w;
        p_h = rs_element->rect.h;

        if (obj->rel_size.from_rect == GUI_RECT_MARGIN) {
            p_w += rs_element->margin_left + rs_element->margin_right;
            p_h += rs_element->margin_top  + rs_element->margin_bottom;
        } else if (obj->rel_size.from_rect == GUI_RECT_PADDING) {
            p_w -= rs_element->padding_left + rs_element->padding_right;
            p_h -= rs_element->padding_top  + rs_element->padding_bottom;
        }
    }

    i32 diff_x = obj->rel_size.diff_x;
    i32 diff_y = obj->rel_size.diff_y;
    f64 scale_x = obj->rel_size.scale_x;
    f64 scale_y = obj->rel_size.scale_y;

    if (diff_x != 0)
        p_w += diff_x;
    else if (scale_x != 0.0)
        p_w = int(double(p_w) * scale_x);
    else
        p_w = -1;

    if (diff_y != 0)
        p_h += diff_y;
    else if (scale_y != 0.0)
        p_h = int(double(p_h) * scale_y);
    else
        p_h = -1;

    i32 min_w = obj->rel_size.min_w;
    i32 min_h = obj->rel_size.min_h;
    i32 max_w = obj->rel_size.max_w;
    i32 max_h = obj->rel_size.max_h;

    if (p_w < min_w && min_w > 0)
        p_w = min_w;
    if (p_w > max_w && max_w > 0)
        p_w = max_w;
    if (p_w < 0 && diff_x != 0 && scale_x != 0)
        p_w = 0;

    if (p_h < min_h && min_h > 0)
        p_h = min_h;
    if (p_h > max_h && max_h > 0)
        p_h = max_h;
    if (p_h < 0 && diff_y != 0 && scale_y != 0)
        p_h = 0;

    gui_element_set_size(obj, p_w, p_h, obj->rel_size.to_rect);
}

SDL_Rect gui_element_get_margin_rect(GUI_Element *obj)
{
    SDL_Rect r = obj->rect;
    r.x = r.x - obj->margin_left;
    r.y = r.y - obj->margin_top;
    r.w += obj->margin_left + obj->margin_right;
    r.h += obj->margin_top + obj->margin_bottom;
    if (r.w < 0 || r.h < 0) {
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
    if (r.w < 0 || r.h < 0) {
        r.w = 0;
        r.h = 0;
    }
    return r;
}

SDL_Rect gui_element_get_clip_rect(GUI_Element *obj)
{
    if (obj->clip_content && !obj->clip_parent)
        return gui_element_get_padding_rect(obj);
    if (obj->clip_content) {
        SDL_Rect clip;
        SDL_Rect parent_clip = gui_element_get_clip_rect(obj->parent);
        SDL_Rect padding_rect = gui_element_get_padding_rect(obj);
        SDL_IntersectRect(&parent_clip, &padding_rect, &clip);
        return clip;
    }
    if (obj->clip_parent)
        return gui_element_get_clip_rect(obj->parent);

    return obj->app->clip_window;
}

bool gui_element_add_child(GUI_Element *parent, GUI_Element *child)
{
    if (!Nst_vector_append(parent->children, child))
        return false;
    gui_element_set_parent(child, parent);

    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = GUI_UE_CHILD_ADDED;
    event.user.data1 = parent;
    event.user.data2 = (void *)(parent->children->len - 1);
    SDL_PushEvent(&event);

    return true;
}

bool gui_element_remove_child(GUI_Element *parent, GUI_Element *child)
{
    return Nst_vector_remove(parent->children, child);
}

void gui_element_clip_parent(GUI_Element *element, bool clip)
{
    element->clip_parent = clip;
}

void gui_element_clip_content(GUI_Element *element, bool clip)
{
    for (usize i = 0, n = element->children->len; i < n; i++) {
        GUI_Element *child = (GUI_Element *)element->children->objs[i];
        gui_element_clip_parent(child, clip);
    }
    element->clip_content = clip;
}

struct _GUI_FontObj *get_font(struct _GUI_App *app, GUI_FontWeight weight,
                              GUI_FontSize size, bool italic, bool monospace)
{
    usize idx = size * 16 + (monospace ? 8 : 0) + (italic ? 4 : 0) + weight;
    GUI_FontObj *font_obj = app->builtin_fonts[idx];
    if (font_obj != nullptr)
        return font_obj;

    i8 buf[23] = { 0 };
    strcat(buf, "font/");
    if (monospace)
        strcat(buf, "jbm-");
    else
        strcat(buf, "inter-");

    if (italic && monospace)
        strcat(buf, "italic-");
    strcat(buf, "0.ttf");

    usize buf_len = strlen(buf);
    buf[buf_len - 5] = '0' + weight;

    Nst_StrObj *font_path = _Nst_get_import_path(buf, buf_len);
    if (font_path == nullptr)
        return nullptr;

    int ptsize;
    switch (size) {
    case GUI_FS_SMALL:  ptsize = 11; break;
    case GUI_FS_MEDIUM: ptsize = 14; break;
    case GUI_FS_LARGE:  ptsize = 17; break;
    default: ptsize = 14;
    }
    TTF_Font *font = TTF_OpenFont(font_path->value, ptsize);
    Nst_dec_ref(font_path);
    if (font == nullptr) {
        Nst_failed_allocation();
        return nullptr;
    }
    if (italic && !monospace)
        TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
    font_obj = (GUI_FontObj *)gui_font_new(font);
    if (font_obj == nullptr) {
        TTF_CloseFont(font);
        return nullptr;
    }

    app->builtin_fonts[idx] = font_obj;
    return font_obj;
}
