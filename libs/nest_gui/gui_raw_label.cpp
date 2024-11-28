#include <cstring>
#include "gui_raw_label.h"

static void raw_label_destructor(GUI_RawLabel *label);

GUI_RawLabel *GUI_RawLabel_New(GUI_Element *parent, i8 *text, isize text_len)
{
    GUI_RawLabel *label = (GUI_RawLabel *)GUI_Element_New(
        sizeof(GUI_RawLabel),
        parent,
        parent->window,
        parent->app);
    if (label == nullptr)
        return nullptr;
    if (text_len < 0)
        label->text_len = strlen(text);
    else
        label->text_len = text_len;
    label->text = (i8 *)Nst_calloc(1, label->text_len + 1, text);
    if (label->text == nullptr) {
        Nst_dec_ref(label);
        return nullptr;
    }

    label->el_type = GUI_ET_RAW_LABEL;
    label->el_destructor = (GUI_ElementDestructor)raw_label_destructor;
    return label;
}

Nst_StrObj *GUI_RawLabel_GetTextObj(GUI_RawLabel *label)
{
    Nst_assert(label->el_type == GUI_ET_RAW_LABEL);

    i8 *value = (i8 *)Nst_calloc(1, label->text_len + 1, label->text);
    if (value == nullptr)
        return nullptr;
    return STR(Nst_str_new_allocated(value, label->text_len));
}

bool GUI_RawLabel_SetTextObj(GUI_RawLabel *label, Nst_StrObj *text)
{
    i8 *new_text = (i8 *)Nst_calloc(1, text->len + 1, text->value);
    if (new_text == nullptr)
        return false;
    Nst_free(label->text);
    label->text = new_text;
    label->text_len = text->len;
    return true;
}

static void raw_label_destructor(GUI_RawLabel *label)
{
    Nst_assert(label->el_type == GUI_ET_RAW_LABEL);

    Nst_free(label->text);
}

/*
static bool raw_label_update(GUI_RawLabel *label)
{
    Nst_assert(label->el_type == GUI_ET_RAW_LABEL);


}
*/
