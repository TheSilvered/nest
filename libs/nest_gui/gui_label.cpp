#include "gui_label.h"
#include "gui_draw.h"
#include "gui_event.h"

bool render_texture(GUI_Label *l)
{
    // TTF_SetFontWrappedAlign(l->font, l->alignment);
    l->texture_render_width = l->rect.w - l->padding_left - l->padding_right;

    SDL_Surface *text_surf = TTF_RenderUTF8_Blended_Wrapped(
        l->font,
        l->text.data,
        l->color,
        l->texture_render_width);
    if ( text_surf == nullptr )
    {
        set_sdl_error();
        return false;
    }

    if ( l->texture != nullptr)
    {
        SDL_DestroyTexture(l->texture);
    }

    l->texture = SDL_CreateTextureFromSurface(l->app->renderer, text_surf);
    if ( l->auto_height &&
         (!Nst_FLAG_HAS(l, GUI_FLAG_REL_SIZE) ||
         (l->rel_size.diff_y == 0 && l->rel_size.scale_y == 0.0)) )
    {
        l->rect.h = l->padding_bottom + l->padding_top + text_surf->h;
    }

    SDL_FreeSurface(text_surf);
    if ( l->texture == nullptr )
    {
        set_sdl_error();
        return false;
    }
    return true;
}

bool gui_label_update(GUI_Label *l)
{
    if ( l->text.len == 0 )
    {
        return true;
    }

    if ( l->texture == nullptr ||
         l->rect.w - l->padding_left - l->padding_right != l->texture_render_width )
    {
        if ( !render_texture(l) )
        {
            return false;
        }
    }

    SDL_Rect clip_rect = l->app->clip_window;
    SDL_Rect padding_rect = gui_element_get_padding_rect((GUI_Element *)l);

    if ( l->clip_content && !l->clip_parent )
    {
        clip_rect = padding_rect;
    }
    else if ( !l->clip_content && l->clip_parent )
    {
        clip_rect = gui_element_get_padding_rect(l->parent);
    }
    else if ( l->clip_content && l->clip_parent )
    {
        SDL_Rect parent_rect = gui_element_get_padding_rect(l->parent);
        if ( !SDL_IntersectRect(&padding_rect, &parent_rect, &clip_rect) )
        {
            return true;
        }
    }

    draw_texture(
        l->app,
        int(l->rect.x + l->padding_left),
        int(l->rect.y + l->padding_top),
        l->texture,
        &clip_rect);

    return true;
}

GUI_Element *gui_label_new(Nst_StrObj *text,
                           TTF_Font   *font,
                           SDL_Color  color,
                           int x, int y, int w, int h,
                           GUI_App *app)
{
    GUI_Label *new_label = (GUI_Label *)gui_element_new(
        GUI_ET_LABEL,
        sizeof(GUI_Label),
        x, y, w, h,
        app);
    if ( new_label == nullptr )
    {
        return nullptr;
    }

    new_label->destructor = (Nst_ObjDestructor)gui_label_destroy;
    // new_label->alignment = TTF_WRAPPED_ALIGN_LEFT;
    new_label->texture = nullptr;
    new_label->texture_render_width = 0;
    new_label->font = font;
    new_label->color = color;
    new_label->frame_update_func = UpdateFunc(gui_label_update);
    new_label->handle_event_func = default_event_handler;
    new_label->auto_height = false;

    if ( !Nst_buffer_init(&new_label->text, text->len + 1) )
    {
        gui_element_destroy((GUI_Element *)new_label);
        return nullptr;
    }
    Nst_buffer_append(&new_label->text, text);

    usize offset = 0;
    i8 *text_p = new_label->text.data;

    bool remove_r = false;
    bool needs_reformat = false;
    for ( usize i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] == '\r' )
        {
            needs_reformat = true;
            continue;
        }
        if ( text_p[i] == '\n' && needs_reformat )
        {
            remove_r = true;
            break;
        }
    }

    if ( !needs_reformat )
    {
        goto end;
    }

    for ( usize i = 0, n = text->len; i < n; i++ )
    {
        if ( text_p[i] != '\r' )
        {
            text_p[i - offset] = text_p[i];
        }
        else if ( remove_r )
        {
            offset++;
        }
        else
        {
            text_p[i] = '\n';
        }
    }

    new_label->text.len -= offset;
    text_p[text->len] = '\0';

end:
    return (GUI_Element *)new_label;
}

void reset_texture(GUI_Label *l)
{
    if ( l->texture != nullptr)
    {
        SDL_DestroyTexture(l->texture);
        l->texture = nullptr;
    }
}

void gui_label_destroy(GUI_Label *l)
{
    Nst_buffer_destroy(&l->text);
    gui_element_destroy((GUI_Element *)l);
}

void gui_label_change_color(GUI_Label *l, SDL_Color new_color)
{
    l->color = new_color;
    reset_texture(l);
}

void gui_label_append_text(GUI_Label *l, Nst_StrObj *str)
{
    Nst_buffer_append(&l->text, str);
    reset_texture(l);
}

void gui_label_append_c_text(GUI_Label *l, i8 *text)
{
    Nst_buffer_append_c_str(&l->text, text);
    reset_texture(l);
}

void gui_label_set_text(GUI_Label *l, Nst_StrObj *str)
{
    i8 *new_data = Nst_realloc_c(
        l->text.data,
        str->len + 1,
        i8, l->text.size);
    if ( new_data == nullptr )
    {
        return;
    }

    memcpy(new_data, str->value, str->len + 1);
    l->text.data = new_data;
    l->text.size = str->len + 1;
    l->text.len = str->len;
    reset_texture(l);
}

void gui_label_set_c_text(GUI_Label *l, i8 *text)
{
    usize str_len = strlen(text);
    i8 *new_data = Nst_realloc_c(
        l->text.data,
        str_len + 1,
        i8, l->text.size);
    if ( new_data == nullptr )
    {
        return;
    }

    memcpy(new_data, text, str_len + 1);
    l->text.data = new_data;
    l->text.size = str_len + 1;
    l->text.len = str_len;
    reset_texture(l);
}
