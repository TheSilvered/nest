#include "gui_element.h"
#include "nest_gui.h"

using namespace GUI;

static Nst_TypeObj *element_type = nullptr;
static Nst_TypeObj *font_type = nullptr;

bool GUI::element_init()
{
    element_type = Nst_cont_type_new(
        "GUI Element",
        (Nst_ObjDstr)element_destroy,
        (Nst_ObjTrav)element_traverse);
    if (element_type == nullptr)
        return false;

    font_type = Nst_type_new("GUI Font", (Nst_ObjDstr)font_destroy);
    if (font_type == nullptr) {
        Nst_dec_ref(element_type);
        return false;
    }
    return true;
}

void GUI::element_quit()
{
    Nst_dec_ref(element_type);
    Nst_dec_ref(font_type);
}

Element *GUI::element_new(ElementType t, usize size, int x, int y, int w, int h,
                          struct _App *app, void (*el_destructor)(void *))
{
    Nst_Obj *el_t = get_element_type();
    if (el_t == nullptr)
        return nullptr;

    Element *obj = (Element *)_Nst_obj_alloc(size, TYPE(el_t));
    if (obj == nullptr)
        return nullptr;
    Nst_GGC_OBJ_INIT(obj);

    obj->rect = { x, y, w, h };
    obj->clip_content = false;

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
    obj->animations = MAP(Nst_map_new());
    if (obj->animations == nullptr) {
        Nst_dec_ref(obj->children);
        Nst_free(obj);
        return nullptr;
    }

    element_set_padding(obj, 0, 0, 0, 0);

    return obj;
}

Nst_Obj *GUI::get_element_type()
{
    return OBJ(element_type);
}

void GUI::element_destroy(Element *obj)
{
    if (obj->el_destructor != nullptr)
        obj->el_destructor(obj);

    Nst_dec_ref(obj->children);
    Nst_dec_ref(obj->animations);

    if (obj->parent != nullptr)
        Nst_dec_ref(obj->parent);
}

void GUI::element_traverse(Element *obj)
{
    Nst_ggc_obj_reachable(obj->children);
    if (obj->parent != nullptr)
        Nst_ggc_obj_reachable(obj->parent);
}

i32 clamp_padding(i32 padding, int w_or_h)
{
    if (padding < 0)
        padding = 0;
    if (padding > w_or_h / 2)
        padding = w_or_h / 2;
    return padding;
}

void GUI::element_set_padding(Element *obj, i32 padding_top, i32 padding_left,
                              i32 padding_bottom, i32 padding_right)
{
    i32 prev_top = obj->padding_top;
    i32 prev_bottom = obj->padding_bottom;
    i32 prev_left = obj->padding_left;
    i32 prev_right = obj->padding_right;

    obj->padding_top = padding_top < 0 ? 0 : padding_top;
    obj->padding_bottom = padding_bottom < 0 ? 0 : padding_bottom;
    obj->padding_left = padding_left < 0 ? 0 : padding_left;
    obj->padding_right = padding_right < 0 ? 0 : padding_right;

    if (prev_top  != obj->padding_top  || prev_bottom != obj->padding_bottom ||
        prev_left != obj->padding_left || prev_right  != obj->padding_right)
    {
        SDL_Event event;
        event.type = SDL_USEREVENT;
        event.user.code = GUI_UE_CHANGED_PADDING;
        event.user.data1 = obj;
        SDL_PushEvent(&event);
    }
}

void GUI::element_get_padding(Element *obj, i32 &padding_top, i32 &padding_left,
                              i32 &padding_bottom, i32 &padding_right)
{
    padding_top = clamp_padding(obj->padding_top, obj->rect.h);
    padding_bottom = clamp_padding(obj->padding_bottom, obj->rect.h);
    padding_left = clamp_padding(obj->padding_left, obj->rect.w);
    padding_right = clamp_padding(obj->padding_right, obj->rect.w);
}

void GUI::element_set_parent(Element *obj, Element *parent)
{
    if (obj->parent != nullptr)
        Nst_dec_ref(obj->parent);

    obj->parent = parent;
    Nst_inc_ref(parent);
}

int GUI::element_get_x(Element *obj, Side side)
{
    SDL_Rect rect;

    if (obj == nullptr)
        rect = get_global_app()->clip_window;
    else if (side >= P_LEFT)
        rect = element_get_padding_rect(obj);
    else
        rect = obj->rect;

    switch (side) {
    case LEFT:
    case P_LEFT:
    case TOP:
    case P_TOP:
    case BOTTOM:
    case P_BOTTOM:
    default:
        return rect.x;
    case RIGHT:
    case P_RIGHT:
        return rect.x + rect.w;
    case CENTER:
    case P_CENTER:
        return rect.x + rect.w / 2;
    }
}

int GUI::element_get_y(Element *obj, Side side)
{
    SDL_Rect rect;

    if (obj == nullptr)
        rect = get_global_app()->clip_window;
    else if (side >= P_LEFT)
        rect = element_get_padding_rect(obj);
    else
        rect = obj->rect;

    switch (side) {
    case TOP:
    case P_TOP:
    case LEFT:
    case P_LEFT:
    case RIGHT:
    case P_RIGHT:
    default:
        return rect.y;
    case BOTTOM:
    case P_BOTTOM:
        return rect.y + rect.h;
    case CENTER:
    case P_CENTER:
        return rect.y + rect.h / 2;
    }
}

void GUI::pos_to_sides(Pos pos, Side &x_side, Side &y_side)
{
    switch (pos) {
    case TL:
    case CL:
    case BL:
    default:
        x_side = LEFT;
        break;
    case P_TL:
    case P_CL:
    case P_BL:
        x_side = P_LEFT;
        break;
    case TC:
    case CC:
    case BC:
        x_side = CENTER;
        break;
    case P_TC:
    case P_CC:
    case P_BC:
        x_side = P_CENTER;
        break;
    case TR:
    case CR:
    case BR:
        x_side = RIGHT;
        break;
    case P_TR:
    case P_CR:
    case P_BR:
        x_side = P_RIGHT;
        break;
    }

    switch (pos) {
    case TL:
    case TC:
    case TR:
    default:
        y_side = TOP;
        break;
    case P_TL:
    case P_TC:
    case P_TR:
        y_side = P_TOP;
        break;
    case CL:
    case CC:
    case CR:
        y_side = CENTER;
        break;
    case P_CL:
    case P_CC:
    case P_CR:
        y_side = P_CENTER;
        break;
    case BL:
    case BC:
    case BR:
        y_side = BOTTOM;
        break;
    case P_BL:
    case P_BC:
    case P_BR:
        y_side = P_BOTTOM;
        break;
    }
}

void GUI::element_get_pos(Element *obj, Pos pos, int &x, int &y)
{
    Side x_side, y_side;
    pos_to_sides(pos, x_side, y_side);
    x = element_get_x(obj, x_side);
    y = element_get_y(obj, y_side);
}

int GUI::element_get_w(Element *obj, bool from_padding)
{
    if (obj == nullptr)
        return get_global_app()->clip_window.w;
    return from_padding ? element_get_padding_rect(obj).w: obj->rect.w;
}

int GUI::element_get_h(Element *obj, bool from_padding)
{
    if (obj == nullptr)
        return get_global_app()->clip_window.h;
    return from_padding ? element_get_padding_rect(obj).h: obj->rect.h;
}

void GUI::element_get_size(Element *obj, int &w, int &h, bool from_padding)
{
    SDL_Rect rect;

    if (obj == nullptr)
        rect = get_global_app()->clip_window;
    else if (from_padding)
        rect = element_get_padding_rect(obj);
    else
        rect = obj->rect;

    w = rect.w;
    h = rect.h;
}

static void push_moved_event(Element *obj)
{
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = GUI_UE_MOVED;
    event.user.data1 = obj;
    SDL_PushEvent(&event);
}

static void push_resized_event(Element *obj)
{
    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = GUI_UE_RESIZED;
    event.user.data1 = obj;
    SDL_PushEvent(&event);
}

static bool set_x_internal(Element *obj, Side side, int x)
{
    int prev_x = obj->rect.x;
    i32 p_left, p_right, p_top, p_bottom;
    element_get_padding(obj, p_top, p_left, p_bottom, p_right);

    switch (side) {
    case LEFT:
    case TOP:
    case BOTTOM:
    default:
        obj->rect.x = x;
        break;
    case RIGHT:
        obj->rect.x = x - obj->rect.w;
        break;
    case CENTER:
        obj->rect.x = x - obj->rect.w / 2;
        break;
    case P_LEFT:
    case P_TOP:
    case P_BOTTOM:
        obj->rect.x = x - p_left;
        break;
    case P_RIGHT:
        obj->rect.x = x - obj->rect.w + p_left;
        break;
    case P_CENTER:
        obj->rect.x =
            x - (obj->rect.w + p_left - p_right) / 2;
        break;
    }
    return prev_x != obj->rect.x;
}

void GUI::element_set_x(Element *obj, Side side, int x)
{
    if (set_x_internal(obj, side, x))
        push_moved_event(obj);
}

static bool set_y_internal(Element *obj, Side side, int y)
{
    int prev_y = obj->rect.y;
    i32 p_left, p_right, p_top, p_bottom;
    element_get_padding(obj, p_top, p_left, p_bottom, p_right);

    switch (side) {
    case TOP:
    case RIGHT:
    case LEFT:
    default:
        obj->rect.y = y;
        break;
    case BOTTOM:
        obj->rect.y = y - obj->rect.h;
        break;
    case CENTER:
        obj->rect.y = y - obj->rect.h / 2;
        break;
    case P_TOP:
    case P_LEFT:
    case P_RIGHT:
        obj->rect.y = y - p_top;
        break;
    case P_BOTTOM:
        obj->rect.y = y - obj->rect.y + p_top;
        break;
    case P_CENTER:
        obj->rect.y =
            y - (obj->rect.y + p_top - p_bottom) / 2;
        break;
    }

    return prev_y != obj->rect.y;
}

void GUI::element_set_y(Element *obj, Side side, int y)
{
    if (set_y_internal(obj, side, y))
        push_moved_event(obj);
}

void GUI::element_set_pos(Element *obj, Pos pos, int x, int y)
{
    Side x_side, y_side;
    pos_to_sides(pos, x_side, y_side);
    bool push_event_x = set_x_internal(obj, x_side, x);
    bool push_event_y = set_y_internal(obj, y_side, y);
    if (push_event_x || push_event_y)
        push_moved_event(obj);
}

static bool set_w_internal(Element *obj, int w, bool to_padding)
{
    int prev_w = obj->rect.w;
    i32 p_left, p_right, p_top, p_bottom;
    element_get_padding(obj, p_top, p_left, p_bottom, p_right);

    if (w < 0)
        w = 0;

    if (to_padding)
        obj->rect.w = w + p_left + p_right;
    else
        obj->rect.w = w;
    return prev_w != obj->rect.w;
}

void GUI::element_set_w(Element *obj, int w, bool to_padding)
{
    if (set_w_internal(obj, w, to_padding))
        push_resized_event(obj);
}

static bool set_h_internal(Element *obj, int h, bool to_padding)
{
    int prev_h = obj->rect.h;
    i32 p_left, p_right, p_top, p_bottom;
    element_get_padding(obj, p_top, p_left, p_bottom, p_right);

    if (h < 0)
        h = 0;

    if (to_padding)
        obj->rect.h = h + p_top + p_bottom;
    else
        obj->rect.h = h;
    return prev_h != obj->rect.h;
}

void GUI::element_set_h(Element *obj, int h, bool to_padding)
{
    if (set_h_internal(obj, h, to_padding))
        push_resized_event(obj);
}

void GUI::element_set_size(Element *obj, int w, int h, bool to_padding)
{
    bool push_event_w = set_w_internal(obj, w, to_padding);
    bool push_event_h = set_h_internal(obj, h, to_padding);
    if (push_event_w || push_event_h)
        push_resized_event(obj);
}

const i8 *GUI::element_get_type_name(Element *obj)
{
    switch (obj->el_type) {
    case GUI_ET_BASE:
        return "Base";
    case GUI_ET_LABEL:
        return "Label";
    case GUI_ET_BUTTON:
        return "Button";
    case GUI_ET_STACK_LAYOUT:
        return "StackLayout";
    default:
        return "<Element type unknwon>";
    }
}

bool GUI::element_set_ani(Element *obj, const i8 *name, struct _AniObj *ani)
{
    return Nst_map_set_str(obj->animations, name, ani);
}

struct _AniObj *GUI::element_get_ani(Element *obj, const i8 *name)
{
    return (struct _AniObj *)Nst_map_get_str(obj->animations, name);
}

SDL_Rect GUI::element_get_padding_rect(Element *obj)
{
    SDL_Rect r = obj->rect;
    i32 p_left, p_right, p_top, p_bottom;
    element_get_padding(obj, p_top, p_left, p_bottom, p_right);

    r.x = r.x + p_left;
    r.y = r.y + p_top;
    r.w -= p_left + p_right;
    r.h -= p_top + p_bottom;
    if (r.w < 0 || r.h < 0) {
        r.w = 0;
        r.h = 0;
    }
    return r;
}

SDL_Rect GUI::element_get_clip_rect(Element *obj)
{
    if (obj->parent == obj) {
        if (obj->clip_content)
            return element_get_padding_rect(obj);
        return obj->app->clip_window;
    }

    if (obj->clip_content) {
        SDL_Rect clip;
        SDL_Rect parent_clip = element_get_clip_rect(obj->parent);
        SDL_Rect padding_rect = element_get_padding_rect(obj);
        SDL_IntersectRect(&parent_clip, &padding_rect, &clip);
        return clip;
    } else
        return element_get_clip_rect(obj->parent);
}

bool GUI::element_add_child(Element *parent, Element *child)
{
    if (!Nst_vector_append(parent->children, child))
        return false;
    element_set_parent(child, parent);

    SDL_Event event;
    event.type = SDL_USEREVENT;
    event.user.code = GUI_UE_CHILD_ADDED;
    event.user.data1 = parent;
    event.user.data2 = (void *)(parent->children->len - 1);
    SDL_PushEvent(&event);

    return true;
}

bool GUI::element_remove_child(Element *parent, Element *child)
{
    return Nst_vector_remove(parent->children, child);
}

void GUI::element_set_clip(Element *element, bool clip)
{
    element->clip_content = clip;
}

FontObj *GUI::get_font(struct _App *app, FontWeight weight, FontSize size,
                       bool italic, bool monospace)
{
    usize idx = size * 16 + (monospace ? 8 : 0) + (italic ? 4 : 0) + weight;
    FontObj *font_obj = app->builtin_fonts[idx];
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
    buf[buf_len - 5] = '0' + (i8)weight;

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
    font_obj = (FontObj *)GUI::font_new(font);
    if (font_obj == nullptr) {
        TTF_CloseFont(font);
        return nullptr;
    }

    app->builtin_fonts[idx] = font_obj;
    return font_obj;
}

Nst_Obj *GUI::font_new(TTF_Font *font)
{
    FontObj *f = Nst_obj_alloc(FontObj, font_type);
    f->font = font;
    if (!Nst_vector_append(get_global_app()->opened_fonts, f)) {
        Nst_dec_ref(f);
        return nullptr;
    }
    return OBJ(f);
}

Nst_Obj *GUI::get_font_type()
{
    return OBJ(font_type);
}

void GUI::font_destroy(FontObj *f)
{
    if (TTF_WasInit()) {
        TTF_CloseFont(f->font);
        Nst_vector_remove(get_global_app()->opened_fonts, f);
    }
}
