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

Nst_Obj *nst_new_string_raw(const char *val, bool allocated);
Nst_Obj *nst_new_string(char *val, size_t len, bool allocated);

Nst_Obj *nst_new_type_obj(const char *val, size_t len);

Nst_Obj *nst_copy_string(Nst_StrObj *src);
Nst_Obj *nst_repr_string(Nst_StrObj *src);

Nst_Obj *nst_parse_int(char *str, Nst_OpErr *err);
Nst_Obj *nst_parse_real(char *str, Nst_OpErr *err);
void nst_destroy_string(Nst_StrObj *str);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H