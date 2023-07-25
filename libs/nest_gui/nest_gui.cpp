#include <SDL.h>
#include <SDL_ttf.h>
#include <cstring>
#include "nest_gui.h"
#include "gui_event.h"
#include "gui_update.h"
#include "gui_label.h"
#include "gui_button.h"
#include "gui_stack_layout.h"

#define FUNC_COUNT 18

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *sdl_error_str;
static GUI_App app;
Nst_TypeObj *gui_element_type;

bool lib_init()
{
    usize idx = 0;

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(init_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(loop_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_window_, 6);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(label_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(button_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(stack_layout_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_position_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_rel_position_, 7);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_size_, 3);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_rel_size_, 9);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_margins_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_padding_, 5);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(show_overflow_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(auto_height_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(add_child_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_root_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(set_func_, 2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_debug_view_, 1);

#if __LINE__ - FUNC_COUNT != 25
#error
#endif

    sdl_error_str = STR(Nst_string_new_c_raw("SDL Error", false));
    gui_element_type = Nst_type_new("GUI Element", 11);

    app.root = nullptr;
    app.focused_element = nullptr;
    app.window = nullptr;
    app.renderer = nullptr;
    app.clip_window = { 0, 0, 0, 0 };

    app.keep_open = false;
    app.show_bounds = false;

    app.regular_small = nullptr;
    app.italic_small = nullptr;
    app.bold_small = nullptr;
    app.regular_medium = nullptr;
    app.italic_medium = nullptr;
    app.bold_medium = nullptr;
    app.regular_big = nullptr;
    app.italic_big = nullptr;
    app.bold_big = nullptr;

    app.bg_color = { 0, 0, 0, 255 };
    app.fg_color = { 255, 255, 255, 255 };
    app.bg_light_color = { 60, 60, 60, 255 };
    app.fg_dimmed_color = { 160, 160, 160, 255 };

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

    if ( app.window != nullptr )
    {
        SDL_DestroyWindow(app.window);
        SDL_DestroyRenderer(app.renderer);
        Nst_dec_ref(app.root);
    }

    TTF_Quit();
    SDL_Quit();
}

void set_sdl_error()
{
    const i8 *sdl_error = SDL_GetError();
    usize len = strlen(sdl_error);
    i8 *new_error = (i8 *)Nst_calloc(
        1,
        sizeof(i8) * (len + 1),
        (void *)sdl_error); // copies the string if the allocation succeded
    if ( new_error == nullptr )
    {
        return;
    }
    Nst_StrObj *msg = STR(Nst_string_new(new_error, len, true));
    if ( msg == nullptr )
    {
        Nst_free(new_error);
        return;
    }
    Nst_set_error(Nst_inc_ref(sdl_error_str), msg);
}

int imin(int n1, int n2)
{
    if ( n2 < n1 )
    {
        return int(n2);
    }
    return int(n1);
}

int imax(int n1, int n2)
{
    if ( n2 > n1 )
    {
        return int(n2);
    }
    return int(n1);
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
    if ( app.focused_element != nullptr )
    {
        Nst_dec_ref(app.focused_element);
        app.focused_element = nullptr;
    }
}

Nst_FUNC_SIGN(init_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    if ( SDL_WasInit(SDL_INIT_EVERYTHING) == SDL_INIT_EVERYTHING )
    {
        Nst_RETURN_NULL;
    }

    if ( SDL_Init(SDL_INIT_EVERYTHING) || TTF_Init() )
    {
        set_sdl_error();
        return nullptr;
    }
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(loop_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    while ( app.keep_open )
    {
        if ( !handle_events(&app) )
        {
            return nullptr;
        }
        if ( !update_elements(&app) )
        {
            return nullptr;
        }
        if ( !tick_elements(&app) )
        {
            return nullptr;
        }

        SDL_RenderPresent(app.renderer);
    }

    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(set_window_)
{
    if ( app.window != nullptr )
    {
        Nst_set_call_error_c("'set_window' was called more than once");
        return nullptr;
    }

    Nst_StrObj *title;
    Nst_Int w, h;
    Nst_Obj *flags_obj;
    Nst_Obj *pos_x_obj, *pos_y_obj;
    int flags, pos_x, pos_y;

    Nst_DEF_EXTRACT("sii?i?i?i", &title, &w, &h, &flags_obj, &pos_x_obj, &pos_y_obj);
    flags = Nst_DEF_VAL(flags_obj, int(AS_INT(flags_obj)), 0);
    pos_x = Nst_DEF_VAL(pos_x_obj, int(AS_INT(pos_x_obj)), SDL_WINDOWPOS_CENTERED);
    pos_y = Nst_DEF_VAL(pos_y_obj, int(AS_INT(pos_y_obj)), SDL_WINDOWPOS_CENTERED);

    app.window = SDL_CreateWindow(title->value, pos_x, pos_y, int(w), int(h), flags);
    if ( app.window == nullptr )
    {
        set_sdl_error();
        return nullptr;
    }
    app.renderer = SDL_CreateRenderer(app.window, -1, 0);
    if ( app.renderer == nullptr )
    {
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
    if ( root == nullptr )
    {
        SDL_DestroyWindow(app.window);
        SDL_DestroyRenderer(app.renderer);
        return nullptr;
    }
    root->handle_event_func = root_handle_event;
    root->frame_update_func = root_update;
    app.root = root;
    gui_element_set_parent(root, root);
    gui_element_set_rel_size(
        root, nullptr,
        GUI_RECT_ELEMENT,
        -1, -1,
        -1, -1,
        1.0, 1.0,
        0, 0);
    gui_element_set_padding(root, 5, 5, 5, 5);

    app.keep_open = true;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(label_)
{
    Nst_StrObj *text;
    Nst_SeqObj *color;
    Nst_Obj *size_obj, *style_obj, *weight_obj;
    u8 r, g, b, a;

    Nst_DEF_EXTRACT(
        "s?i?i?i?A.i",
        &text,
        &size_obj, &style_obj, &weight_obj,
        &color);

    Nst_Int size   = Nst_DEF_VAL(size_obj,   AS_INT(size_obj),   -1);
    Nst_Int style  = Nst_DEF_VAL(style_obj,  AS_INT(style_obj),  -1);
    Nst_Int weight = Nst_DEF_VAL(weight_obj, AS_INT(weight_obj), -1);

    if ( OBJ(color) == Nst_null() )
    {
        r = app.fg_color.r;
        g = app.fg_color.g;
        b = app.fg_color.b;
        a = app.fg_color.a;
    }
    else if ( color->len == 3 )
    {
        r = u8(AS_INT(color->objs[0]));
        g = u8(AS_INT(color->objs[1]));
        b = u8(AS_INT(color->objs[2]));
        a = 255;
    }
    else if ( color->len == 4 )
    {
        r = u8(AS_INT(color->objs[0]));
        g = u8(AS_INT(color->objs[1]));
        b = u8(AS_INT(color->objs[2]));
        a = u8(AS_INT(color->objs[4]));
    }
    else
    {
        Nst_set_value_error_c("the color must be of length 3 or 4");
        return nullptr;
    }

    TTF_Font *font = get_font(
        &app,
        (GUI_FontSize)size,
        (GUI_FontStyle)style,
        (GUI_FontWeight)weight);
    if ( font == nullptr )
    {
        return nullptr;
    }

    int w, h;
    TTF_SizeUTF8(font, text->value, &w, &h);

    GUI_Element *label = gui_label_new(
        text, font,
        { r, g, b, a },
        0, 0, w + 1, h,
        &app);

    if ( label == nullptr )
    {
        return nullptr;
    }
    gui_element_set_margin(label, 0, 0, 10, 0);
    return OBJ(label);
}

Nst_FUNC_SIGN(button_)
{
    Nst_StrObj *text;
    Nst_DEF_EXTRACT("s", &text);

    TTF_Font *font = get_font(
        &app,
        GUI_FSZ_MEDIUM,
        GUI_FST_REGULAR,
        GUI_FW_REGULAR);

    if ( font == nullptr )
    {
        return nullptr;
    }

    int w, h;
    TTF_SizeUTF8(font, text->value, &w, &h);

    GUI_Element *label = gui_label_new(
        text, font,
        app.fg_color,
        0, 0, w + 1, h,
        &app);

    if ( label == nullptr )
    {
        return nullptr;
    }

    GUI_Element *button = gui_button_new((GUI_Label *)label, &app);
    if ( button == nullptr )
    {
        return nullptr;
    }
    return OBJ(button);
}

Nst_FUNC_SIGN(stack_layout_)
{
    Nst_Obj *direction_obj;
    Nst_Obj *alignment_obj;

    Nst_DEF_EXTRACT("?i?i", &direction_obj, &alignment_obj);
    Nst_Int direction = Nst_DEF_VAL(direction_obj, AS_INT(direction_obj), 0);
    Nst_Int alignment = Nst_DEF_VAL(alignment_obj, AS_INT(alignment_obj), 0);

    if ( direction < 0 || direction > 3 )
    {
        Nst_set_value_error_c("invalid direction");
        return nullptr;
    }
    if ( alignment < 0 || alignment > 2 )
    {
        Nst_set_value_error_c("invalid alignment");
        return nullptr;
    }

    return OBJ(gui_stack_layout_new(
        (GUI_StackDir)direction,
        (GUI_StackAlign)alignment,
        0, 0, 0, 0,
        &app));
}

Nst_FUNC_SIGN(set_position_)
{
    GUI_Element *element;
    Nst_Int x, y;
    Nst_DEF_EXTRACT("#ii", gui_element_type, &element, &x, &y);
    element->rect.x = (int)x;
    element->rect.y = (int)y;
    if ( Nst_FLAG_HAS(element, GUI_FLAG_REL_POS) )
    {
        Nst_FLAG_DEL(element, GUI_FLAG_REL_POS);
        Nst_dec_ref(element->rel_pos.element);
        element->rel_pos.element = nullptr;
    }
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(set_rel_position_)
{
    GUI_Element *from_element, *to_element;
    Nst_Int from_x, from_y, to_x, to_y;
    Nst_Obj *to_x_obj, *to_y_obj;
    Nst_Obj *rect_obj;

    Nst_DEF_EXTRACT(
        "?##ii?i?i?i",
        gui_element_type, &from_element,
        gui_element_type, &to_element,
        &from_x, &from_y, &rect_obj,
        &to_x_obj, &to_y_obj);

    to_x = Nst_DEF_VAL(to_x_obj, AS_INT(to_x_obj), from_x);
    to_y = Nst_DEF_VAL(to_y_obj, AS_INT(to_y_obj), from_y);
    Nst_Int rect = Nst_DEF_VAL(rect_obj, AS_INT(rect_obj), 0);

    if ( OBJ(from_element) == Nst_null() )
    {
        Nst_dec_ref(from_element);
        from_element = nullptr;
    }

    gui_element_set_rel_pos(
        to_element, from_element,
        (GUI_RelRect)rect,
        (GUI_RelPosX)from_x, (GUI_RelPosY)from_y,
        (GUI_RelPosX)to_x,   (GUI_RelPosY)to_y);
    return Nst_inc_ref(from_element);
}

Nst_FUNC_SIGN(set_size_)
{
    GUI_Element *element;
    Nst_Int w, h;
    Nst_DEF_EXTRACT("#ii", gui_element_type, &element, &w, &h);
    element->rect.w = (int)w;
    element->rect.h = (int)h;
    if ( Nst_FLAG_HAS(element, GUI_FLAG_REL_SIZE) )
    {
        Nst_FLAG_DEL(element, GUI_FLAG_REL_SIZE);
        if ( element->rel_size.element != nullptr )
        {
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
    Nst_Int min_w, min_h, max_w, max_h;
    Nst_Obj *rsr_obj;

    Nst_DEF_EXTRACT(
        "?##i|ri|r?i?i?i?i?i",
        gui_element_type, &from_element,
        gui_element_type, &to_element,
        &size_x, &size_y,
        &rsr_obj,
        &min_w_obj, &min_h_obj, &max_w_obj, &max_h_obj);

    if ( OBJ(from_element) == Nst_null() )
    {
        Nst_dec_ref(from_element);
        from_element = nullptr;
    }

    min_w = Nst_DEF_VAL(min_w_obj, AS_INT(min_w_obj), -1);
    min_h = Nst_DEF_VAL(min_h_obj, AS_INT(min_h_obj), -1);
    max_w = Nst_DEF_VAL(max_w_obj, AS_INT(max_w_obj), -1);
    max_h = Nst_DEF_VAL(max_h_obj, AS_INT(max_h_obj), -1);
    Nst_Int rsr = Nst_DEF_VAL(
        rsr_obj,
        AS_INT(rsr_obj),
        (Nst_Int)GUI_RECT_PADDING);

    i32 diff_x = 0, diff_y = 0;
    f64 scale_x = 0.0, scale_y = 0.0;

    if ( size_x->type == Nst_type()->Int )
    {
        diff_x = (i32)AS_INT(size_x);
    }
    else
    {
        scale_x = AS_REAL(size_x);
    }

    if ( size_y->type == Nst_type()->Int )
    {
        diff_y = (i32)AS_INT(size_y);
    }
    else
    {
        scale_y = AS_REAL(size_y);
    }

    gui_element_set_rel_size(
        to_element, from_element,
        (GUI_RelRect)rsr,
        (int)min_w, (int)min_h, (int)max_w, (int)max_h,
        scale_x, scale_y,
        diff_x, diff_y);

    return Nst_inc_ref(from_element);
}

Nst_FUNC_SIGN(set_margins_)
{
    GUI_Element *element;
    Nst_Int l, r, t, b;
    Nst_DEF_EXTRACT("#iiii", gui_element_type, &element, &l, &r, &t, &b);

    gui_element_set_margin(element, (i32)t, (i32)l, (i32)b, (i32)r);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(set_padding_)
{
    GUI_Element *element;
    Nst_Int l, r, t, b;
    Nst_DEF_EXTRACT("#iiii", gui_element_type, &element, &l, &r, &t, &b);

    gui_element_set_padding(element, (i32)t, (i32)l, (i32)b, (i32)r);
    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(show_overflow_)
{
    GUI_Element *element;
    Nst_Bool overflow;

    Nst_DEF_EXTRACT("#o_b", gui_element_type, &element, &overflow);

    gui_element_clip_content(element, !overflow);

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(auto_height_)
{
    GUI_Element *element;
    Nst_Bool auto_height;

    Nst_DEF_EXTRACT("#o_b", gui_element_type, &element, &auto_height);

    if ( element->el_type == GUI_ET_LABEL )
    {
        ((GUI_Label *)element)->auto_height = auto_height;
    }

    return Nst_inc_ref(element);
}

Nst_FUNC_SIGN(add_child_)
{
    GUI_Element *parent, *child;
    Nst_DEF_EXTRACT("##", gui_element_type, &parent, gui_element_type, &child);
    if ( !gui_element_add_child(parent, child) )
    {
        return nullptr;
    }
    return Nst_inc_ref(parent);
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

Nst_FUNC_SIGN(_debug_view_)
{
    Nst_Bool show_view;
    Nst_DEF_EXTRACT("o_b", &show_view);
    app.show_bounds = show_view;
    Nst_RETURN_NULL;
}
