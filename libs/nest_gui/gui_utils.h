#ifndef GUI_UTILS_H
#define GUI_UTILS_H

#include "nest.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

bool GUI_init_utils(void);
void GUI_quit_utils(void);

void GUI_throw_sdl_error(void);

int max_int(int a, int b);
int min_int(int a, int b);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !GUI_UTILS_H
