#ifndef GUI_ANIMATION_H
#define GUI_ANIMATION_H

#include "nest.h"
#include "gui_element.h"
#include <vector>
#include <chrono>

#define MATCH_X_ANI_NAME        "stdgui::match_x"
#define MATCH_Y_ANI_NAME        "stdgui::match_y"
#define MATCH_POS_ANI_NAME      "stdgui::match_pos"
#define MATCH_W_ANI_NAME        "stdgui::match_w"
#define MATCH_H_ANI_NAME        "stdgui::match_h"
#define MATCH_SIZE_ANI_NAME     "stdgui::match_size"
#define PERC_POS_ANI_NAME       "stdgui::perc_pos"
#define DIFF_POS_ANI_NAME       "stdgui::diff_pos"
#define PERC_SIZE_ANI_NAME      "stdgui::perc_size"
#define DIFF_SIZE_ANI_NAME      "stdgui::diff_size"
#define MIN_X_ANI_NAME          "stdgui::min_x"
#define MIN_Y_ANI_NAME          "stdgui::min_y"
#define MIN_POS_ANI_NAME        "stdgui::min_pos"
#define MAX_X_ANI_NAME          "stdgui::max_x"
#define MAX_Y_ANI_NAME          "stdgui::max_y"
#define MAX_POS_ANI_NAME        "stdgui::max_pos"
#define MIN_W_ANI_NAME          "stdgui::min_w"
#define MIN_H_ANI_NAME          "stdgui::min_h"
#define MIN_SIZE_ANI_NAME       "stdgui::min_size"
#define MAX_W_ANI_NAME          "stdgui::max_w"
#define MAX_H_ANI_NAME          "stdgui::max_h"
#define MAX_SIZE_ANI_NAME       "stdgui::max_size"
#define FILL_BETWEEN_W_ANI_NAME "stdgui::fill_between_w"
#define FILL_BETWEEN_H_ANI_NAME "stdgui::fill_between_h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#ifdef _Nst_ARCH_x64
typedef f64 fsize;
#else
typedef f32 fsize;
#endif

namespace GUI {

typedef enum _EntryType {
    ET_NEST_OBJECT,
    ET_HEAP_OBJECT,
    ET_OTHER
} EntryType;

struct AniDataEntry {
    EntryType data_type;
    void *data;
};

void entry_init(AniDataEntry &entry, EntryType type, void *data_ptr);
void entry_free_data(AniDataEntry &entry);
void entry_track_data(AniDataEntry &entry);

Element *entry_get_element(AniDataEntry &entry);
fsize entry_get_fsize(AniDataEntry &entry);
int entry_get_int(AniDataEntry &entry);
i32 entry_get_i32(AniDataEntry &entry);
u32 entry_get_u32(AniDataEntry &entry);
usize entry_get_usize(AniDataEntry &entry);
isize entry_get_isize(AniDataEntry &entry);
Side entry_get_side(AniDataEntry &entry);
Pos entry_get_pos(AniDataEntry &entry);
bool entry_get_bool(AniDataEntry &entry);
void *entry_raw(AniDataEntry &entry);

struct _AniObj;
typedef void (*AniUpdateFunc)(struct _AniObj *, double);

typedef struct _AniObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_SizedBuffer data;
    std::chrono::duration<i32, std::milli> duration;
    std::chrono::high_resolution_clock::time_point start_time;
    bool loop = false;
    bool running = false;
    AniUpdateFunc update_func;
} AniObj;

bool ani_init();
void ani_quit();

Nst_Obj *ani_new(AniUpdateFunc update_func, double duration, bool loop);
Nst_Obj *get_ani_type();

void ani_add_data(AniObj *ani, void *value, EntryType type);
AniDataEntry &ani_get_data(AniObj *ani, usize index);
void ani_set_data(AniObj *ani, usize index, void *value, EntryType type,
                  bool free_prev);

void ani_destroy(AniObj *ani);
void ani_traverse(AniObj *ani);

void ani_start(AniObj *ani);
void ani_stop(AniObj *ani);
bool ani_is_running(AniObj *ani);
void ani_update(AniObj *ani);

void match_x(Element *from_el, Element *to_el, Side from, Side to);
void match_y(Element *from_el, Element *to_el, Side from, Side to);
void match_pos(Element *from_el, Element *to_el, Pos from_pos, Pos to_pos);
void match_w(Element *from_el, Element *to_el, bool from_p, bool to_p);
void match_h(Element *from_el, Element *to_el, bool from_p, bool to_p);
void match_size(Element *from_el, Element *to_el, bool from_p, bool to_p);

void perc_pos(Element *el, fsize x, fsize y, Pos pos);
void diff_pos(Element *el, int x, int y);
void perc_size(Element *el, fsize w, fsize h, bool padding_rect);
void diff_size(Element *el, int w, int h);

void min_x(Element *el, int min_x, Side side);
void min_y(Element *el, int min_y, Side side);
void min_pos(Element *el, int min_x, int min_y, Pos pos);
void max_x(Element *el, int max_x, Side side);
void max_y(Element *el, int max_y, Side side);
void max_pos(Element *el, int max_x, int max_y, Pos pos);
void min_w(Element *el, int min_w, bool padding_rect);
void min_h(Element *el, int min_h, bool padding_rect);
void min_size(Element *el, int min_w, int min_h, bool padding_rect);
void max_w(Element *el, int max_w, bool padding_rect);
void max_h(Element *el, int max_h, bool padding_rect);
void max_size(Element *el, int max_w, int max_h, bool padding_rect);

void fill_w(Element *el, Element *left, Element *right,
                    Side left_obj_side, Side right_obj_side,
                    bool padding_rect);
void fill_h(Element *el, Element *top, Element *bottom,
                    Side top_obj_side, Side bottom_obj_side,
                    bool padding_rect);
}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ANIMATION_H
