#ifndef GUI_ELEMENT_H
#define GUI_ELEMENT_H

#include "nest.h"
#include <SDL.h>
#include <cinttypes>

#define GUI_ELEMENT_HEAD                                                      \
    Nst_OBJ_HEAD;                                                             \
    Nst_GGC_HEAD;                                                             \
    GUI_ElementType el_type;                                                  \
    GUI_ElementState state;                                                   \
    GUI_ElementState prev_state;                                              \
    bool important;                                                           \
    SDL_Rect rect;                                                            \
    i32 pad_t, pad_b, pad_l, pad_r;                                           \
    struct _GUI_Element *parent;                                              \
    Nst_VectorObj *children;                                                  \
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
    GUI_ET_ROOT,
    GUI_ET_RAW_LABEL
} GUI_ElementType;

typedef enum _GUI_ElementState : i8 {
    GUI_ES_NONE,
    GUI_ES_ENABLED,
    GUI_ES_READONLY,
    GUI_ES_DISABLED
} GUI_ElementState;

typedef struct _GUI_Element {
    GUI_ELEMENT_HEAD;
} GUI_Element;

GUI_Element *GUI_Element_New(usize size, GUI_Element *parent,
                             struct _GUI_Window *window, struct _GUI_App *app);
void GUI_Element_Destroy(GUI_Element *element);
void GUI_Element_Traverse(GUI_Element *element);

/**
 * @brief Changes the size of an element, generates a GUI_E_RESIZE event only
 * if the size actually changes.
 */
void GUI_Element_SetSize(GUI_Element *element, int w, int h);
/**
 * @brief Fills `w` and `h` with the size of the element, `w` and `h` may be
 * `nullptr` in which case they are ignored.
 */
void GUI_Element_GetSize(GUI_Element *element, int *w, int *h);
/**
 * @brief Changes the width of an element, generates a GUI_E_RESIZE event only
 * if the width actually changes.
 */
void GUI_Element_SetWidth(GUI_Element *element, int w);
/* Returns the width of an element. */
int GUI_Element_GetWidth(GUI_Element *element);
/**
 * @brief Changes the height of an element, generates a GUI_E_RESIZE event only
 * if the height actually changes.
 */
void GUI_Element_SetHeight(GUI_Element *element, int h);
/* Returns the height of an element. */
int GUI_Element_GetHeight(GUI_Element *element);
/* Sets whether an element is an important element. */
void GUI_Element_SetImportant(GUI_Element *element, bool important);
/* Gets whether an element is an important element. */
bool GUI_Element_IsImportant(GUI_Element *element);

void GUI_Element_SetPadding(GUI_Element *element, i32 t, i32 b, i32 l, i32 r);
void GUI_Element_GetPadding(GUI_Element *element,
                            i32 *t, i32 *b, i32 *l, i32 *r);
void GUI_Element_SetPaddingTop(GUI_Element *element, i32 pad_top);
i32 GUI_Element_GetPaddingTop(GUI_Element *element);
void GUI_Element_SetPaddingBottom(GUI_Element *element, i32 pad_bottom);
i32 GUI_Element_GetPaddingBottom(GUI_Element *element);
void GUI_Element_SetPaddingLeft(GUI_Element *element, i32 pad_left);
i32 GUI_Element_GetPaddingLeft(GUI_Element *element);
void GUI_Element_SetPaddingRight(GUI_Element *element, i32 pad_left);
i32 GUI_Element_GetPaddingRight(GUI_Element *element);

/* Gets whether an element is disabled. */
bool GUI_Element_IsDisabled(GUI_Element *element);
/**
 * @brief Disables an element and its children, the previous state is saved and
 * set when the element is enabled.
 */
void GUI_Element_Disable(GUI_Element *element);
/**
 * @brief Disables an element and its children, the previous state is not taken
 * into consideration and is not saved.
 */
void GUI_Element_DisableAll(GUI_Element *element);

/* Gets whether an element is enabled in readonly mode. */
bool GUI_Element_IsReadonly(GUI_Element *element);
/**
 * @brief Enables an element and its children in readonly mode, the previous
 * state is saved and set when the element is fully enabled. If an element was
 * disabled and had a previously saved state, the previous state is restored
 * and forgotten.
 */
void GUI_Element_EnableReadonly(GUI_Element *element);
/**
 * @brief Enables an element and its children in readonly mode, the previous
 * state is not taken into consideration and is not saved.
 */
void GUI_Element_EnableReadonlyAll(GUI_Element *element);

/* Checks whether an element is fully enabled. */
bool GUI_Element_IsEnabled(GUI_Element *element);
/**
 * @brief Fully enables an element and its children. If an element was enabled
 * in readonly mode or disabled and had a previously saved state, the previous
 * state is restored and forgotten.
 */
void GUI_Element_Enable(GUI_Element *element);
/**
 * @brief Fully enables an element and its children, the previous state is not
 * taken into consideration and is not saved.
 */
void GUI_Element_EnableAll(GUI_Element *element);

/* Makes an element and its children forget their previous state. */
void GUI_Element_ForgetPrevState(GUI_Element *element);

GUI_Element *GUI_Root_New(struct _GUI_Window *window, struct _GUI_App *app);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_ELEMENT_H
