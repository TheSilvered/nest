#ifndef GUI_OBJ_TYPES_H
#define GUI_OBJ_TYPES_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

extern Nst_TypeObj *element_type;
extern Nst_TypeObj *constr_type;

bool GUI_InitTypes();
void GUI_QuitTypes();

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_OBJ_TYPES_H
