#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "nest.h"
#include <SDL.h>

#define GUI_ELEMENT_HEAD                                                      \
    Nst_OBJ_HEAD;                                                             \
    Nst_GGC_HEAD;                                                             \
    ElementType el_type;                                                      \
    SDL_Rect rect;                                                            \
    i32 pad_u, pad_d, pad_l, pad_r;                                           \
    struct _Element *parent;                                                  \
    Nst_ArrayObj *children;                                                   \
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

typedef enum _ElementType {
    GUI_ET_ROOT
} ElementType;

typedef struct _GUI_Element {
    GUI_ELEMENT_HEAD;
} GUI_Element;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ELEMENT_H
