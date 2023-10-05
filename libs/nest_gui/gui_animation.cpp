#include "gui_animation.h"

using namespace std;
using namespace std::chrono;
using namespace GUI;

typedef high_resolution_clock hr_clock;
typedef milliseconds ms;

static Nst_TypeObj *ani_type = nullptr;

bool GUI::ani_init()
{
    ani_type = Nst_cont_type_new(
        "GUI Animation",
        (Nst_ObjDstr)GUI::ani_destroy,
        (Nst_ObjTrav)GUI::ani_traverse);
    return ani_type != nullptr;
}

void GUI::ani_quit()
{
    Nst_dec_ref(ani_type);
}

Nst_Obj *GUI::get_ani_type()
{
    return OBJ(ani_type);
}

Nst_Obj *GUI::ani_new(AniUpdateFunc update_func, double duration, bool loop)
{
    AniObj *ani = Nst_obj_alloc(AniObj, ani_type);
    Nst_sbuffer_init(&ani->data, sizeof(AniDataEntry), 5);
    ani->duration = chrono::duration<i32, milli>(i32(duration * 1000));
    ani->start_time = hr_clock::now();
    ani->loop = loop;
    ani->running = false;
    ani->update_func = update_func;

    Nst_GGC_OBJ_INIT(GGC_OBJ(ani));

    return OBJ(ani);
}

void GUI::ani_destroy(AniObj *ani)
{
    for (usize i = 0; i < ani->data.len; i++)
        entry_free_data(((AniDataEntry *)ani->data.data)[i]);
}

void GUI::ani_traverse(AniObj *ani)
{
    for (usize i = 0; i < ani->data.len; i++)
        entry_track_data(((AniDataEntry *)ani->data.data)[i]);
}

void GUI::ani_add_data(AniObj *ani, void *value, EntryType type)
{
    AniDataEntry entry;
    entry_init(entry, type, value);
    Nst_sbuffer_append(&ani->data, &entry);
}

AniDataEntry &GUI::ani_get_data(AniObj *ani, usize index)
{
    return ((AniDataEntry *)ani->data.data)[index];
}

void GUI::ani_set_data(AniObj *ani, usize index, void *value, EntryType type,
                       bool free_prev)
{
    if (free_prev)
        entry_free_data(((AniDataEntry *)ani->data.data)[index]);

    AniDataEntry entry;
    entry_init(entry, type, value);
    ((AniDataEntry *)ani->data.data)[index] = entry;
}

void GUI::ani_start(AniObj *ani)
{
    ani->running = true;
    ani->start_time = hr_clock::now();
}

void GUI::ani_stop(AniObj *ani)
{
    ani->running = false;
}

bool GUI::ani_is_running(AniObj *ani)
{
    return ani->running;
}

void GUI::ani_update(AniObj *ani)
{
    auto now = hr_clock::now();
    if (now - ani->start_time > ani->duration) {
        if (ani->loop)
            ani->start_time = now;
        else {
            ani->running = false;
            return;
        }
    }

    i64 time_passed = (duration_cast<ms>(now - ani->start_time)).count();
    i64 max_time = ani->duration.count();

    ani->update_func(ani, double(time_passed) / double(max_time));

    return;
}

void GUI::entry_init(AniDataEntry &entry, EntryType type, void *data_ptr)
{
    switch (type) {
    case ET_NEST_OBJECT:
        Nst_ninc_ref(data_ptr);
        break;
    default:
        break;
    }
    entry.data_type = type;
    entry.data = data_ptr;
}

void GUI::entry_free_data(AniDataEntry &entry)
{
    switch (entry.data_type) {
    case ET_NEST_OBJECT:
        Nst_ndec_ref(entry.data);
        break;
    case ET_HEAP_OBJECT:
        Nst_free(entry.data);
        break;
    default:
        break;
    }
}

void GUI::entry_track_data(AniDataEntry &entry)
{
    switch (entry.data_type) {
    case ET_NEST_OBJECT:
        if (entry.data != nullptr)
            Nst_ggc_obj_reachable(entry.data);
        break;
    default:
        break;
    }
}

#ifndef Nst_WIN
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

Element *GUI::entry_get_element(AniDataEntry &entry)
{
    return (Element *)entry.data;
}

fsize GUI::entry_get_fsize(AniDataEntry &entry)
{
    return *(fsize *)&entry.data;
}

int GUI::entry_get_int(AniDataEntry &entry)
{
    return int(isize(entry.data));
}

i32 GUI::entry_get_i32(AniDataEntry &entry)
{
    return i32(isize(entry.data));
}

u32 GUI::entry_get_u32(AniDataEntry &entry)
{
    return u32(usize(entry.data));
}

usize GUI::entry_get_usize(AniDataEntry &entry)
{
    return usize(entry.data);
}

isize GUI::entry_get_isize(AniDataEntry &entry)
{
    return isize(entry.data);
}

Side GUI::entry_get_side(AniDataEntry &entry)
{
    return Side(isize(entry.data));
}

Pos GUI::entry_get_pos(AniDataEntry &entry)
{
    return Pos(isize(entry.data));
}

bool GUI::entry_get_bool(AniDataEntry &entry)
{
    return entry.data != nullptr;
}

void *GUI::entry_raw(AniDataEntry &entry)
{
    return entry.data;
}

static void match_x_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    Side from_side = entry_get_side(ani_get_data(ani, 2));
    Side to_side   = entry_get_side(ani_get_data(ani, 3));

    element_set_x(to_el, to_side, element_get_x(from_el, from_side));
}

static void match_y_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    Side from_side = entry_get_side(ani_get_data(ani, 2));
    Side to_side   = entry_get_side(ani_get_data(ani, 3));

    element_set_y(to_el, to_side, element_get_y(from_el, from_side));
}

static void match_pos_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    Pos from_pos = entry_get_pos(ani_get_data(ani, 2));
    Pos to_pos   = entry_get_pos(ani_get_data(ani, 3));
    int x, y;
    element_get_pos(from_el, from_pos, x, y);
    element_set_pos(to_el, to_pos, x, y);
}

static void match_w_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    bool from_p = entry_get_bool(ani_get_data(ani, 2));
    bool to_p   = entry_get_bool(ani_get_data(ani, 3));

    element_set_w(to_el, element_get_w(from_el, from_p), to_p);
}

static void match_h_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    bool from_p = entry_get_bool(ani_get_data(ani, 2));
    bool to_p   = entry_get_bool(ani_get_data(ani, 3));

    element_set_h(to_el, element_get_h(from_el, from_p), to_p);
}

static void match_size_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *from_el = entry_get_element(ani_get_data(ani, 0));
    Element *to_el   = entry_get_element(ani_get_data(ani, 1));
    bool from_p = entry_get_bool(ani_get_data(ani, 2));
    bool to_p   = entry_get_bool(ani_get_data(ani, 3));

    int w, h;
    element_get_size(from_el, w, h, from_p);
    element_set_size(to_el, w, h, to_p);
}

static void perc_pos_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    fsize x_perc = entry_get_fsize(ani_get_data(ani, 1));
    fsize y_perc = entry_get_fsize(ani_get_data(ani, 2));
    Pos pos = entry_get_pos(ani_get_data(ani, 3));

    int x, y;

    element_get_pos(el, pos, x, y);
    element_set_pos(el, pos, int(x * x_perc), int(y * y_perc));
}

static void diff_pos_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int x_diff = entry_get_int(ani_get_data(ani, 1));
    int y_diff = entry_get_int(ani_get_data(ani, 2));

    int x, y;

    element_get_pos(el, TL, x, y);
    element_set_pos(el, TL, x + x_diff, y + y_diff);
}

static void perc_size_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    fsize w_perc = entry_get_fsize(ani_get_data(ani, 1));
    fsize h_perc = entry_get_fsize(ani_get_data(ani, 2));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 3));

    int w, h;

    element_get_size(el, w, h, padding_rect);
    element_set_size(el, int(w * w_perc), int(h * h_perc), padding_rect);
}

static void diff_size_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int w_diff = entry_get_int(ani_get_data(ani, 1));
    int h_diff = entry_get_int(ani_get_data(ani, 2));

    int w, h;

    element_get_size(el, w, h, false);
    element_set_size(el, w + w_diff, h + h_diff, false);
}

static void min_x_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_x = entry_get_int(ani_get_data(ani, 1));
    Side side = entry_get_side(ani_get_data(ani, 2));

    int x = element_get_x(el, side);
    if (x < min_x)
        element_set_x(el, side, min_x);
}

static void min_y_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_y = entry_get_int(ani_get_data(ani, 1));
    Side side = entry_get_side(ani_get_data(ani, 2));

    int y = element_get_y(el, side);
    if (y < min_y)
        element_set_y(el, side, min_y);
}

static void min_pos_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_x = entry_get_int(ani_get_data(ani, 1));
    int min_y = entry_get_int(ani_get_data(ani, 2));
    Pos pos = entry_get_pos(ani_get_data(ani, 3));

    int x, y;
    element_get_pos(el, pos, x, y);

    if (x < min_x)
        x = min_x;
    if (y < min_y)
        y = min_y;
    element_set_pos(el, pos, x, y);
}

static void max_x_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_x = entry_get_int(ani_get_data(ani, 1));
    Side side = entry_get_side(ani_get_data(ani, 2));

    int x = element_get_x(el, side);
    if (x > max_x)
        element_set_x(el, side, max_x);
}

static void max_y_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_y = entry_get_int(ani_get_data(ani, 1));
    Side side = entry_get_side(ani_get_data(ani, 2));

    int y = element_get_y(el, side);
    if (y > max_y)
        element_set_y(el, side, max_y);
}

static void max_pos_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_x = entry_get_int(ani_get_data(ani, 1));
    int max_y = entry_get_int(ani_get_data(ani, 2));
    Pos pos = entry_get_pos(ani_get_data(ani, 3));

    int x, y;
    element_get_pos(el, pos, x, y);

    if (x > max_x)
        x = max_x;
    if (y > max_y)
        y = max_y;
    element_set_pos(el, pos, x, y);
}

static void min_w_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_w = entry_get_int(ani_get_data(ani, 1));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int w = element_get_w(el, padding_rect);
    if (w < min_w)
        element_set_w(el, min_w, padding_rect);
}

static void min_h_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_h = entry_get_int(ani_get_data(ani, 1));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int h = element_get_h(el, padding_rect);
    if (h < min_h)
        element_set_h(el, min_h, padding_rect);
}

static void min_size_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int min_w = entry_get_int(ani_get_data(ani, 1));
    int min_h = entry_get_int(ani_get_data(ani, 2));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int w, h;
    element_get_size(el, w, h, padding_rect);

    if (w < min_w)
        w = min_w;
    if (h < min_h)
        h = min_h;
    element_set_size(el, w, h, padding_rect);
}

static void max_w_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_w = entry_get_int(ani_get_data(ani, 1));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int w = element_get_w(el, padding_rect);
    if (w > max_w)
        element_set_w(el, max_w, padding_rect);
}

static void max_h_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_h = entry_get_int(ani_get_data(ani, 1));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int h = element_get_h(el, padding_rect);
    if (h > max_h)
        element_set_h(el, max_h, padding_rect);
}

static void max_size_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el = entry_get_element(ani_get_data(ani, 0));
    int max_w = entry_get_int(ani_get_data(ani, 1));
    int max_h = entry_get_int(ani_get_data(ani, 2));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 2));

    int w, h;
    element_get_size(el, w, h, padding_rect);

    if (w > max_w)
        w = max_w;
    if (h > max_h)
        h = max_h;
    element_set_size(el, w, h, padding_rect);
}

static void fill_between_w_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el    = entry_get_element(ani_get_data(ani, 0));
    Element *left  = entry_get_element(ani_get_data(ani, 1));
    Element *right = entry_get_element(ani_get_data(ani, 2));
    Side left_side  = entry_get_side(ani_get_data(ani, 3));
    Side right_side = entry_get_side(ani_get_data(ani, 4));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 5));

    int left_x = element_get_x(left, left_side);
    int right_x = element_get_x(right, right_side);
    if (right_x - left_x < 0)
        left_x = right_x;

    element_set_w(el, right_x - left_x, padding_rect);
}

static void fill_between_h_update(AniObj *ani, double t)
{
    Nst_UNUSED(t);
    Element *el     = entry_get_element(ani_get_data(ani, 0));
    Element *top    = entry_get_element(ani_get_data(ani, 1));
    Element *bottom = entry_get_element(ani_get_data(ani, 2));
    Side top_side    = entry_get_side(ani_get_data(ani, 3));
    Side bottom_side = entry_get_side(ani_get_data(ani, 4));
    bool padding_rect = entry_get_bool(ani_get_data(ani, 5));

    int top_y = element_get_y(top, top_side);
    int bottom_y = element_get_y(bottom, bottom_side);
    if (bottom_y - top_y < 0)
        top_y = bottom_y;

    element_set_h(el, bottom_y - top_y, padding_rect);
}

void GUI::match_x(Element *from_el, Element *to_el, Side from, Side to)
{
    AniObj *ani = (AniObj *)ani_new(match_x_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from,    ET_OTHER);
    ani_add_data(ani, (void *)to,      ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_X_ANI_NAME, ani);
}

void GUI::match_y(Element *from_el, Element *to_el, Side from, Side to)
{
    AniObj *ani = (AniObj *)ani_new(match_y_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from,    ET_OTHER);
    ani_add_data(ani, (void *)to,      ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_Y_ANI_NAME, ani);
}

void GUI::match_pos(Element *from_el, Element *to_el, Pos from, Pos to)
{
    AniObj *ani = (AniObj *)ani_new(match_pos_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from,    ET_OTHER);
    ani_add_data(ani, (void *)to,      ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_POS_ANI_NAME, ani);
}

void GUI::match_w(Element *from_el, Element *to_el, bool from_p, bool to_p)
{
    AniObj *ani = (AniObj *)ani_new(match_w_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from_p,  ET_OTHER);
    ani_add_data(ani, (void *)to_p,    ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_W_ANI_NAME, ani);
}

void GUI::match_h(Element *from_el, Element *to_el, bool from_p, bool to_p)
{
    AniObj *ani = (AniObj *)ani_new(match_h_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from_p,  ET_OTHER);
    ani_add_data(ani, (void *)to_p,    ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_H_ANI_NAME, ani);
}

void GUI::match_size(Element *from_el, Element *to_el, bool from_p, bool to_p)
{
    AniObj *ani = (AniObj *)ani_new(match_size_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)from_el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)to_el,   ET_NEST_OBJECT);
    ani_add_data(ani, (void *)from_p,  ET_OTHER);
    ani_add_data(ani, (void *)to_p,    ET_OTHER);
    ani_start(ani);

    element_set_ani(to_el, MATCH_SIZE_ANI_NAME, ani);
}

void GUI::perc_pos(Element *el, fsize x, fsize y, Pos pos)
{
    AniObj *ani = (AniObj *)ani_new(perc_pos_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)*(usize *)&x, ET_OTHER);
    ani_add_data(ani, (void *)*(usize *)&y, ET_OTHER);
    ani_add_data(ani, (void *)pos, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, PERC_POS_ANI_NAME, ani);
}

void GUI::diff_pos(Element *el, int x, int y)
{
    AniObj *ani = (AniObj *)ani_new(diff_pos_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)x, ET_OTHER);
    ani_add_data(ani, (void *)(isize)y, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, DIFF_POS_ANI_NAME, ani);
}

void GUI::perc_size(Element *el, fsize w, fsize h, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(perc_size_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)*(usize *)&w, ET_OTHER);
    ani_add_data(ani, (void *)*(usize *)&h, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, PERC_SIZE_ANI_NAME, ani);
}

void GUI::diff_size(Element *el, int w, int h)
{
    AniObj *ani = (AniObj *)ani_new(diff_size_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)w, ET_OTHER);
    ani_add_data(ani, (void *)(isize)h, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, DIFF_SIZE_ANI_NAME, ani);
}

void GUI::min_x(Element *el, int min_x, Side side)
{
    AniObj *ani = (AniObj *)ani_new(min_x_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_x, ET_OTHER);
    ani_add_data(ani, (void *)side, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_X_ANI_NAME, ani);
}

void GUI::min_y(Element *el, int min_y, Side side)
{
    AniObj *ani = (AniObj *)ani_new(min_y_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_y, ET_OTHER);
    ani_add_data(ani, (void *)side, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_Y_ANI_NAME, ani);
}

void GUI::min_pos(Element *el, int min_x, int min_y, Pos pos)
{
    AniObj *ani = (AniObj *)ani_new(min_pos_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_x, ET_OTHER);
    ani_add_data(ani, (void *)(isize)min_y, ET_OTHER);
    ani_add_data(ani, (void *)pos, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_POS_ANI_NAME, ani);
}

void GUI::max_x(Element *el, int max_x, Side side)
{
    AniObj *ani = (AniObj *)ani_new(max_x_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_x, ET_OTHER);
    ani_add_data(ani, (void *)side, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_X_ANI_NAME, ani);
}

void GUI::max_y(Element *el, int max_y, Side side)
{
    AniObj *ani = (AniObj *)ani_new(max_y_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_y, ET_OTHER);
    ani_add_data(ani, (void *)side, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_Y_ANI_NAME, ani);
}

void GUI::max_pos(Element *el, int max_x, int max_y, Pos pos)
{
    AniObj *ani = (AniObj *)ani_new(max_pos_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_x, ET_OTHER);
    ani_add_data(ani, (void *)(isize)max_y, ET_OTHER);
    ani_add_data(ani, (void *)pos, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_POS_ANI_NAME, ani);
}

void GUI::min_w(Element *el, int min_w, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(min_w_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_w, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_W_ANI_NAME, ani);
}

void GUI::min_h(Element *el, int min_h, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(min_h_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_h, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_H_ANI_NAME, ani);
}

void GUI::min_size(Element *el, int min_w, int min_h, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(min_size_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)min_w, ET_OTHER);
    ani_add_data(ani, (void *)(isize)min_h, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MIN_SIZE_ANI_NAME, ani);
}

void GUI::max_w(Element *el, int max_w, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(max_w_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_w, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_W_ANI_NAME, ani);
}

void GUI::max_h(Element *el, int max_h, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(max_h_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_h, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_H_ANI_NAME, ani);
}

void GUI::max_size(Element *el, int max_w, int max_h, bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(max_size_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)(isize)max_w, ET_OTHER);
    ani_add_data(ani, (void *)(isize)max_h, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, MAX_SIZE_ANI_NAME, ani);
}

void GUI::fill_w(Element *el, Element *left, Element *right,
                         Side left_obj_side, Side right_obj_side,
                         bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(fill_between_w_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el,    ET_NEST_OBJECT);
    ani_add_data(ani, (void *)left,  ET_NEST_OBJECT);
    ani_add_data(ani, (void *)right, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)left_obj_side,  ET_OTHER);
    ani_add_data(ani, (void *)right_obj_side, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect, ET_OTHER);
    ani_start(ani);

    element_set_ani(el, FILL_BETWEEN_W_ANI_NAME, ani);
}

void GUI::fill_h(Element *el, Element *top, Element *bottom,
                         Side top_obj_side, Side bottom_obj_side,
                         bool padding_rect)
{
    AniObj *ani = (AniObj *)ani_new(fill_between_h_update, 1, true);
    if (ani == nullptr)
        return;

    ani_add_data(ani, (void *)el,     ET_NEST_OBJECT);
    ani_add_data(ani, (void *)top,    ET_NEST_OBJECT);
    ani_add_data(ani, (void *)bottom, ET_NEST_OBJECT);
    ani_add_data(ani, (void *)top_obj_side,    ET_OTHER);
    ani_add_data(ani, (void *)bottom_obj_side, ET_OTHER);
    ani_add_data(ani, (void *)padding_rect,    ET_OTHER);
    ani_start(ani);

    element_set_ani(el, FILL_BETWEEN_H_ANI_NAME, ani);
}
