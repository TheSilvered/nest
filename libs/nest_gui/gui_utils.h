#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool GUI_InitUtils(void);
void GUI_QuitUtils(void);

void GUI_ThrowSDLError(void);

int GUI_MaxInt2(int a, int b);
int GUI_MinInt2(int a, int b);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_UTILS_H
