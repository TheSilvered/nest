#include "gui_button.h"
#include "gui_draw.h"
#include "gui_event.h"

#define BUTTON_RAD 5

static void draw_button(GUI_Button *b)
{
    SDL_Texture* texture;
    SDL_Color c_i, c_b;
    SDL_Rect clip = gui_element_get_clip_rect((GUI_Element *)b);

    switch (b->cs) {
    case GUI_CS_IDLE:
        texture = b->textures[0];
        if (texture == nullptr) {
            c_i = b->app->bg_color2;
            c_b = b->app->bg_dark;
            texture = draw_round_border_rect(
                b->app->renderer,
                b->rect, 1,
                BUTTON_RAD, BUTTON_RAD, BUTTON_RAD, BUTTON_RAD,
                c_i.r, c_i.g, c_i.b, c_i.a,
                c_b.r, c_b.g, c_b.b, c_b.a);
            b->textures[0] = texture;
        }
        gui_label_change_color(b->text, b->app->fg_color);
        draw_texture(b->app, b->rect.x, b->rect.y, texture, &clip);
        break;
    case GUI_CS_HOVER:
    case GUI_CS_HOVER_IDLE:
        texture = b->textures[1];
        if (texture == nullptr) {
            c_i = b->app->bg_color3;
            c_b = b->app->bg_color3;
            texture = draw_round_border_rect(
                b->app->renderer,
                b->rect, 1,
                BUTTON_RAD, BUTTON_RAD, BUTTON_RAD, BUTTON_RAD,
                c_i.r, c_i.g, c_i.b, c_i.a,
                c_b.r, c_b.g, c_b.b, c_b.a);
            b->textures[1] = texture;
        }
        gui_label_change_color(b->text, b->app->fg_color);
        draw_texture(b->app, b->rect.x, b->rect.y, texture, &clip);
        break;
    case GUI_CS_CLICKED:
        texture = b->textures[2];
        if (texture == nullptr) {
            c_i = b->app->bg_color2;
            c_b = b->app->bg_color3;
            texture = draw_round_border_rect(
                b->app->renderer,
                b->rect, 1,
                BUTTON_RAD, BUTTON_RAD, BUTTON_RAD, BUTTON_RAD,
                c_i.r, c_i.g, c_i.b, c_i.a,
                c_b.r, c_b.g, c_b.b, c_b.a);
            b->textures[2] = texture;
        }
        gui_label_change_color(b->text, b->app->fg_color);
        draw_texture(b->app, b->rect.x, b->rect.y, texture, &clip);
        break;
    default:
        break;
    }
}

static SDL_Rect clickable_area_rect(GUI_Button *b)
{
    SDL_Rect ca = b->clickable_area;
    ca.x += b->rect.x;
    ca.y += b->rect.y;
    SDL_Rect clip = gui_element_get_clip_rect((GUI_Element *)b);
    SDL_IntersectRect(&ca, &clip, &ca);
    return ca;
}

bool gui_button_update(GUI_Button *b)
{
    draw_button(b);
    return true;
}

i32 gui_button_handle_event(SDL_Event *e, GUI_Button *b)
{
    SDL_Rect ca = clickable_area_rect(b);
    switch (e->type) {
    case SDL_MOUSEMOTION: {
        SDL_Point mp = { e->motion.x, e->motion.y };
        if (!SDL_PointInRect(&mp, &ca) || IS_HIDDEN(b)) {
            b->cs = GUI_CS_IDLE;
            return 0;
        }

        if (get_focused_element() != nullptr || b->app->element_reached) {
            b->cs = GUI_CS_IDLE;
            return 0;
        }

        b->app->element_reached = true;

        if (b->cs == GUI_CS_IDLE)
            b->cs = GUI_CS_HOVER;
        return 0;
    }
    case SDL_MOUSEBUTTONDOWN: {
        SDL_Point mp = { e->button.x, e->button.y };
        if (b->cs != GUI_CS_HOVER || !SDL_PointInRect(&mp, &ca)
            || get_focused_element() != nullptr || b->app->element_reached
            || IS_HIDDEN(b))
        {
            b->cs = GUI_CS_IDLE;
            return 0;
        }

        b->app->element_reached = true;

        b->cs = GUI_CS_CLICKED;
        return 1;
    }
    case SDL_MOUSEBUTTONUP: {
        SDL_Point mp = { e->button.x, e->button.y };
        if (!SDL_PointInRect(&mp, &ca) || get_focused_element() != nullptr
            || b->app->element_reached || IS_HIDDEN(b)) {
            b->cs = GUI_CS_IDLE;
            return 0;
        }

        b->app->element_reached = true;

        if (b->cs != GUI_CS_CLICKED) {
            b->cs = GUI_CS_HOVER;
            return 0;
        }

        b->cs = GUI_CS_HOVER;
        b->current_state++;
        b->current_state %= b->number_of_states;
        bool res = b->func == nullptr ? true : b->func(b);
        return res ? 1 : -1;
    }
    case SDL_USEREVENT:
        if (e->user.data1 != b || e->user.code != GUI_UE_RESIZED)
            return 0;
        if (b->textures[0] != nullptr) {
            SDL_DestroyTexture(b->textures[0]);
            b->textures[0] = nullptr;
        }
        if (b->textures[1] != nullptr) {
            SDL_DestroyTexture(b->textures[1]);
            b->textures[1] = nullptr;
        }
        if (b->textures[2] != nullptr) {
            SDL_DestroyTexture(b->textures[2]);
            b->textures[2] = nullptr;
        }
        b->clickable_area.w = b->rect.w;
        b->clickable_area.h = b->rect.h;
        return 1;
    default:
        return default_event_handler(e, (GUI_Element *)b);
    }
}

void gui_button_destroy(GUI_Button *b)
{
    Nst_dec_ref(b->text);
    Nst_ndec_ref(b->nest_func);

    for (int i = 0; i < 5; i++) {
        if (b->textures[i] != nullptr)
            SDL_DestroyTexture(b->textures[i]);
    }
}

GUI_Element *gui_button_new(GUI_Label *text, GUI_App *app)
{
    GUI_Button *b = (GUI_Button *)gui_element_new(
        GUI_ET_BUTTON,
        sizeof(GUI_Button),
        0, 0,
        text->rect.w + 14, text->rect.h + 5,
        app,
        (Nst_ObjDstr)gui_button_destroy);
    if (b == nullptr) {
        Nst_dec_ref(text);
        return nullptr;
    }

    gui_element_set_rel_pos(
        (GUI_Element *)text, (GUI_Element *)b,
        GUI_RECT_PADDING, GUI_RECT_ELEMENT,
        GUI_MIDDLE, GUI_CENTER,
        GUI_MIDDLE, GUI_CENTER);
    gui_element_clip_parent((GUI_Element *)text, true);
    gui_element_set_padding((GUI_Element *)b, 2, 7, 3, 7);
    gui_element_set_margin((GUI_Element *)b, 5, 5, 5, 5);

    if (!gui_element_add_child((GUI_Element *)b, (GUI_Element *)text)) {
        Nst_dec_ref(text);
        Nst_dec_ref(b);
        return nullptr;
    }

    b->text = text;
    b->cs = GUI_CS_IDLE;
    b->number_of_states = 1;
    b->current_state = 0;
    b->disabled = false;
    b->func = nullptr;
    b->nest_func = nullptr;
    b->clickable_area = b->rect;

    for (int i = 0; i < 5; i++)
        b->textures[i] = nullptr;

    text->handle_event_func = nullptr;
    b->frame_update_func = (UpdateFunc)gui_button_update;
    b->handle_event_func = (HandleEventFunc)gui_button_handle_event;

    return (GUI_Element *)b;
}

bool gui_button_call_nest_func(GUI_Button *b)
{
    if (b->nest_func == nullptr)
        return true;

    Nst_Obj *arg = Nst_int_new(b->current_state);
    if (arg == nullptr)
        return false;
    Nst_Obj *result = Nst_call_func(b->nest_func, &arg);
    Nst_dec_ref(arg);
    if (result == nullptr)
        return false;
    Nst_dec_ref(result);
    return true;
}
