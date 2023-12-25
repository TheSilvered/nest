#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "nest.h"

#ifdef Nst_WIN
#pragma warning(push)
#pragma warning(disable: 4995)
#endif

#include <SDL.h>
#include <SDL_ttf.h>
#include <vector>

#ifdef Nst_WIN
#pragma warning(pop)
#endif

#define GUI_ELEMENT_HEAD                                                      \
    Nst_OBJ_HEAD;                                                             \
    Nst_GGC_HEAD;                                                             \
    ElementType el_type;                                                      \
    SDL_Rect rect;                                                            \
    i32 padding_top, padding_left, padding_bottom, padding_right;             \
    Nst_VectorObj *children;                                                  \
    struct _Element *parent;                                                  \
    void (*el_destructor)(void *);                                            \
    HandleEventFunc handle_event_func;                                        \
    UpdateFunc frame_update_func;                                             \
    UpdateFunc tick_update_func;                                              \
    bool clip_content;                                                        \
    struct _App *app;                                                         \
    Nst_MapObj *animations

#define IS_HIDDEN(element) Nst_HAS_FLAG(element, GUI_FLAG_IS_HIDDEN)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

namespace GUI {

struct _Element;

typedef i32 (*HandleEventFunc)(SDL_Event *, struct _Element *);
typedef bool (*UpdateFunc)(struct _Element *);
typedef bool (*OnChildAdded)(struct _Element *, usize);

typedef enum _Flags {
    GUI_FLAG_IS_HIDDEN = Nst_FLAG(3)
} Flags;

typedef enum _Side {
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    CENTER,

    // padding rect sides

    P_LEFT,
    P_RIGHT,
    P_TOP,
    P_BOTTOM,
    P_CENTER
} Side;

typedef enum _Pos {
    TL, TC, TR,
    CL, CC, CR,
    BL, BC, BR,

    // padding rect positions

    P_TL, P_TC, P_TR,
    P_CL, P_CC, P_CR,
    P_BL, P_BC, P_BR
} Pos;

typedef enum _ElementType {
    GUI_ET_BASE,
    GUI_ET_LABEL,
    GUI_ET_STACK_LAYOUT,
    GUI_ET_BUTTON
} ElementType;

typedef enum _UserEventCode {
    GUI_UE_CHILD_ADDED,
    GUI_UE_RESIZED,
    GUI_UE_MOVED,
    GUI_UE_CHANGED_PADDING
} UserEventCode;

struct _Element;
struct _App;

typedef struct _Element {
    GUI_ELEMENT_HEAD;
} Element;

extern Nst_TypeObj *gui_element_type;

bool element_init();
void element_quit();

Element *element_new(ElementType t, usize size, int x, int y, int w, int h,
                     struct _App *app, void (*el_destructor)(void *));
Nst_Obj *get_element_type();

void element_destroy(Element *obj);
void element_traverse(Element *obj);
void element_set_padding(Element *obj, i32 padding_top, i32 padding_left,
                         i32 padding_bottom, i32 padding_right);
void element_get_padding(Element *obj, i32 &padding_top, i32 &padding_left,
                         i32 &padding_bottom, i32 &padding_right);

void element_set_parent(Element *obj, Element *parent);

int element_get_x(Element *obj, Side side);
int element_get_y(Element *obj, Side side);
void element_get_pos(Element *obj, Pos pos, int &x, int &y);
int element_get_w(Element *obj, bool from_padding);
int element_get_h(Element *obj, bool from_padding);
void element_get_size(Element *obj, int &w, int &h, bool from_padding);

void pos_to_sides(Pos pos, Side &x_side, Side &y_side);
Pos sides_to_pos(Side x_side, Side y_side);

void element_set_x(Element *obj, Side side, int x);
void element_set_y(Element *obj, Side side, int y);
void element_set_pos(Element *obj, Pos pos, int x, int y);
void element_set_w(Element *obj, int w, bool to_padding);
void element_set_h(Element *obj, int h, bool to_padding);
void element_set_size(Element *obj, int w, int h, bool to_padding);

const i8 *element_get_type_name(Element *obj);

struct _AniObj;

bool element_set_ani(Element *obj, const i8 *name, struct _AniObj *ani);
struct _AniObj *element_get_ani(Element *obj, const i8 *name);

SDL_Rect element_get_padding_rect(Element *obj);
SDL_Rect element_get_clip_rect(Element *obj);

bool element_add_child(Element *parent, Element *child);
bool element_remove_child(Element *parent, Element *child);

void element_set_clip(Element *element, bool clip);

typedef enum _FontWeight {
    GUI_FW_LIGHT,
    GUI_FW_REGULAR,
    GUI_FW_BOLD,
    GUI_FW_EXTRA_BOLD
} FontWeight;

typedef enum _FontSize {
    GUI_FS_SMALL,
    GUI_FS_MEDIUM,
    GUI_FS_LARGE,
} FontSize;

typedef struct _FontObj {
    Nst_OBJ_HEAD;
    TTF_Font *font;
} FontObj;

FontObj *get_font(struct _App *app, FontWeight weight, FontSize size,
                  bool italic, bool monospace);

Nst_Obj *font_new(TTF_Font *font);
Nst_Obj *get_font_type();
void font_destroy(FontObj *f);
}

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ELEMENT_H
