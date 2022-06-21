#ifndef STR_H
#define STR_H

#include "obj.h"
#include "simple_types.h"
#include "error.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Nst_string
{
    size_t len;
    char *value;
    bool allocated;
}
Nst_string;

Nst_Obj *new_str_obj(Nst_string *str);
Nst_string *new_string_raw(const char *val, bool allocated);
Nst_string *new_string(char *val, size_t len, bool allocated);
Nst_string *copy_string(Nst_string *src);

Nst_int *parse_int(char *str, OpErr *err);
Nst_real *parse_real(char *str, OpErr *err);

void destroy_string(Nst_string *str);

#ifdef __cplusplus
}
#endif // !__cplusplus

#define AS_STR(ptr) ((Nst_string *)(ptr->value))
#define AS_STR_V(ptr) ((Nst_string *)(ptr))

#endif // !STR_H