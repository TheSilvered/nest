#include <SDL.h>
#include <SDL_ttf.h>
#include <cstring>
#include "nest_gui.h"
#include "gui_event.h"
#include "gui_update.h"
#include "gui_label.h"
#include "gui_button.h"
#include "gui_stack_layout.h"

#define FUNC_COUNT 32

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *sdl_error_str;
static GUI_App app;
Nst_TypeObj *gui_element_type;
Nst_TypeObj *gui_font_type;
GUI_FontObj *default_font;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(init_sdl_and_ttf_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(loop_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(label_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(button_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(stack_layout_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_window_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_pos_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_rel_pos_, 8);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_rel_size_, 10);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_size_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_overflow_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_overflow_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_margins_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_margins_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_padding_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_padding_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(show_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hide_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_hidden_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_auto_height_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_auto_height_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_font_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_font_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(add_child_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(remove_child_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_root_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_func_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_builtin_font_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(open_font_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_debug_view_, 1);

#if __LINE__ - FUNC_COUNT != 27
#error
#endif

    sdl_error_str = STR(Nst_string_new_c_raw("SDL Error", false));
    gui_element_type = Nst_type_new("GUI_Element");
    gui_font_type = Nst_type_new("GUI Font");
    default_font = nullptr;

    app.root = nullptr;
    app.focused_element = nullptr;
    app.window = nullptr;
    app.renderer = nullptr;
    app.clip_window = { 0, 0, 0, 0 };

    app.keep_open = false;
    app.show_bounds = false;
    app.element_reached = false;

    app.fg_color = { 245, 245, 245, 255 };
    app.fg_color2 = { 173, 173, 173, 255 };
    app.fg_color3 = { 117, 117, 117, 255 };
    app.bg_color3 = { 64, 64, 64, 255 };
    app.bg_color2 = { 46, 46, 46, 255 };
    app.bg_color = { 26, 26, 26, 255 };
    app.bg_dark = { 18, 18, 18, 255 };

    memset(app.builtin_fonts, 0, sizeof(TTF_Font *) * 48);

    app.opened_fonts = Nst_vector_new(0);

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    Nst_dec_ref(sdl_error_str);
    Nst_dec_ref(gui_element_type);
    Nst_dec_ref(gui_font_type);

    if (app.window != nullptr) {
        SDL_DestroyRenderer(app.renderer);
        SDL_DestroyWindow(app.window);
        Nst_dec_ref(app.root);
    }

    for (usize i = 0, n = SEQ(app.opened_fonts)->len; i < n; i++)
        TTF_CloseFont(((GUI_FontObj *)(SEQ(app.opened_fonts)->objs[i]))->font);

    TTF_Quit();
    SDL_Quit();

    for (int i = 0; i < 48; i++)
        Nst_ndec_ref(app.builtin_fonts[i]);
}

static void gui_font_destructor(GUI_FontObj *f)
{
    if (!TTF_WasInit()) {
        TTF_CloseFont(f->font);
        Nst_vector_remove(app.opened_fonts, f);
    }
}

Nst_Obj *gui_font_new(TTF_Font *font)
{
    GUI_FontObj *f = Nst_obj_alloc(
        GUI_FontObj,
        gui_font_type,
        (Nst_ObjDestructor)(gui_font_destructor));
    f->font = font;
    if (!Nst_vector_append(app.opened_fonts, f)) {
        Nst_dec_ref(f);
        return nullptr;
    }
    return OBJ(f);
}

void set_sdl_error()
{
    const i8 *sdl_error = SDL_GetError();
    usize len = strlen(sdl_error);
    i8 *new_error = (i8 *)Nst_calloc(
        1,
        sizeof(i8) * (len + 1),
        (void *)sdl_error); // copies the string if the allocation succeded
    if (new_error == nullptr)
        return;
    Nst_StrObj *msg = STR(Nst_string_new(new_error, len, true));
    if (msg == nullptr) {
        Nst_free(new_error);
        return;
    }
    Nst_set_error(Nst_inc_ref(sdl_error_str), msg);
}

int imin(int n1, int n2)
{
    return n2 < n1 ? n2 : n1;
}

int imax(int n1, int n2)
{
    return n2 > n1 ? n2 : n1;
}

void set_focused_element(GUI_Element *el)
{
    remove_focused_element();
    app.focused_element = el;
    Nst_inc_ref(el);
}

GUI_Element *get_focused_element()
{
    return app.focused_element;
}

void remove_focused_element()
{
    if (app.focused_element != nullptr) {
        Nst_dec_ref(app.focused_element);
        app.focused_element = nullptr;
    }
}

Nst_FUNC_SIGN(init_sdl_and_ttf_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING)
        Nst_RETURN_NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        set_sdl_error();
        return nullptr;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(loop_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    while (app.keep_open) {
        if (!handle_events(&app))
            return nullptr;
        if (!update_elements(&app))
            return nullptr;
        if (!tick_elements(&app))
            return nullptr;

        SDL_RenderPresent(app.renderer);
    }

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(label_)
{
    Nst_StrObj *text;
    GUI_FontObj *font;
    Nst_SeqObj *color;
    u8 r, g, b, a;

    Nst_DEF_EXTRACT("s?#?A.i", &text, gui_font_type, &font, &color);

    if (OBJ(color) == Nst_null()) {
        r = app.fg_color.r;
        g = app.fg_color.g;
        b = app.fg_color.b;
        a = app.fg_color.a;
    } else if (color->len == 3) {
        r = u8(AS_INT(color->objs[0]));
        g = u8(AS_INT(color->objs[1]));
        b = u8(AS_INT(color->objs[2]));
        a = 255;
    } else if (color->len == 4) {
        r = u8(AS_INT(color->objs[0]));
        g = u8(AS_INT(color->objs[1]));
        b = u8(AS_INT(color->objs[2]));
        a = u8(AS_INT(color->objs[4]));
    } else {
        Nst_set_value_error_c("the color must be of length 3 or 4");
        return nullptr;
    }

    if (OBJ(font) == Nst_null()) {
        font = get_font(&app, GUI_FW_REGULAR, GUI_FS_MEDIUM, false, false);
        if (font == nullptr)
            return nullptr;
    }
    Nst_inc_ref(font);

    int w, h;
    TTF_SizeUTF8(font->font, text->value, &w, &h);

    GUI_Element *label = gui_label_new(
        text, font,
        { r, g, b, a },
        0, 0, w + 1, h,
        &app);

    if (label == nullptr)
        return nullptr;
    gui_element_set_margin(label, 10, 0, 0, 0);
    return OBJ(label);
}

Nst_FUNC_SIGN(button_)
{
    Nst_StrObj *text;
    GUI_FontObj *font;
    Nst_DEF_EXTRACT("s?#", &text, gui_font_type, &font);

    if (OBJ(font) == Nst_null()) {
        font = get_font(&app, GUI_FW_REGULAR, GUI_FS_MEDIUM, false, false);
        if (font == nullptr)
            return nullptr;
    }
    Nst_inc_ref(font);

    int w, h;
    TTF_SizeUTF8(font->font, text->value, &w, &h);

    GUI_Element *label = gui_label_new(
        text, font,
        app.fg_color,
        0, 0, w + 1, h,
        &app);

    if (label == nullptr)
        return nullptr;

    GUI_Element *button = gui_button_new((GUI_Label *)label, &app);
    return OBJ(button);
}

Nst_FUNC_SIGN(stack_layout_)
{
    Nst_Obj *direction_obj;
    Nst_Obj *alignment_obj;

    Nst_DEF_EXTRACT("?i?i", &direction_obj, &alignment_obj);
    i64 direction = Nst_DEF_VAL(direction_obj, AS_INT(direction_obj), 0);
    i64 alignment = Nst_DEF_VAL(alignment_obj, AS_INT(alignment_obj), 0);

    if (direction < 0 || direction > 3) {
        Nst_set_value_error_c("invalid direction");
        return nullptr;
    }
    if (alignment < 0 || alignment > 2) {
        Nst_set_value_error_c("invalid alignment");
        return nullptr;
    }

    return OBJ(gui_stack_layout_new(
        (GUI_StackDir)direction,
        (GUI_StackAlign)alignment,
        0, 0, 0, 0,
        &app));
}

Nst_FUNC_SIGN(set_window_)
{
    if (app.window != nullptr) {
        Nst_set_call_error_c("'set_window' was called more than once");
        return nullptr;
    }

    Nst_StrObj *title;
    i64 w, h;
    Nst_Obj *flags_obj;
    Nst_Obj *pos_x_obj, *pos_y_obj;
    int flags, pos_x, pos_y;

    Nst_DEF_EXTRACT("sii?i?i?i", &title, &w, &h, &flags_obj, &pos_x_obj, &pos_y_obj);
    flags = Nst_DEF_VAL(flags_obj, int(AS_INT(flags_obj)), 0);
    pos_x = Nst_DEF_VAL(pos_x_obj, int(AS_INT(pos_x_obj)), SDL_WINDOWPOS_CENTERED);
    pos_y = Nst_DEF_VAL(pos_y_obj, int(AS_INT(pos_y_obj)), SDL_WINDOWPOS_CENTERED);

    app.window = SDL_CreateWindow(title->value, pos_x, pos_y, int(w), int(h), flags);
    if (app.window == nullptr) {
        set_sdl_error();
        return nullptr;
    }
    app.renderer = SDL_CreateRenderer(app.window, -1, 0);
    if (app.renderer == nullptr) {
        set_sdl_error();
        SDL_DestroyWindow(app.window);
        return nullptr;
    }

    GUI_Element *root = gui_element_new(
        GUI_ET_BASE,
        sizeof(GUI_Element),
        0, 0,
        int(w), int(h),
        &app);
    if (root == nullptr) {
        SDL_DestroyRenderer(app.renderer);
        SDL_DestroyWindow(app.window);
        return nullptr;
    }
    root->handle_event_func = root_handle_event;
    root->frame_update_func = root_update;
    app.root = root;

    gui_element_set_parent(root, root);
    gui_element_set_rel_size(
        root, nullptr,
        GUI_RECT_ELEMENT, GUI_RECT_ELEMENT,
        -1, -1,
        -1, -1,
        1.0, 1.0,
        0, 0);
    gui_element_set_padding(root, 5, 5, 5, 5);

    app.keep_open = true;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(set_pos_)
{
    GUI_Element *element;
    i64 x, y, rect, pos_x, pos_y;
    Nst_Obj *rect_obj, *pos_x_obj, *pos_y_obj;
    Nst_DEF_EXTRACT(
        "#ii?i?i?i",
        gui_element_type, &element,
        &x, &y,
        &pos_x_obj, &pos_y_obj,
        &rect_obj);
    rect = Nst_DEF_VAL(rect_obj, AS_INT(rect_obj), 1);
    pos_x = Nst_DEF_VAL(pos_x_obj, AS_INT(pos_x_obj), 0);
    pos_y = Nst_DEF_VAL(pos_y_obj, AS_INT(pos_y_obj), 0);

    gui_element_set_pos(
        element,
        (GUI_RelPosX)pos_x, (int)x,
        (GUI_RelPosY)pos_y, (int)y,
        (GUI_RelRect)rect);
    if (Nst_FLAG_HAS(element, GUI_FLAG_REL_POS)) {
        Nst_FLAG_DEL(element, GUI_FLAG_REL_POS);
        Nst_dec_ref(element->rel_pos.element);
        element->rel_pos.element = nullptr;
    }
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(set_rel_pos_)
{
    GUI_Element *from_element, *to_element;
    i64 from_x, from_y, to_x, to_y;
    Nst_Obj *to_x_obj, *to_y_obj;
    Nst_Obj *from_rect_obj, *to_rect_obj;

    Nst_DEF_EXTRACT(
        "?#" // from_element
        "#" // to_element
        "i" // from_x
        "i" // from_y
        "?i" // to_x_obj
        "?i" // to_y_obj
        "?i" // from_rect_obj
        "?i", // to_rect_obj
        gui_element_type, &from_element,
        gui_element_type, &to_element,
        &from_x, &from_y, &to_x_obj, &to_y_obj,
        &from_rect_obj, &to_rect_obj);

    to_x = Nst_DEF_VAL(to_x_obj, AS_INT(to_x_obj), from_x);
    to_y = Nst_DEF_VAL(to_y_obj, AS_INT(to_y_obj), from_y);
    i64 from_rect = Nst_DEF_VAL(from_rect_obj, AS_INT(from_rect_obj), 0);
    i64 to_rect = Nst_DEF_VAL(to_rect_obj, AS_INT(to_rect_obj), 2);

    if (OBJ(from_element) == Nst_null()) {
        Nst_dec_ref(from_element);
        from_element = nullptr;
    }

    gui_element_set_rel_pos(
        to_element, from_element,
        (GUI_RelRect)from_rect, (GUI_RelRect)to_rect,
        (GUI_RelPosX)from_x, (GUI_RelPosY)from_y,
        (GUI_RelPosX)to_x,   (GUI_RelPosY)to_y);
    return Nst_inc_ref(from_element);
}

Nst_FUNC_SIGN(get_pos_)
{
    GUI_Element *element;
    Nst_Obj *pos_x_obj, *pos_y_obj, *rect_obj;
    Nst_DEF_EXTRACT(
        "#?i?i?i",
        gui_element_type, &element,
        &pos_x_obj, &pos_y_obj,
        &rect_obj);
    i64 pos_x = Nst_DEF_VAL(pos_x_obj, AS_INT(pos_x_obj), 0);
    i64 pos_y = Nst_DEF_VAL(pos_y_obj, AS_INT(pos_y_obj), 0);
    i64 rel_rect = Nst_DEF_VAL(rect_obj, AS_INT(rect_obj), 1);

    SDL_Rect rect;
    if ((GUI_RelRect)rel_rect == GUI_RECT_MARGIN)
        rect = gui_element_get_margin_rect(element);
    else if ((GUI_RelRect)rel_rect == GUI_RECT_PADDING)
        rect = gui_element_get_padding_rect(element);
    else
        rect = element->rect;

    int x, y;
    switch ((GUI_RelPosX)pos_x) {
    case GUI_MIDDLE: x = rect.x + rect.w / 2; break;
    case GUI_RIGHT: x = rect.x + rect.w; break;
    default: x = rect.x;
    }

    switch ((GUI_RelPosX)pos_y) {
    case GUI_MIDDLE: y = rect.y + rect.h / 2; break;
    case GUI_RIGHT: y = rect.y + rect.h; break;
    default: y = rect.y;
    }

    return Nst_array_create_c("ii", x, y);
}

Nst_FUNC_SIGN(set_size_)
{
    GUI_Element *element;
    i64 w, h;
    Nst_Obj *rect_obj;
    Nst_DEF_EXTRACT("#ii?i", gui_element_type, &element, &w, &h, &rect_obj);
    i64 rect = Nst_DEF_VAL(rect_obj, AS_INT(rect_obj), 1);
    gui_element_set_size(element, (int)w, (int)h, (GUI_RelRect)rect);
    if (Nst_FLAG_HAS(element, GUI_FLAG_REL_SIZE)) {
        Nst_FLAG_DEL(element, GUI_FLAG_REL_SIZE);
        if (element->rel_size.element != nullptr) {
            Nst_dec_ref(element->rel_size.element);
            element->rel_size.element = nullptr;
        }
    }
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(set_rel_size_)
{
    GUI_Element *from_element, *to_element;
    Nst_Obj *size_x, *size_y;
    Nst_Obj *min_w_obj, *min_h_obj, *max_w_obj, *max_h_obj;
    i64 min_w, min_h, max_w, max_h;
    Nst_Obj *from_rect_obj, *to_rect_obj;

    Nst_DEF_EXTRACT(
        "?#" // from_element
        "#" // to_element
        "i|r" // size_x
        "i|r" // size_y
        "?i" // min_w_obj
        "?i" // min_h_obj
        "?i" // max_w_obj
        "?i" // max_h_obj
        "?i" // from_rect_obj
        "?i", // to_rect_obj
        gui_element_type, &from_element,
        gui_element_type, &to_element,
        &size_x, &size_y,
        &min_w_obj, &min_h_obj, &max_w_obj, &max_h_obj,
        &from_rect_obj, &to_rect_obj);

    if (OBJ(from_element) == Nst_null()) {
        Nst_dec_ref(from_element);
        from_element = nullptr;
    }

    min_w = Nst_DEF_VAL(min_w_obj, AS_INT(min_w_obj), -1);
    min_h = Nst_DEF_VAL(min_h_obj, AS_INT(min_h_obj), -1);
    max_w = Nst_DEF_VAL(max_w_obj, AS_INT(max_w_obj), -1);
    max_h = Nst_DEF_VAL(max_h_obj, AS_INT(max_h_obj), -1);
    i64 from_rect = Nst_DEF_VAL(from_rect_obj, AS_INT(from_rect_obj), 0);
    i64 to_rect = Nst_DEF_VAL(from_rect_obj, AS_INT(from_rect_obj), 2);

    i32 diff_x = 0, diff_y = 0;
    f64 scale_x = 0.0, scale_y = 0.0;

    if (size_x->type == Nst_type()->Int)
        diff_x = (i32)AS_INT(size_x);
    else
        scale_x = AS_REAL(size_x);

    if (size_y->type == Nst_type()->Int)
        diff_y = (i32)AS_INT(size_y);
    else
        scale_y = AS_REAL(size_y);

    gui_element_set_rel_size(
        to_element, from_element,
        (GUI_RelRect)from_rect, (GUI_RelRect)to_rect,
        (int)min_w, (int)min_h, (int)max_w, (int)max_h,
        scale_x, scale_y,
        diff_x, diff_y);

    return Nst_inc_ref(from_element);
}

Nst_FUNC_SIGN(get_size_)
{
    GUI_Element *element;
    Nst_Obj *rect_obj;
    Nst_DEF_EXTRACT("#?i", gui_element_type, &element, &rect_obj);
    i64 rel_rect = Nst_DEF_VAL(rect_obj, AS_INT(rect_obj), 1);

    SDL_Rect rect;
    if ((GUI_RelRect)rel_rect == GUI_RECT_MARGIN)
        rect = gui_element_get_margin_rect(element);
    else if ((GUI_RelRect)rel_rect == GUI_RECT_PADDING)
        rect = gui_element_get_padding_rect(element);
    else
        rect = element->rect;
    return Nst_array_create_c("ii", rect.w, rect.h);
}

Nst_FUNC_SIGN(set_overflow_)
{
    GUI_Element *element;
    bool overflow;

    Nst_DEF_EXTRACT("#o_b", gui_element_type, &element, &overflow);

    gui_element_clip_content(element, !overflow);

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_overflow_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    Nst_RETURN_COND(element->clip_content);
}

Nst_FUNC_SIGN(set_margins_)
{
    GUI_Element *element;
    Nst_Obj *l_obj, *b_obj, *r_obj;
    i64 t, l, b, r;
    Nst_DEF_EXTRACT(
        "#i?i?i?i",
        gui_element_type, &element,
        &t, &l_obj, &b_obj, &r_obj);

    Nst_Obj *null = Nst_null();

    if (l_obj == null && b_obj == null && r_obj == null) {
        l = t;
        b = t;
        r = t;
    } else if (l_obj != null && b_obj == null && r_obj == null) {
        b = t;
        l = AS_INT(l_obj);
        r = l;
    } else if (l_obj != null && b_obj != null && r_obj == null) {
        l = AS_INT(l_obj);
        b = AS_INT(b_obj);
        r = l;
    } else if (l_obj != null && b_obj != null && r_obj != null) {
        l = AS_INT(l_obj);
        b = AS_INT(b_obj);
        r = AS_INT(r_obj);
    } else {
        l = Nst_DEF_VAL(l_obj, AS_INT(l_obj), -1);
        b = Nst_DEF_VAL(b_obj, AS_INT(b_obj), -1);
        r = Nst_DEF_VAL(r_obj, AS_INT(r_obj), -1);
    }

    gui_element_set_margin(element, (i32)t, (i32)l, (i32)b, (i32)r);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_margins_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    return Nst_array_create_c(
        "iiii",
        element->margin_left, element->margin_right,
        element->margin_top, element->margin_bottom);
}

Nst_FUNC_SIGN(set_padding_)
{
    GUI_Element *element;
    Nst_Obj *l_obj, *b_obj, *r_obj;
    i64 t, l, b, r;
    Nst_DEF_EXTRACT(
        "#i?i?i?i",
        gui_element_type, &element,
        &t, &l_obj, &b_obj, &r_obj);

    Nst_Obj *null = Nst_null();

    if (l_obj == null && b_obj == null && r_obj == null) {
        l = t;
        b = t;
        r = t;
    } else if (l_obj != null && b_obj == null && r_obj == null) {
        b = t;
        l = AS_INT(l_obj);
        r = l;
    } else if (l_obj != null && b_obj != null && r_obj == null) {
        l = AS_INT(l_obj);
        b = AS_INT(b_obj);
        r = l;
    } else if (l_obj != null && b_obj != null && r_obj != null) {
        l = AS_INT(l_obj);
        b = AS_INT(b_obj);
        r = AS_INT(r_obj);
    } else {
        l = Nst_DEF_VAL(l_obj, AS_INT(l_obj), -1);
        b = Nst_DEF_VAL(b_obj, AS_INT(b_obj), -1);
        r = Nst_DEF_VAL(r_obj, AS_INT(r_obj), -1);
    }

    gui_element_set_padding(element, (i32)t, (i32)l, (i32)b, (i32)r);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_padding_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    return Nst_array_create_c(
        "iiii",
        element->padding_left, element->padding_right,
        element->padding_top, element->padding_bottom);
}

Nst_FUNC_SIGN(show_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    Nst_FLAG_DEL(element, GUI_FLAG_IS_HIDDEN);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(hide_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    Nst_FLAG_SET(element, GUI_FLAG_IS_HIDDEN);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(is_hidden_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);
    Nst_RETURN_COND(Nst_FLAG_HAS(element, GUI_FLAG_IS_HIDDEN));
}

Nst_FUNC_SIGN(set_auto_height_)
{
    GUI_Element *element;
    bool auto_height;

    Nst_DEF_EXTRACT("#o_b", gui_element_type, &element, &auto_height);

    if (element->el_type == GUI_ET_LABEL)
        ((GUI_Label *)element)->auto_height = auto_height;

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_auto_height_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("#", gui_element_type, &element);

    if (element->el_type == GUI_ET_LABEL)
        Nst_RETURN_COND(((GUI_Label *)element)->auto_height);

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(set_font_)
{
    GUI_Element *element;
    GUI_FontObj *font;
    Nst_Obj *change_size_obj;
    Nst_DEF_EXTRACT(
        "##o",
        gui_element_type, &element,
        gui_font_type, &font,
        &change_size_obj);

    bool change_size = Nst_DEF_VAL(
        change_size_obj,
        Nst_obj_to_bool(change_size_obj),
        true);

    if (element->el_type == GUI_ET_LABEL)
        gui_label_set_font((GUI_Label *)element, font, change_size);
    else if (element->el_type == GUI_ET_BUTTON)
        gui_label_set_font(((GUI_Button *)element)->text, font, change_size);

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_font_)
{
    GUI_Element *element;
    Nst_DEF_EXTRACT("##", gui_element_type, &element);

    if (element->el_type == GUI_ET_LABEL)
        return Nst_inc_ref(((GUI_Label *)element)->font);
    else if (element->el_type == GUI_ET_BUTTON)
        return Nst_inc_ref(((GUI_Button *)element)->text->font);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(add_child_)
{
    GUI_Element *parent, *child;
    Nst_DEF_EXTRACT("##", gui_element_type, &parent, gui_element_type, &child);
    if (!gui_element_add_child(parent, child))
        return nullptr;
    return Nst_inc_ref(parent);
}

Nst_FUNC_SIGN(remove_child_)
{
    GUI_Element *parent, *child;
    Nst_DEF_EXTRACT("##", gui_element_type, &parent, gui_element_type, &child);
    Nst_RETURN_COND(gui_element_remove_child(parent, child));
}

Nst_FUNC_SIGN(get_root_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if ( app.window == nullptr )
    {
        Nst_set_call_error_c("'set_window' must be called before getting the root");
        return nullptr;
    }
    return Nst_inc_ref(app.root);
}

Nst_FUNC_SIGN(set_func_)
{
    GUI_Element *el;
    Nst_FuncObj *func;
    Nst_DEF_EXTRACT("#f", gui_element_type, &el, &func);
    if ( el->el_type != GUI_ET_BUTTON )
    {
        Nst_set_value_error_c("the element must be a button");
        return nullptr;
    }
    if ( func->arg_num != 1 )
    {
        Nst_set_value_error_c("the function of a button must take exactly 1 argument");
        return nullptr;
    }
    GUI_Button *b = (GUI_Button *)el;
    b->func = gui_button_call_nest_func;
    b->nest_func = FUNC(Nst_inc_ref(func));
    return Nst_inc_ref(el);
}

Nst_FUNC_SIGN(get_builtin_font_)
{
    Nst_Obj *weight_obj, *size_obj;
    bool monospace, italic;
    Nst_DEF_EXTRACT("?i?io_bo_b", &weight_obj, &size_obj, &italic, &monospace);
    i64 weight = Nst_DEF_VAL(
        weight_obj,
        AS_INT(weight_obj),
        (i64)GUI_FW_REGULAR);
    i64 size = Nst_DEF_VAL(
        size_obj,
        AS_INT(size_obj),
        (i64)GUI_FS_MEDIUM);

    return OBJ(get_font(
        &app,
        (GUI_FontWeight)weight,
        (GUI_FontSize)size,
        (int)italic,
        (int)monospace));
}

Nst_FUNC_SIGN(open_font_)
{
    Nst_StrObj *path;
    i64 size;
    bool bold, italic, underline, strikethrough;
    Nst_DEF_EXTRACT(
        "sio_bo_bo_bo_b",
        &path, &size,
        &bold, &italic, &underline, &strikethrough);
    TTF_Font *font = TTF_OpenFont(path->value, (int)size);
    if (font == nullptr) {
        Nst_set_value_error(Nst_sprintf(
            "font at %.4096s could not be opened", path->value));
        return nullptr;
    }
    if (bold)
        TTF_SetFontStyle(font, TTF_STYLE_BOLD);
    if (italic)
        TTF_SetFontStyle(font, TTF_STYLE_ITALIC);
    if (underline)
        TTF_SetFontStyle(font, TTF_STYLE_UNDERLINE);
    if (strikethrough)
        TTF_SetFontStyle(font, TTF_STYLE_STRIKETHROUGH);

    GUI_FontObj *font_obj = (GUI_FontObj *)gui_font_new(font);
    if (font_obj == nullptr)
        TTF_CloseFont(font);
    return OBJ(font_obj);
}

Nst_FUNC_SIGN(_debug_view_)
{
    bool show_view;
    Nst_DEF_EXTRACT("o_b", &show_view);
    app.show_bounds = show_view;
    Nst_RETURN_NULL;
}
