#include "gui_button.h"
#include "gui_draw.h"
#include "gui_event.h"

static void draw_button(GUI_Button *b)
{
    SDL_Texture* texture;
    SDL_Color c;
    SDL_Rect clip = b->rect;
    if (b->clip_parent)
    {
        clip = gui_element_get_padding_rect(b->parent);
    }

    if (b->textures[3] == nullptr)
    {
        c = b->app->fg_color;
        texture = draw_round_rect(
            b->app->renderer,
            b->rect,
            3, 3, 3, 3,
            c.r, c.g, c.b, c.a);
        b->textures[3] = texture;
    }
    draw_texture(b->app, b->rect.x, b->rect.y, b->textures[3], &clip);

    switch (b->cs)
    {
    case GUI_CS_IDLE:
        texture = b->textures[0];
        if (texture == nullptr)
        {
            SDL_Rect rect = b->rect;
            rect.x++; rect.y++;
            rect.w -= 2; rect.h -= 2;
            c = b->app->bg_light_color;
            texture = draw_round_rect(
                b->app->renderer,
                rect,
                3, 3, 3, 3,
                c.r, c.g, c.b, c.a);
            b->textures[0] = texture;
        }
        gui_label_change_color(b->text, b->app->fg_color);
        draw_texture(b->app, b->rect.x + 1, b->rect.y + 1, texture, &clip);
        break;
    case GUI_CS_HOVER:
    case GUI_CS_HOVER_IDLE:
        gui_label_change_color(b->text, b->app->bg_color);
        break;
    case GUI_CS_CLICKED:
        texture = b->textures[1];
        if (texture == nullptr)
        {
            SDL_Rect rect = b->rect;
            rect.x++; rect.y++;
            rect.w -= 2; rect.h -= 2;
            c = b->app->fg_dimmed_color;
            texture = draw_round_rect(
                b->app->renderer,
                rect,
                3, 3, 3, 3,
                c.r, c.g, c.b, c.a);
            b->textures[1] = texture;
        }
        gui_label_change_color(b->text, b->app->bg_color);
        draw_texture(b->app, b->rect.x + 1, b->rect.y + 1, texture, &clip);
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
    return ca;
}

bool gui_button_update(GUI_Button *b)
{
    SDL_Point m_pos;
    SDL_GetMouseState(&m_pos.x, &m_pos.y);
    SDL_Rect ca = clickable_area_rect(b);
    if ( !SDL_PointInRect(&m_pos, &ca) )
    {
        b->cs = GUI_CS_IDLE;
        if ( get_focused_element() == (GUI_Element *)b )
        {
            remove_focused_element();
        }
    }

    draw_button(b);
    return true;
}

i32 gui_button_handle_event(SDL_Event *e, GUI_Button *b)
{
    SDL_Rect ca = clickable_area_rect(b);
    switch ( e->type )
    {
    case SDL_MOUSEMOTION:
    {
        SDL_Point mp = { e->motion.x, e->motion.y };
        if ( !SDL_PointInRect(&mp, &ca) )
        {
            b->cs = GUI_CS_IDLE;
            if ( get_focused_element() == (GUI_Element *)b )
            {
                remove_focused_element();
            }
            return 0;
        }

        if ( get_focused_element() != nullptr )
        {
            return 0;
        }

        if ( b->cs == GUI_CS_IDLE )
        {
            b->cs = GUI_CS_HOVER;
        }
        set_focused_element((GUI_Element *)b);
        return 0;
    }
    case SDL_MOUSEBUTTONDOWN:
    {
        SDL_Point mp = { e->button.x, e->button.y };
        if ( b->cs != GUI_CS_HOVER && !SDL_PointInRect(&mp, &ca) )
        {
            return 0;
        }
        if ( get_focused_element() != (GUI_Element *)b &&
             get_focused_element() != nullptr )
        {
            return 0;
        }

        set_focused_element((GUI_Element *)b);
        b->cs = GUI_CS_CLICKED;
        return 1;
    }
    case SDL_MOUSEBUTTONUP:
    {
        SDL_Point mp = { e->button.x, e->button.y };
        if ( !SDL_PointInRect(&mp, &ca) )
        {
            b->cs = GUI_CS_IDLE;
            return 0;
        }

        if ( b->cs != GUI_CS_CLICKED )
        {
            return 0;
        }

        if ( get_focused_element() != (GUI_Element *)b &&
             get_focused_element() != nullptr )
        {
            return 0;
        }

        set_focused_element((GUI_Element *)b);
        b->cs = GUI_CS_HOVER;
        b->current_state++;
        b->current_state %= b->number_of_states;
        bool res = b->func == nullptr ? true : b->func(b);
        return res ? 1 : -1;
    }
    default:
        return default_event_handler(e, (GUI_Element *)b);
    }
}

void gui_button_destroy(GUI_Button *b)
{
    Nst_dec_ref(b->text);
    if ( b->nest_func != nullptr )
    {
        Nst_dec_ref(b->nest_func);
    }

    for ( int i = 0; i < 5; i++)
    {
        SDL_DestroyTexture(b->textures[i]);
    }
    gui_element_destroy((GUI_Element *)b);
}

GUI_Element *gui_button_new(GUI_Label *text, GUI_App *app)
{
    GUI_Button *b = (GUI_Button *)gui_element_new(
        GUI_ET_BUTTON,
        sizeof(GUI_Button),
        0, 0,
        text->rect.w + 14, text->rect.h + 10,
        app);
    if ( b == nullptr )
    {
        Nst_dec_ref(text);
        return nullptr;
    }

    gui_element_set_rel_pos(
        (GUI_Element *)text,
        (GUI_Element *)b,
        GUI_RECT_ELEMENT,
        GUI_MIDDLE, GUI_CENTER,
        GUI_MIDDLE, GUI_CENTER);
    gui_element_clip_parent((GUI_Element *)text, true);
    gui_element_set_padding((GUI_Element *)b, 5, 7, 5, 7);
    gui_element_set_margin((GUI_Element *)b, 5, 5, 5, 5);

    if ( !gui_element_add_child((GUI_Element *)b, (GUI_Element *)text) )
    {

    }

    b->text = text;
    b->cs = GUI_CS_IDLE;
    b->number_of_states = 1;
    b->current_state = 0;
    b->disabled = false;
    b->func = nullptr;
    b->nest_func = nullptr;
    b->clickable_area = b->rect;

    for ( int i = 0; i < 5; i++)
    {
        b->textures[i] = nullptr;
    }

    text->handle_event_func = nullptr;
    b->frame_update_func = (UpdateFunc)gui_button_update;
    b->handle_event_func = (HandleEventFunc)gui_button_handle_event;
    b->destructor = (Nst_ObjDestructor)gui_button_destroy;

    return (GUI_Element *)b;
}

bool gui_button_call_nest_func(GUI_Button *b)
{
    if ( b->nest_func == nullptr )
    {
        return true;
    }

    Nst_Obj *arg = Nst_int_new(b->current_state);
    if ( arg == nullptr )
    {
        return false;
    }
    Nst_Obj *result = Nst_call_func(b->nest_func, &arg);
    Nst_dec_ref(arg);
    if ( result == nullptr )
    {
        return false;
    }
    Nst_dec_ref(result);
    return true;
}
