#include "gui_draw.h"

void draw_rect(SDL_Renderer *renderer, SDL_Rect *rect)
{
    if ( rect == nullptr )
    {
        return;
    }

    if ( rect->w == 0 && rect->h == 0 )
    {
        return;
    }

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
    if ( r * 2 > min_side )
    {
        return min_side / 2;
    }
    return r;
}

static void draw_point(SDL_Surface *surf, int x, int y, u8 r, u8 g, u8 b, u8 a)
{
    SDL_Rect rect = { x, y, 1, 1 };
    SDL_FillRect(surf, &rect, (a << 24) + (r << 16) + (g << 8) + b);
}

static void draw_angle_point(SDL_Surface *surf,
                             int rad,
                             int i, int j, int x, int y,
                             u8 r, u8 g, u8 b, u8 a)
{
    if ( rad - i <= 0 || rad - j <= 0 )
    {
        return;
    }

    i32 dist = (rad-i) * (rad-i) + (rad-j) * (rad-j);

    if ( dist < rad*rad )
    {
        draw_point(surf, x, y, r, g, b, a);
    }
    else if ( dist < (rad+1)*(rad+1) )
    {
        u8 new_opacity = u8((1 - sqrt(dist) + rad) * a);
        draw_point(surf, x, y, r, g, b, new_opacity);
    }
}

SDL_Texture *draw_round_rect(SDL_Renderer *renderer,
                             SDL_Rect rect,
                             int rtl, int rtr, int rbl, int rbr,
                             u8 r, u8 g, u8 b, u8 a)
{
    SDL_Surface *surf = SDL_CreateRGBSurface(
        0, rect.w, rect.h, 32,
        0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
    SDL_Texture *texture;

    if ( surf == nullptr )
    {
        return nullptr;
    }

    if ( rect.w == 0 && rect.h == 0 )
    {
        texture = SDL_CreateTextureFromSurface(renderer, surf);
        SDL_FreeSurface(surf);
        return texture;
    }

    i32 min_side = rect.w;
    if ( min_side > rect.h )
    {
        min_side = rect.h;
    }
    rtl = fix_radius(min_side, rtl);
    rtr = fix_radius(min_side, rtr);
    rbl = fix_radius(min_side, rbl);
    rbr = fix_radius(min_side, rbr);

    u32 color = (a << 24) + (r << 16) + (g << 8) + b;

    int max_l = imax(rtl, rbl);
    int max_r = imax(rtr, rbr);

    int min_l = imin(rtl, rbl);
    int min_r = imin(rtr, rbr);

    SDL_Rect cr = { max_l, 0, rect.w - max_r - max_l, rect.h };
    SDL_FillRect(surf, &cr, color);

    cr = { 0, rtl, min_l, rect.h - rtl - rbl };
    SDL_FillRect(surf, &cr, color);

    cr = {  min_l, rtl == min_l ? 0 : rtl, max_l - min_l, rect.h - (rtl == min_l ? rbl : rtl) };
    SDL_FillRect(surf, &cr, color);

    cr = { rect.w - min_r, rtr, min_r, rect.h - rtr - rbr };
    SDL_FillRect(surf, &cr, color);

    cr = { rect.w - max_r, rtr == min_r ? 0 : rtr, max_r - min_r, rect.h - (rtr == min_r ? rbr : rtr) };
    SDL_FillRect(surf, &cr, color);

    int max_rad = imax(max_l, max_r);

    for ( int i = 0; i < max_rad; i++ )
    {
        for ( int j = 0; j < max_rad; j++ )
        {
            draw_angle_point(surf, rtl, i, j, i, j, r, g, b, a);
            draw_angle_point(surf, rtr, i, j, rect.w - i - 1, j, r, g, b, a);
            draw_angle_point(surf, rbr, i, j, rect.w - i - 1, rect.h - j - 1, r, g, b, a);
            draw_angle_point(surf, rbl, i, j, i, rect.h - j - 1, r, g, b, a);
        }
    }

    texture = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}

void draw_texture(GUI_App *app, int x, int y, SDL_Texture *texture, SDL_Rect *clip)
{
    int w, h;
    SDL_QueryTexture(texture, nullptr, nullptr, &w, &h);
    SDL_Rect texture_rect = { x, y, w, h };

    if ( clip == nullptr )
    {
        SDL_RenderCopy(app->renderer, texture, NULL, &texture_rect);
        return;
    }

    SDL_Rect dst_rect;
    if ( !SDL_IntersectRect(clip, &texture_rect, &dst_rect) )
    {
        return;
    }
    SDL_Rect src_rect = { dst_rect.x - x, dst_rect.y - y, dst_rect.w, dst_rect.h };
    SDL_RenderCopy(app->renderer, texture, &src_rect, &dst_rect);
}
