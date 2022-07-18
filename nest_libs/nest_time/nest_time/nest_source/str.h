#ifndef STR_H
#define STR_H

#include <string.h>
#include "obj.h"
#include "simple_types.h"
#include "error.h"

#define AS_STR(ptr) ((Nst_StrObj *)(ptr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    OBJ_HEAD;
    size_t len;
    char *value;
    bool allocated;
}
Nst_StrObj;

Nst_Obj *new_string_raw(const char *val, bool allocated);
Nst_Obj *new_string(char *val, size_t len, bool allocated);

Nst_Obj *new_type_obj(const char *val, size_t len);

Nst_Obj *copy_string(Nst_StrObj *src);
Nst_Obj *repr_string(Nst_StrObj *src);

Nst_Obj *parse_int(char *str, OpErr *err);
Nst_Obj *parse_real(char *str, OpErr *err);
void destroy_string(Nst_StrObj *str);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H