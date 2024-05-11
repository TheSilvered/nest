#include "gui_draw.h"
#include "gui_utils.h"

void GUI_draw_rect(SDL_Renderer *renderer, SDL_Rect *rect)
{
    if (rect == nullptr)
        return;

    if (rect->w == 0 && rect->h == 0)
        return;

    int l = rect->x;
    int r = l + rect->w - 1;
    int t = rect->y;
    int b = t + rect->h - 1;

    SDL_RenderDrawLine(renderer, l, t, r, t);
    SDL_RenderDrawLine(renderer, r, t, r, b);
    SDL_RenderDrawLine(renderer, l, b, r, b);
    SDL_RenderDrawLine(renderer, l, t, l, b);
}

static int fix_radius(int min_side, int r)
{
    if (r * 2 > min_side)
        return min_side / 2;
    return r;
}

static void draw_point(SDL_Surface *surf, int x, int y, u8 r, u8 g, u8 b, u8 a)
{
    SDL_Rect rect = { x, y, 1, 1 };
    SDL_FillRect(surf, &rect, (a << 24) + (r << 16) + (g << 8) + b);
}

static void draw_angle_point(SDL_Surface *surf, int rad,
                             int i, int j, int x, int y,
                             u8 r, u8 g, u8 b, u8 a)
{
    if (rad - i <= 0 || rad - j <= 0)
        return;

    i32 dist = (rad-i) * (rad-i) + (rad-j) * (rad-j);

    if (dist < rad * rad)
        draw_point(surf, x, y, r, g, b, a);
    else if (dist < (rad+1) * (rad+1)) {
        u8 new_opacity = u8((1 - sqrt(dist) + rad) * a);
        draw_point(surf, x, y, r, g, b, new_opacity);
    }
}

static void draw_angle_border_point(SDL_Surface *surf, int rad, int border,
                                    int i, int j, int x, int y,
                                    u8 r_i, u8 g_i, u8 b_i, u8 a_i,
                                    u8 r_b, u8 g_b, u8 b_b, u8 a_b)
{
    if (rad - i <= 0 || rad - j <= 0)
        return;

    i32 dist = (rad-i) * (rad-i) + (rad-j) * (rad-j);
    int b_rad = rad - border;
    if (dist < (b_rad) * (b_rad))
        draw_point(surf, x, y, r_i, g_i, b_i, a_i);
    else if (dist < (b_rad+1) * (b_rad+1)) {
        double t = 1 - sqrt(dist) + b_rad;
        u8 new_r = u8(t * r_i + (1 - t) * r_b);
        u8 new_g = u8(t * g_i + (1 - t) * g_b);
        u8 new_b = u8(t * b_i + (1 - t) * b_b);
        u8 new_a = u8(t * a_i + (1 - t) * a_b);
        draw_point(surf, x, y, new_r, new_g, new_b, new_a);
    } else if (dist < rad * rad)
        draw_point(surf, x, y, r_b, g_b, b_b, a_b);
    else if (dist < (rad+1) * (rad+1)) {
        u8 new_opacity = u8((1 - sqrt(dist) + rad) * a_b);
        draw_point(surf, x, y, r_b, g_b, b_b, new_opacity);
    }
}

SDL_Texture *GUI_draw_round_rect(SDL_Renderer *renderer, SDL_Rect rect,
                                 int rtl, int rtr, int rbl, int rbr,
                                 u8 r, u8 g, u8 b, u8 a)
{
    SDL_Surface *surf = SDL_CreateRGBSurface(
        0, rect.w, rect.h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture *texture;

    if (surf == nullptr)
        return nullptr;

    if (rect.w == 0 && rect.h == 0) {
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        return texture;
    }

    i32 min_side = rect.w;
    if (min_side > rect.h)
        min_side = rect.h;
    rtl = fix_radius(min_side, rtl);
    rtr = fix_radius(min_side, rtr);
    rbl = fix_radius(min_side, rbl);
    rbr = fix_radius(min_side, rbr);

    u32 color = (a << 24) + (r << 16) + (g << 8) + b;

    SDL_Rect cr = { 0, 0, rect.w, rect.h };
    SDL_FillRect(surf, &cr, color);

    cr = { 0, 0, rtl, rtl };
    SDL_FillRect(surf, &cr, 0);

    cr = { rect.w - rtr, 0, rtr, rtr };
    SDL_FillRect(surf, &cr, 0);

    cr = { 0, rect.h - rbl, rbl, rbl };
    SDL_FillRect(surf, &cr, 0);

    cr = { rect.w - rbr, rect.h - rbr, rbr, rbr };
    SDL_FillRect(surf, &cr, 0);

    int max_rad = max_int(max_int(rtl, rtr), max_int(rbl, rbr));

    for (int i = 0; i < max_rad; i++) {
        for (int j = 0; j < max_rad; j++) {
            draw_angle_point(
                surf, rtl,
                i, j, i, j,
                r, g, b, a);
            draw_angle_point(
                surf, rtr,
                i, j, rect.w - i - 1, j,
                r, g, b, a);
            draw_angle_point(
                surf, rbr,
                i, j, rect.w - i - 1, rect.h - j - 1,
                r, g, b, a);
            draw_angle_point(
                surf, rbl,
                i, j, i, rect.h - j - 1,
                r, g, b, a);
        }
    }

    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}

SDL_Texture *GUI_draw_round_border_rect(SDL_Renderer *renderer, SDL_Rect rect,
                                        int border_thickness,
                                        int rtl, int rtr, int rbl, int rbr,
                                        u8 r_i, u8 g_i, u8 b_i, u8 a_i,
                                        u8 r_b, u8 g_b, u8 b_b, u8 a_b)
{
    if (border_thickness == 0) {
        return GUI_draw_round_rect(
            renderer,
            rect,
            rtl, rtr, rbl, rbr,
            r_i, g_i, b_i, a_i);
    }

    SDL_Surface *surf = SDL_CreateRGBSurface(
        0, rect.w, rect.h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture *texture;

    if (surf == nullptr)
        return nullptr;

    if (rect.w == 0 && rect.h == 0) {
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        return texture;
    }

    i32 min_side = rect.w;
    if (min_side > rect.h)
        min_side = rect.h;
    rtl = fix_radius(min_side, rtl);
    rtr = fix_radius(min_side, rtr);
    rbl = fix_radius(min_side, rbl);
    rbr = fix_radius(min_side, rbr);

    u32 i_color = (a_i << 24) + (r_i << 16) + (g_i << 8) + b_i;
    u32 b_color = (a_b << 24) + (r_b << 16) + (g_b << 8) + b_b;

    SDL_Rect cr = { 0, 0, rect.w, rect.h };
    SDL_FillRect(surf, &cr, b_color);

    cr = {
        border_thickness, border_thickness,
        rect.w - border_thickness * 2,
        rect.h - border_thickness * 2 };
    SDL_FillRect(surf, &cr, i_color);

    cr = { 0, 0, rtl, rtl };
    SDL_FillRect(surf, &cr, 0);

    cr = { rect.w - rtr, 0, rtr, rtr };
    SDL_FillRect(surf, &cr, 0);

    cr = { 0, rect.h - rbl, rbl, rbl };
    SDL_FillRect(surf, &cr, 0);

    cr = { rect.w - rbr, rect.h - rbr, rbr, rbr };
    SDL_FillRect(surf, &cr, 0);

    int max_rad = max_int(max_int(rtl, rtr), max_int(rbl, rbr));

    for (int i = 0; i < max_rad; i++) {
        for (int j = 0; j < max_rad; j++) {
            draw_angle_border_point(
                surf,
                rtl, border_thickness,
                i, j, i, j,
                r_i, g_i, b_i, a_i,
                r_b, g_b, b_b, a_b);
            draw_angle_border_point(
                surf,
                rtr, border_thickness,
                i, j, rect.w - i - 1, j,
                r_i, g_i, b_i, a_i,
                r_b, g_b, b_b, a_b);
            draw_angle_border_point(
                surf,
                rbr, border_thickness,
                i, j, rect.w - i - 1, rect.h - j - 1,
                r_i, g_i, b_i, a_i,
                r_b, g_b, b_b, a_b);
            draw_angle_border_point(
                surf,
                rbl, border_thickness,
                i, j, i, rect.h - j - 1,
                r_i, g_i, b_i, a_i,
                r_b, g_b, b_b, a_b);
        }
    }

    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}

void GUI_draw_texture(SDL_Renderer *renderer, int x, int y,
                      SDL_Texture *texture, SDL_Rect *clip)
{
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect texture_rect = { x, y, w, h };

    if (clip == nullptr) {
        SDL_RenderCopy(renderer, texture, nullptr, &texture_rect);
        return;
    }

    SDL_Rect dst_rect;
    if (!SDL_IntersectRect(clip, &texture_rect, &dst_rect))
        return;
    SDL_Rect src_rect = { dst_rect.x - x, dst_rect.y - y, dst_rect.w, dst_rect.h };
    SDL_RenderCopy(renderer, texture, &src_rect, &dst_rect);
}
