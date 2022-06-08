#ifndef STR_H
#define STR_H

#include "obj.h"

typedef struct Nst_string
{
    size_t len;
    char *value;
    bool allocated;
}
Nst_string;

Nst_Obj *new_str_obj(Nst_string *str);
Nst_string *new_string_raw(char *val, bool allocated);
Nst_string *new_string(char *val, size_t len, bool allocated);
Nst_string *copy_string(Nst_string *src);

void destroy_string(Nst_string *str);
#define AS_STR(ptr) ((Nst_string *)(ptr->value))
#define AS_STR_V(ptr) ((Nst_string *)(ptr))

#endif // !STR_H