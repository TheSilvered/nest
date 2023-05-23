#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "nest.h"
#include <SDL.h>

#define GUI_ELEMENT_HEAD \
    NST_OBJ_HEAD; \
    NST_GGC_HEAD; \
    GUI_ElementType el_type; \
    SDL_Rect rect; \
    i32 margin_top, margin_bottom, margin_left, margin_right; \
    i32 padding_top, padding_bottom, padding_left, padding_right; \
    GUI_RelPos rel_pos; \
    GUI_RelSize rel_size; \
    Nst_SeqObj *children; \
    struct _GUI_Element *parent; \
    HandleEventFunc handle_event_func; \
    UpdateFunc frame_update_func; \
    UpdateFunc tick_update_func; \
    struct _GUI_App *app

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef bool (*HandleEventFunc)(SDL_Event *, Nst_OpErr *);
typedef void (*UpdateFunc)(Nst_OpErr *);

typedef enum _GUI_Flags
{
    GUI_FLAG_REL_POS  = 0b00000001,
    GUI_FLAG_REL_SIZE = 0b00000010,
}
GUI_Flags;

typedef enum _GUI_RelPosX
{
    GUI_LEFT,
    GUI_MIDDLE,
    GUI_RIGHT
}
GUI_RelPosX;

typedef enum _GUI_RelPosY
{
    GUI_TOP,
    GUI_CENTER,
    GUI_BOTTOM
}
GUI_RelPosY;

typedef enum _GUI_ElementType
{
    GUI_ET_BASE
}
GUI_ElementType;

struct _GUI_Element;

typedef struct _GUI_RelPos
{
    struct _GUI_Element *element;
    GUI_RelPosX from_x;
    GUI_RelPosY from_y;
    GUI_RelPosX to_x;
    GUI_RelPosY to_y;
}
GUI_RelPos;

typedef struct _GUI_RelSize
{
    struct _GUI_Element *element;
    i32 min_w, min_h;
    i32 max_w, max_h;
    f64 scale_x, scale_y;
    i32 diff_x, diff_y;
}
GUI_RelSize;

struct _GUI_App;

typedef struct _GUI_Element
{
    GUI_ELEMENT_HEAD;
}
GUI_Element;

extern Nst_TypeObj *gui_element_type;

Nst_Obj *gui_element_new(GUI_ElementType t,
                         usize size,
                         int x, int y,
                         int w, int h,
                         struct _GUI_App *app,
                         Nst_OpErr *err);
void gui_element_destroy(GUI_Element *obj);
void gui_element_track(GUI_Element *obj);
void gui_element_traverse(GUI_Element *obj);
void gui_element_set_margin(GUI_Element *obj,
                            i32 margin_top,
                            i32 margin_left,
                            i32 margin_bottom,
                            i32 margin_right);
void gui_element_set_padding(GUI_Element *obj,
                             i32 padding_top,
                             i32 padding_left,
                             i32 padding_bottom,
                             i32 padding_right);
void gui_element_set_parent(GUI_Element *obj, GUI_Element *parent);
int gui_element_get_content_x(GUI_Element *obj, GUI_RelPosX pos);
int gui_element_get_content_y(GUI_Element *obj, GUI_RelPosY pos);

void gui_element_set_x(GUI_Element *obj, GUI_RelPosX pos, int x);
void gui_element_set_y(GUI_Element *obj, GUI_RelPosY pos, int y);

void gui_element_set_rel_pos(GUI_Element *obj,
                             GUI_Element *element,
                             GUI_RelPosX from_x, GUI_RelPosY from_y,
                             GUI_RelPosX to_x, GUI_RelPosY to_y);

void gui_element_set_rel_size(GUI_Element *obj,
                              GUI_Element *element,
                              i32 min_w, i32 min_h,
                              i32 max_w, i32 max_h,
                              f64 scale_x, f64 scale_y,
                              i32 diff_x, i32 diff_y);

void gui_element_update_pos(GUI_Element *obj);
void gui_element_update_size(GUI_Element *obj);


#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ELEMENT_H
