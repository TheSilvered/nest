#include <SDL.h>
#include <SDL_ttf.h>
#include <cstring>
#include "nest_gui.h"
#include "gui_event.h"
#include "gui_update.h"
#include "gui_label.h"
#include "gui_button.h"
#include "gui_stack_layout.h"
#include "gui_animation.h"

#define FUNC_COUNT 52

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *sdl_error_str;
static GUI::App app;
static i32 label_count = 1, button_count = 1;
GUI::FontObj *default_font;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(init_sdl_and_ttf_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(loop_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(label_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(button_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(stack_layout_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_window_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_pos_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_size_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_overflow_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_overflow_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_padding_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_padding_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(show_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hide_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(is_hidden_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_auto_height_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_auto_height_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_font_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_font_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_root_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_func_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_builtin_font_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(open_font_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_x_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_y_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_w_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_h_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(match_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(perc_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(diff_pos_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(perc_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(diff_size_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_x_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_y_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_x_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_y_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_pos_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_w_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_h_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(min_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_w_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_h_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(max_size_, 4);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(fill_w_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(fill_h_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_text_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_text_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_debug_view_, 1);

#if __LINE__ - FUNC_COUNT != 27
#error
#endif

    if (!GUI::ani_init())
        return false;
    if (!GUI::element_init()) {
        GUI::ani_quit();
        return false;
    }

    sdl_error_str = STR(Nst_string_new_c_raw("SDL Error", false));
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
    GUI::ani_quit();
    GUI::element_quit();

    if (app.window != nullptr) {
        SDL_DestroyRenderer(app.renderer);
        SDL_DestroyWindow(app.window);
        Nst_dec_ref(app.root);
    }

    for (usize i = 0, n = SEQ(app.opened_fonts)->len; i < n; i++)
        TTF_CloseFont(((GUI::FontObj *)(SEQ(app.opened_fonts)->objs[i]))->font);

    TTF_Quit();
    SDL_Quit();

    for (int i = 0; i < 48; i++)
        Nst_ndec_ref(app.builtin_fonts[i]);
}

void GUI::set_sdl_error()
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

int GUI::imin(int n1, int n2)
{
    return n2 < n1 ? n2 : n1;
}

int GUI::imax(int n1, int n2)
{
    return n2 > n1 ? n2 : n1;
}

void GUI::set_focused_element(GUI::Element *el)
{
    GUI::remove_focused_element();
    app.focused_element = el;
    Nst_inc_ref(el);
}

GUI::Element *GUI::get_focused_element()
{
    return app.focused_element;
}

void GUI::remove_focused_element()
{
    if (app.focused_element != nullptr) {
        Nst_dec_ref(app.focused_element);
        app.focused_element = nullptr;
    }
}

GUI::App *GUI::get_global_app()
{
    return &app;
}

static void fmt_label_text(const i8 *base, i8 *buf, i32 &count)
{
    sprintf(buf, base, count);
    count++;
}

Nst_FUNC_SIGN(init_sdl_and_ttf_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING)
        Nst_RETURN_NULL;

    if (SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init()) {
        GUI::set_sdl_error();
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
    GUI::Element *parent;

    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &parent);
    u8 r, g, b, a;
    r = app.fg_color.r;
    g = app.fg_color.g;
    b = app.fg_color.b;
    a = app.fg_color.a;

    GUI::FontObj *font = get_font(
        &app,
        GUI::GUI_FW_REGULAR, GUI::GUI_FS_MEDIUM,
        false, false);
    if (font == nullptr)
        return nullptr;
    Nst_inc_ref(font);

    i8 buf[16];
    fmt_label_text("Label %li", buf, button_count);
    Nst_StrObj text = Nst_string_temp(buf, strlen(buf));
    int w, h;
    TTF_SizeUTF8(font->font, text.value, &w, &h);
    GUI::Element *label = GUI::label_new(
        &text, font,
        { r, g, b, a },
        0, 0, w, h,
        &app);

    if (label == nullptr)
        return nullptr;

    if (!GUI::element_add_child(parent, label)) {
        Nst_dec_ref(label);
        return nullptr;
    }
    return OBJ(label);
}

Nst_FUNC_SIGN(button_)
{
    GUI::Element *parent;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &parent);

    GUI::FontObj *font = get_font(
        &app,
        GUI::GUI_FW_REGULAR, GUI::GUI_FS_MEDIUM,
        false, false);
    if (font == nullptr)
        return nullptr;
    Nst_inc_ref(font);

    i8 buf[17];
    fmt_label_text("Button %li", buf, label_count);
    Nst_StrObj text = Nst_string_temp(buf, strlen(buf));
    int w, h;
    TTF_SizeUTF8(font->font, text.value, &w, &h);

    GUI::Element *label = GUI::label_new(
        &text, font,
        app.fg_color,
        0, 0, w, h,
        &app);

    if (label == nullptr)
        return nullptr;

    GUI::Element *button = GUI::button_new((GUI::Label *)label, &app);
    if (!GUI::element_add_child(parent, button)) {
        Nst_dec_ref(button);
        return nullptr;
    }
    return OBJ(button);
}

Nst_FUNC_SIGN(stack_layout_)
{
    GUI::Element *parent;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &parent);

    GUI::Element *sl = GUI::stack_layout_new(
        GUI::SD_TOP_BOTTOM,
        GUI::SA_LEFT_TOP,
        0, 0, 0, 0,
        &app);
    if (!GUI::element_add_child(parent, sl)) {
        Nst_dec_ref(sl);
        return nullptr;
    }
    return OBJ(sl);
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

    Nst_DEF_EXTRACT(
        "s i i ?i ?i ?i",
        &title, &w, &h,
        &flags_obj,
        &pos_x_obj, &pos_y_obj);
    flags = Nst_DEF_VAL(flags_obj, int(AS_INT(flags_obj)), 0);
    pos_x = Nst_DEF_VAL(pos_x_obj, int(AS_INT(pos_x_obj)), SDL_WINDOWPOS_CENTERED);
    pos_y = Nst_DEF_VAL(pos_y_obj, int(AS_INT(pos_y_obj)), SDL_WINDOWPOS_CENTERED);

    app.window = SDL_CreateWindow(title->value, pos_x, pos_y, int(w), int(h), flags);
    if (app.window == nullptr) {
        GUI::set_sdl_error();
        return nullptr;
    }
    app.renderer = SDL_CreateRenderer(app.window, -1, 0);
    if (app.renderer == nullptr) {
        GUI::set_sdl_error();
        SDL_DestroyWindow(app.window);
        return nullptr;
    }

    GUI::Element *root = GUI::element_new(
        GUI::GUI_ET_BASE,
        sizeof(GUI::Element),
        0, 0,
        int(w), int(h),
        &app, NULL);
    if (root == nullptr) {
        SDL_DestroyRenderer(app.renderer);
        SDL_DestroyWindow(app.window);
        return nullptr;
    }
    root->handle_event_func = GUI::root_handle_event;
    root->frame_update_func = GUI::root_update;
    app.root = root;

    GUI::element_set_parent(root, root);
    GUI::match_size(nullptr, app.root, false, false);

    app.keep_open = true;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(set_pos_)
{
    GUI::Element *element;
    i64 x, y, pos;
    Nst_Obj *pos_obj;
    Nst_DEF_EXTRACT(
        "# i i ?i",
        GUI::get_element_type(), &element,
        &x, &y,
        &pos_obj);
    pos = Nst_DEF_VAL(pos_obj, AS_INT(pos_obj), 0);

    GUI::element_set_pos(element, (GUI::Pos)pos, (int)x, (int)y);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_pos_)
{
    GUI::Element *element;
    Nst_Obj *pos_obj;
    Nst_DEF_EXTRACT("# ?i", GUI::get_element_type(), &element, &pos_obj);
    GUI::Pos pos = Nst_DEF_VAL(pos_obj, GUI::Pos(AS_INT(pos_obj)), GUI::TL);

    int x, y;
    GUI::element_get_pos(element, pos, x, y);

    return Nst_array_create_c("ii", x, y);
}

Nst_FUNC_SIGN(set_size_)
{
    GUI::Element *element;
    i64 w, h;
    bool to_padding;
    Nst_DEF_EXTRACT(
        "# i i y",
        GUI::get_element_type(), &element,
        &w, &h,
        &to_padding);

    GUI::element_set_size(element, (int)w, (int)h, to_padding);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_size_)
{
    GUI::Element *element;
    bool from_padding;
    Nst_DEF_EXTRACT("# y", GUI::get_element_type(), &element, &from_padding);

    int w, h;
    GUI::element_get_size(element, w, h, from_padding);

    return Nst_array_create_c("ii", w, h);
}

Nst_FUNC_SIGN(set_overflow_)
{
    GUI::Element *element;
    bool overflow;

    Nst_DEF_EXTRACT("# y", GUI::get_element_type(), &element, &overflow);

    GUI::element_set_clip(element, !overflow);

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_overflow_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);
    Nst_RETURN_COND(element->clip_content);
}

Nst_FUNC_SIGN(set_padding_)
{
    GUI::Element *element;
    Nst_Obj *l_obj, *b_obj, *r_obj;
    i64 t, l, b, r;
    Nst_DEF_EXTRACT(
        "# i ?i ?i ?i",
        GUI::get_element_type(), &element,
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

    GUI::element_set_padding(element, (i32)t, (i32)l, (i32)b, (i32)r);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_padding_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);
    i32 p_top, p_left, p_bottom, p_right;
    GUI::element_get_padding(element, p_top, p_left, p_bottom, p_right);
    return Nst_array_create_c("iiii", p_top, p_left, p_bottom, p_right);
}

Nst_FUNC_SIGN(show_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);
    Nst_DEL_FLAG(element, GUI::GUI_FLAG_IS_HIDDEN);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(hide_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);
    Nst_SET_FLAG(element, GUI::GUI_FLAG_IS_HIDDEN);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(is_hidden_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);
    Nst_RETURN_COND(Nst_HAS_FLAG(element, GUI::GUI_FLAG_IS_HIDDEN));
}

Nst_FUNC_SIGN(set_auto_height_)
{
    GUI::Element *element;
    bool auto_height;

    Nst_DEF_EXTRACT("# y", GUI::get_element_type(), &element, &auto_height);

    if (element->el_type == GUI::GUI_ET_LABEL)
        ((GUI::Label *)element)->auto_height = auto_height;

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_auto_height_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);

    if (element->el_type == GUI::GUI_ET_LABEL)
        Nst_RETURN_COND(((GUI::Label *)element)->auto_height);

    Nst_RETURN_FALSE;
}

Nst_FUNC_SIGN(set_font_)
{
    GUI::Element *element;
    GUI::FontObj *font;
    Nst_Obj *change_size_obj;
    Nst_DEF_EXTRACT(
        "# # o",
        GUI::get_element_type(), &element,
        GUI::get_font_type(), &font,
        &change_size_obj);

    bool change_size = Nst_DEF_VAL(
        change_size_obj,
        Nst_obj_to_bool(change_size_obj),
        true);

    if (element->el_type == GUI::GUI_ET_LABEL)
        GUI::label_set_font((GUI::Label *)element, font, change_size);
    else if (element->el_type == GUI::GUI_ET_BUTTON)
        GUI::label_set_font(((GUI::Button *)element)->text, font, change_size);

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(get_font_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("# #", GUI::get_element_type(), &element);

    if (element->el_type == GUI::GUI_ET_LABEL)
        return Nst_inc_ref(((GUI::Label *)element)->font);
    else if (element->el_type == GUI::GUI_ET_BUTTON)
        return Nst_inc_ref(((GUI::Button *)element)->text->font);

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(get_root_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if (app.window == nullptr) {
        Nst_set_call_error_c(
            "'set_window' must be called before getting the root");
        return nullptr;
    }
    return Nst_inc_ref(app.root);
}

Nst_FUNC_SIGN(set_func_)
{
    GUI::Element *el;
    Nst_FuncObj *func;
    Nst_DEF_EXTRACT("# f", GUI::get_element_type(), &el, &func);
    if (el->el_type != GUI::GUI_ET_BUTTON) {
        Nst_set_value_error_c("the element must be a button");
        return nullptr;
    }
    if (func->arg_num != 1) {
        Nst_set_value_error_c(
            "the function of a button must take exactly 1 argument");
        return nullptr;
    }
    GUI::Button *b = (GUI::Button *)el;
    b->func = GUI::button_call_nest_func;
    b->nest_func = FUNC(Nst_inc_ref(func));
    return Nst_inc_ref(el);
}

Nst_FUNC_SIGN(get_builtin_font_)
{
    Nst_Obj *weight_obj, *size_obj;
    bool monospace, italic;
    Nst_DEF_EXTRACT("?i ?i y y", &weight_obj, &size_obj, &italic, &monospace);
    i64 weight = Nst_DEF_VAL(
        weight_obj,
        AS_INT(weight_obj),
        (i64)GUI::GUI_FW_REGULAR);
    i64 size = Nst_DEF_VAL(
        size_obj,
        AS_INT(size_obj),
        (i64)GUI::GUI_FS_MEDIUM);

    return OBJ(get_font(
        &app,
        (GUI::FontWeight)weight,
        (GUI::FontSize)size,
        (int)italic,
        (int)monospace));
}

Nst_FUNC_SIGN(open_font_)
{
    Nst_StrObj *path;
    i64 size;
    bool bold, italic, underline, strikethrough;
    Nst_DEF_EXTRACT(
        "s i y y y y",
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

    GUI::FontObj *font_obj = (GUI::FontObj *)GUI::font_new(font);
    if (font_obj == nullptr)
        TTF_CloseFont(font);
    return OBJ(font_obj);
}

Nst_FUNC_SIGN(match_x_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    Nst_Obj *side_from_obj;
    Nst_Obj *side_to_obj;
    Nst_DEF_EXTRACT(
        "# ?# ?i ?i",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &side_to_obj,
        &side_from_obj);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::Side side_to = Nst_DEF_VAL(
        side_to_obj,
        GUI::Side(AS_INT(side_to_obj)),
        GUI::LEFT);
    GUI::Side side_from = Nst_DEF_VAL(
        side_from_obj,
        GUI::Side(AS_INT(side_from_obj)),
        GUI::LEFT);

    GUI::match_x(from_el, to_el, side_from, side_to);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(match_y_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    Nst_Obj *side_from_obj;
    Nst_Obj *side_to_obj;
    Nst_DEF_EXTRACT(
        "# ?# ?i ?i",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &side_to_obj,
        &side_from_obj);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::Side side_to = Nst_DEF_VAL(
        side_to_obj,
        GUI::Side(AS_INT(side_to_obj)),
        GUI::TOP);
    GUI::Side side_from = Nst_DEF_VAL(
        side_from_obj,
        GUI::Side(AS_INT(side_from_obj)),
        GUI::TOP);

    GUI::match_y(from_el, to_el, side_from, side_to);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(match_pos_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    Nst_Obj *pos_from_obj;
    Nst_Obj *pos_to_obj;
    Nst_DEF_EXTRACT(
        "# ?# ?i ?i",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &pos_to_obj,
        &pos_from_obj);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::Pos pos_to = Nst_DEF_VAL(
        pos_to_obj,
        GUI::Pos(AS_INT(pos_to_obj)),
        GUI::TL);
    GUI::Pos pos_from = Nst_DEF_VAL(
        pos_from_obj,
        GUI::Pos(AS_INT(pos_from_obj)),
        GUI::TL);

    GUI::match_pos(from_el, to_el, pos_from, pos_to);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(match_w_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    bool from_p, to_p;
    Nst_DEF_EXTRACT(
        "# ?# y y",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &from_p,
        &to_p);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::match_w(from_el, to_el, from_p, to_p);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(match_h_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    bool from_p, to_p;
    Nst_DEF_EXTRACT(
        "# ?# y y",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &from_p,
        &to_p);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::match_h(from_el, to_el, from_p, to_p);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(match_size_)
{
    GUI::Element *to_el;
    GUI::Element *from_el;
    bool from_p, to_p;
    Nst_DEF_EXTRACT(
        "# ?# y y",
        GUI::get_element_type(), &to_el,
        GUI::get_element_type(), &from_el,
        &from_p,
        &to_p);

    if (OBJ(from_el) == Nst_null())
        from_el = to_el->parent;

    GUI::match_size(from_el, to_el, from_p, to_p);
    return OBJ(to_el);
}

Nst_FUNC_SIGN(perc_pos_)
{
    GUI::Element *el;
    Nst_Obj *x_obj;
    Nst_Obj *y_obj;
    Nst_Obj *pos_obj;
    Nst_DEF_EXTRACT(
        "# ?N ?N ?i",
        GUI::get_element_type(), &el,
        &x_obj, &y_obj, &pos_obj);
    fsize x = Nst_DEF_VAL(x_obj, fsize(AS_REAL(x_obj)), 1);
    fsize y = Nst_DEF_VAL(y_obj, fsize(AS_REAL(y_obj)), 1);
    GUI::Pos pos = Nst_DEF_VAL(pos_obj, GUI::Pos(AS_INT(pos_obj)), GUI::TL);

    GUI::perc_pos(el, x, y, pos);
    return OBJ(el);
}

Nst_FUNC_SIGN(diff_pos_)
{
    GUI::Element *el;
    Nst_Obj *x_obj;
    Nst_Obj *y_obj;
    Nst_DEF_EXTRACT(
        "# ?i ?i",
        GUI::get_element_type(), &el,
        &x_obj, &y_obj);
    i64 x = Nst_DEF_VAL(x_obj, AS_INT(x_obj), 0);
    i64 y = Nst_DEF_VAL(y_obj, AS_INT(y_obj), 0);

    GUI::diff_pos(el, int(x), int(y));
    return OBJ(el);
}

Nst_FUNC_SIGN(perc_size_)
{
    GUI::Element *el;
    Nst_Obj *w_obj;
    Nst_Obj *h_obj;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# ?N:r ?N:r y",
        GUI::get_element_type(), &el,
        &w_obj, &h_obj, &padding_rect);
    fsize w = Nst_DEF_VAL(w_obj, fsize(AS_REAL(w_obj)), 1);
    fsize h = Nst_DEF_VAL(h_obj, fsize(AS_REAL(h_obj)), 1);

    GUI::perc_size(el, w, h, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(diff_size_)
{
    GUI::Element *el;
    Nst_Obj *w_obj;
    Nst_Obj *h_obj;
    Nst_DEF_EXTRACT(
        "# ?i ?i",
        GUI::get_element_type(), &el,
        &w_obj, &h_obj);
    i64 w = Nst_DEF_VAL(w_obj, AS_INT(w_obj), 0);
    i64 h = Nst_DEF_VAL(h_obj, AS_INT(h_obj), 0);

    GUI::diff_size(el, int(w), int(h));
    return OBJ(el);
}

Nst_FUNC_SIGN(min_x_)
{
    GUI::Element *el;
    i64 min_x;
    Nst_Obj *side_obj;
    Nst_DEF_EXTRACT("# i ?i", GUI::get_element_type(), &el, &min_x, &side_obj);

    GUI::Side side = Nst_DEF_VAL(
        side_obj,
        GUI::Side(AS_INT(side_obj)),
        GUI::LEFT);

    GUI::min_x(el, (int)min_x, side);
    return OBJ(el);
}

Nst_FUNC_SIGN(min_y_)
{
    GUI::Element *el;
    i64 min_y;
    Nst_Obj *side_obj;
    Nst_DEF_EXTRACT("# i ?i", GUI::get_element_type(), &el, &min_y, &side_obj);

    GUI::Side side = Nst_DEF_VAL(
        side_obj,
        GUI::Side(AS_INT(side_obj)),
        GUI::TOP);

    GUI::min_y(el, (int)min_y, side);
    return OBJ(el);
}

Nst_FUNC_SIGN(min_pos_)
{
    GUI::Element *el;
    i64 min_x;
    i64 min_y;
    Nst_Obj *pos_obj;
    Nst_DEF_EXTRACT(
        "# i i ?i",
        GUI::get_element_type(), &el,
        &min_x, &min_y,
        &pos_obj);

    GUI::Pos pos = Nst_DEF_VAL(pos_obj, GUI::Pos(AS_INT(pos_obj)), GUI::TL);

    GUI::min_pos(el, (int)min_x, (int)min_y, pos);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_x_)
{
    GUI::Element *el;
    i64 max_x;
    Nst_Obj *side_obj;
    Nst_DEF_EXTRACT("# i ?i", GUI::get_element_type(), &el, &max_x, &side_obj);

    GUI::Side side = Nst_DEF_VAL(
        side_obj,
        GUI::Side(AS_INT(side_obj)),
        GUI::LEFT);

    GUI::max_x(el, (int)max_x, side);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_y_)
{
    GUI::Element *el;
    i64 max_y;
    Nst_Obj *side_obj;
    Nst_DEF_EXTRACT("# i ?i", GUI::get_element_type(), &el, &max_y, &side_obj);

    GUI::Side side = Nst_DEF_VAL(
        side_obj,
        GUI::Side(AS_INT(side_obj)),
        GUI::TOP);

    GUI::max_y(el, (int)max_y, side);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_pos_)
{
    GUI::Element *el;
    i64 max_x;
    i64 max_y;
    Nst_Obj *pos_obj;
    Nst_DEF_EXTRACT(
        "# i i ?i",
        GUI::get_element_type(), &el,
        &max_x, &max_y,
        &pos_obj);

    GUI::Pos pos = Nst_DEF_VAL(pos_obj, GUI::Pos(AS_INT(pos_obj)), GUI::TL);

    GUI::max_pos(el, (int)max_x, (int)max_y, pos);
    return OBJ(el);
}

Nst_FUNC_SIGN(min_w_)
{
    GUI::Element *el;
    i64 min_w;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i y",
        GUI::get_element_type(), &el,
        &min_w,
        &padding_rect);

    GUI::min_w(el, (int)min_w, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(min_h_)
{
    GUI::Element *el;
    i64 min_h;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i y",
        GUI::get_element_type(), &el,
        &min_h,
        &padding_rect);

    GUI::min_w(el, (int)min_h, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(min_size_)
{
    GUI::Element *el;
    i64 min_w;
    i64 min_h;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i i y",
        GUI::get_element_type(), &el,
        &min_w, &min_h,
        &padding_rect);

    GUI::min_size(el, (int)min_w, (int)min_h, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_w_)
{
    GUI::Element *el;
    i64 max_w;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i y",
        GUI::get_element_type(), &el,
        &max_w,
        &padding_rect);

    GUI::max_w(el, (int)max_w, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_h_)
{
    GUI::Element *el;
    i64 max_h;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i y",
        GUI::get_element_type(), &el,
        &max_h,
        &padding_rect);

    GUI::max_w(el, (int)max_h, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(max_size_)
{
    GUI::Element *el;
    i64 max_w;
    i64 max_h;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# i i y",
        GUI::get_element_type(), &el,
        &max_w, &max_h,
        &padding_rect);

    GUI::min_size(el, (int)max_w, (int)max_h, padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(fill_w_)
{
    GUI::Element *el;
    GUI::Element *left_el;
    GUI::Element *right_el;
    Nst_Obj *left_obj_side_obj;
    Nst_Obj *right_obj_side_obj;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# # # ?i ?i y",
        GUI::get_element_type(), &el,
        GUI::get_element_type(), &left_el,
        GUI::get_element_type(), &right_el,
        &left_obj_side_obj,
        &right_obj_side_obj,
        &padding_rect);

    GUI::Side left_obj_side = Nst_DEF_VAL(
        left_obj_side_obj,
        GUI::Side(AS_INT(left_obj_side_obj)),
        GUI::RIGHT);
    GUI::Side right_obj_side = Nst_DEF_VAL(
        right_obj_side_obj,
        GUI::Side(AS_INT(right_obj_side_obj)),
        GUI::LEFT);

    fill_w(
        el, left_el, right_el,
        left_obj_side, right_obj_side,
        padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(fill_h_)
{
    GUI::Element *el;
    GUI::Element *top_el;
    GUI::Element *bottom_el;
    Nst_Obj *top_obj_side_obj;
    Nst_Obj *bottom_obj_side_obj;
    bool padding_rect;
    Nst_DEF_EXTRACT(
        "# # # ?i ?i y",
        GUI::get_element_type(), &el,
        GUI::get_element_type(), &top_el,
        GUI::get_element_type(), &bottom_el,
        &top_obj_side_obj,
        &bottom_obj_side_obj,
        &padding_rect);

    GUI::Side top_obj_side = Nst_DEF_VAL(
        top_obj_side_obj,
        GUI::Side(AS_INT(top_obj_side_obj)),
        GUI::BOTTOM);
    GUI::Side bottom_obj_side = Nst_DEF_VAL(
        bottom_obj_side_obj,
        GUI::Side(AS_INT(bottom_obj_side_obj)),
        GUI::TOP);

    fill_h(
        el, top_el, bottom_el,
        top_obj_side, bottom_obj_side,
        padding_rect);
    return OBJ(el);
}

Nst_FUNC_SIGN(get_text_)
{
    GUI::Element *element;
    Nst_DEF_EXTRACT("#", GUI::get_element_type(), &element);

    Nst_Buffer *buf;
    if (element->el_type == GUI::GUI_ET_LABEL)
        buf = &((GUI::Label *)element)->text;
    else if (element->el_type == GUI::GUI_ET_BUTTON)
        buf = &((GUI::Button *)element)->text->text;
    else {
        Nst_set_type_errorf(
            "cannot get the text of '%s' elements",
            GUI::element_get_type_name(element));
        return nullptr;
    }

    Nst_Buffer new_buf;
    if (!Nst_buffer_copy(buf, &new_buf))
        return nullptr;
    Nst_StrObj *text = Nst_buffer_to_string(&new_buf);
    return OBJ(text);
}

Nst_FUNC_SIGN(set_text_)
{
    GUI::Element *element;
    Nst_StrObj *text;
    bool keep_same_size;
    Nst_DEF_EXTRACT(
        "# s y",
        GUI::get_element_type(),
        &element,
        &text,
        &keep_same_size);

    GUI::Label *label;
    if (element->el_type == GUI::GUI_ET_LABEL)
        label = (GUI::Label *)element;
    else if (element->el_type == GUI::GUI_ET_BUTTON)
        label = ((GUI::Button *)element)->text;
    else {
        Nst_set_type_errorf(
            "cannot set the text of '%s' elements",
            GUI::element_get_type_name(element));
        return nullptr;
    }

    GUI::label_set_text(label, text, !keep_same_size);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(_debug_view_)
{
    bool show_view;
    Nst_DEF_EXTRACT("y", &show_view);
    app.show_bounds = show_view;
    Nst_RETURN_NULL;
}
