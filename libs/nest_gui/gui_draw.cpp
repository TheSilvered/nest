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

    // int mx = (l + r) / 2;
    // int my = (t + b) / 2;

    SDL_RenderDrawLine(renderer, l, t, r, t);
    SDL_RenderDrawLine(renderer, r, t, r, b);
    SDL_RenderDrawLine(renderer, l, b, r, b);
    SDL_RenderDrawLine(renderer, l, t, l, b);

    // SDL_RenderDrawLine(renderer, mx, my - 3, mx, my + 3);
    // SDL_RenderDrawLine(renderer, mx - 3, my, mx + 3, my);
}

static int fix_radius(int min_side, int r)
{
    if ( r * 2 > min_side )
    {
        return min_side / 2;
    }
    return r;
}

static int imin(int n1, int n2)
{
    if ( n2 < n1 )
    {
        return int(n2);
    }
    return int(n1);
}

static int imax(int n1, int n2)
{
    if ( n2 > n1 )
    {
        return int(n2);
    }
    return int(n1);
}

static void draw_angle_point(SDL_Renderer *renderer,
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
        SDL_SetRenderDrawColor(renderer, r, g, b, a);
        SDL_RenderDrawPoint(renderer, x, y);
    }
    else if ( dist < (rad+1)*(rad+1) )
    {
        u8 new_opacity = u8((1 - sqrt(dist) + rad) * a);
        SDL_SetRenderDrawColor(renderer, r, g, b, new_opacity);
        SDL_RenderDrawPoint(renderer, x, y);
    }
}

void draw_round_rect(SDL_Renderer *renderer,
                     SDL_Rect *rect,
                     int rtl, int rtr, int rbl, int rbr,
                     u8 r, u8 g, u8 b, u8 a)
{
    if ( rect == nullptr )
    {
        return;
    }

    if ( rect->w == 0 && rect->h == 0 )
    {
        return;
    }

    int x = rect->x;
    int y = rect->y;
    int w = rect->w;
    int h = rect->h;

    i32 min_side = w;
    if ( min_side > h )
    {
        min_side = h;
    }
    rtl = fix_radius(min_side, rtl);
    rtr = fix_radius(min_side, rtr);
    rbl = fix_radius(min_side, rbl);
    rbr = fix_radius(min_side, rbr);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, r, g, b, a);

    int max_l = imax(rtl, rbl);
    int max_r = imax(rtr, rbr);

    int min_l = imin(rtl, rbl);
    int min_r = imin(rtr, rbr);

    SDL_Rect cr = { x + max_l, y, w - max_r - max_l, h };
    SDL_RenderFillRect(renderer, &cr);

    cr = { x, y + rtl, min_l, h - rtl - rbl };
    SDL_RenderFillRect(renderer, &cr);

    cr = { x + min_l, y + (rtl == min_l ? 0 : rtl), max_l - min_l, h - (rtl == min_l ? rbl : rtl) };
    SDL_RenderFillRect(renderer, &cr);

    cr = { x + w - min_r, y + rtr, min_r, h - rtr - rbr };
    SDL_RenderFillRect(renderer, &cr);

    cr = { x + w - max_r, y + (rtr == min_r ? 0 : rtr), max_r - min_r, h - (rtr == min_r ? rbr : rtr) };
    SDL_RenderFillRect(renderer, &cr);

    int max_rad = imax(max_l, max_r);

    for ( int i = 0; i < max_rad; i++ )
    {
        for ( int j = 0; j < max_rad; j++ )
        {
            draw_angle_point(renderer, rtl, i, j, x + i, y + j, r, g, b, a);
            draw_angle_point(renderer, rtr, i, j, x + w - i - 1, y + j, r, g, b, a);
            draw_angle_point(renderer, rbr, i, j, x + w - i - 1, y + h - j - 1, r, g, b, a);
            draw_angle_point(renderer, rbl, i, j, x + i, y + h - j - 1, r, g, b, a);
        }
    }
}
