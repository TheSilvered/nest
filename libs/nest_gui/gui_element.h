#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "nest.h"
#include <SDL.h>

#define GUI_ELEMENT_HEAD                                                      \
    Nst_OBJ_HEAD;                                                             \
    Nst_GGC_HEAD;                                                             \
    GUI_ElementType el_type;                                                  \
    SDL_Rect rect;                                                            \
    i32 pad_u, pad_d, pad_l, pad_r;                                           \
    struct _GUI_Element *parent;                                                  \
    Nst_VectorObj *children;                                                   \
    struct _GUI_Window *window;                                               \
    struct _GUI_App *app;                                                     \
    Nst_MapObj *constraints_before;                                           \
    Nst_MapObj *constraints_after;                                            \
    GUI_EventHandler event_handler;                                           \
    GUI_UpdateFunc frame_update;                                              \
    GUI_UpdateFunc tick_update;                                               \
    GUI_ElementDestructor el_destructor

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _GUI_Element;
struct _GUI_Window;
struct _GUI_App;

typedef i32 (*GUI_EventHandler)(struct _GUI_Element *, SDL_Event *);
typedef bool (*GUI_UpdateFunc)(struct _GUI_Element *);
typedef void (*GUI_ElementDestructor)(struct _GUI_Element *);

typedef enum _GUI_ElementType {
    GUI_ET_NONE,
    GUI_ET_ROOT
} GUI_ElementType;

typedef struct _GUI_Element {
    GUI_ELEMENT_HEAD;
} GUI_Element;

extern Nst_TypeObj *element_type;

GUI_Element *GUI_Element_New(usize size, GUI_Element *parent,
                             struct _GUI_Window *window, struct _GUI_App *app);
void GUI_Element_Destroy(GUI_Element *element);
void GUI_Element_Traverse(GUI_Element *element);

GUI_Element *GUI_Root_New(struct _GUI_Window *window, struct _GUI_App *app);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ELEMENT_H
