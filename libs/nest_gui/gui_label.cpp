#include "gui_label.h"
#include "gui_draw.h"
#include "gui_event.h"

using namespace GUI;

static bool render_texture(Label *l)
{
    i32 p_top, p_left, p_bottom, p_right;
    element_get_padding((Element *)l, p_top, p_left, p_bottom, p_right);

    // TTF_SetFontWrappedAlign(l->font, l->alignment);
    l->texture_render_width = l->rect.w - p_left - p_right;

    SDL_Surface *text_surf = TTF_RenderUTF8_Blended_Wrapped(
        l->font->font,
        l->text.data,
        l->color,
        l->texture_render_width);
    if (text_surf == nullptr) {
        set_sdl_error();
        return false;
    }

    if (l->texture != nullptr)
        SDL_DestroyTexture(l->texture);

    l->texture = SDL_CreateTextureFromSurface(l->app->renderer, text_surf);
    if (l->auto_height)
        l->rect.h = p_bottom + p_top + text_surf->h;

    SDL_FreeSurface(text_surf);
    if (l->texture == nullptr) {
        set_sdl_error();
        return false;
    }
    return true;
}

bool GUI::label_update(Label *l)
{
    i32 p_top, p_left, p_bottom, p_right;
    element_get_padding((Element *)l, p_top, p_left, p_bottom, p_right);

    if (l->text.len == 0)
        return true;

    if (l->texture == nullptr
        || l->rect.w
           - p_left
           - p_right != l->texture_render_width)
    {
        if (!render_texture(l))
            return false;
    }

    SDL_Rect clip_rect = element_get_clip_rect((Element *)l);

    draw_texture(
        l->app,
        int(l->rect.x + p_left),
        int(l->rect.y + p_right),
        l->texture,
        &clip_rect);

    return true;
}

Element *GUI::label_new(Nst_StrObj *text, FontObj *font, SDL_Color color,
                        int x, int y, int w, int h, App *app)
{
    Label *new_label = (Label *)element_new(
        GUI_ET_LABEL,
        sizeof(Label),
        x, y, w + 7, h + 6,
        app,
        (Nst_ObjDstr)label_destroy);
    if (new_label == nullptr) {
        Nst_dec_ref(font);
        return nullptr;
    }

    // new_label->alignment = TTF_WRAPPED_ALIGN_LEFT;
    new_label->texture = nullptr;
    new_label->texture_render_width = 0;
    new_label->font = font;
    new_label->color = color;
    new_label->frame_update_func = UpdateFunc(label_update);
    new_label->handle_event_func = default_event_handler;
    new_label->auto_height = false;

    if (!Nst_buffer_init(&new_label->text, text->len + 1)) {
        element_destroy((Element *)new_label);
        Nst_dec_ref(font);
        return nullptr;
    }
    Nst_buffer_append(&new_label->text, text);
    element_set_padding((Element *)new_label, 3, 3, 3, 3);

    usize offset = 0;
    i8 *text_p = new_label->text.data;

    bool remove_r = false;
    bool needs_reformat = false;
    for (usize i = 0, n = text->len; i < n; i++) {
        if (text_p[i] == '\r') {
            needs_reformat = true;
            continue;
        }
        if (text_p[i] == '\n' && needs_reformat) {
            remove_r = true;
            break;
        }
    }

    if (!needs_reformat)
        goto end;

    for (usize i = 0, n = text->len; i < n; i++) {
        if (text_p[i] != '\r')
            text_p[i - offset] = text_p[i];
        else if (remove_r)
            offset++;
        else
            text_p[i] = '\n';
    }

    new_label->text.len -= offset;
    text_p[text->len] = '\0';

end:
    return (Element *)new_label;
}

static void reset_texture(Label *l, bool change_size)
{
    if (l->texture != nullptr) {
        SDL_DestroyTexture(l->texture);
        l->texture = nullptr;
    }
    if (change_size) {
        int new_w, new_h;
        TTF_SizeUTF8(l->font->font, l->text.data, &new_w, &new_h);
        new_w++;
        element_set_size((Element *)l, new_w, new_h, true);
    }
}

void GUI::label_destroy(Label *l)
{
    Nst_buffer_destroy(&l->text);
}

void GUI::label_change_color(Label *l, SDL_Color new_color)
{
    l->color = new_color;
    reset_texture(l, false);
}

void GUI::label_append_text(Label *l, Nst_StrObj *str, bool change_size)
{
    Nst_buffer_append(&l->text, str);
    reset_texture(l, change_size);
}

void GUI::label_append_c_text(Label *l, i8 *text, bool change_size)
{
    Nst_buffer_append_c_str(&l->text, text);
    reset_texture(l, change_size);
}

void GUI::label_set_text(Label *l, Nst_StrObj *str, bool change_size)
{
    i8 *new_data = Nst_realloc_c(
        l->text.data,
        str->len + 1,
        i8, l->text.cap);
    if (new_data == nullptr)
        return;

    memcpy(new_data, str->value, str->len + 1);
    l->text.data = new_data;
    l->text.cap = str->len + 1;
    l->text.len = str->len;
    reset_texture(l, change_size);
}

void GUI::label_set_c_text(Label *l, i8 *text, bool change_size)
{
    usize str_len = strlen(text);
    i8 *new_data = Nst_realloc_c(
        l->text.data,
        str_len + 1,
        i8, l->text.cap);
    if (new_data == nullptr)
        return;

    memcpy(new_data, text, str_len + 1);
    l->text.data = new_data;
    l->text.cap = str_len + 1;
    l->text.len = str_len;
    reset_texture(l, change_size);
}

void GUI::label_set_font(Label *l, FontObj *font, bool change_size)
{
    Nst_inc_ref(font);
    Nst_dec_ref(l->font);
    l->font = font;
    reset_texture(l, change_size);
}
