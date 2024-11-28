#ifndef GUI_RAW_LABEL_H
#define GUI_RAW_LABEL_H

#include "gui_element.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct _GUI_RawLabel {
    GUI_ELEMENT_HEAD;
    i8 *text;
    usize text_len;
} GUI_RawLabel;

/* Creates a new RawLabel element, if `text_len` is `-1` strlen is used. */
GUI_RawLabel *GUI_RawLabel_New(GUI_Element *parent, i8 *text, isize text_len);

Nst_StrObj *GUI_RawLabel_GetTextObj(GUI_RawLabel *label);
bool GUI_RawLabel_SetTextObj(GUI_RawLabel *label, Nst_StrObj *text);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // !GUI_RAW_LABEL_H
