/* Nst_StrObj interface, implements Str and Type */

#ifndef STR_H
#define STR_H

#include <string.h>
#include "obj.h"
#include "simple_types.h"
#include "error.h"

#define AS_STR(ptr) ((Nst_StrObj *)(ptr))
#define TYPE_NAME(obj) (AS_STR(obj->type)->value)
#define NST_STR_IS_ALLOC(str) ((str)->flags & NST_FLAG_STR_IS_ALLOC)

#define nst_copy_string(src) _nst_copy_string(AS_STR(src))
#define nst_repr_string(src) _nst_repr_string(AS_STR(src))
#define nst_string_get_idx(str, idx) _nst_string_get_idx(AS_STR(str), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    NST_OBJ_HEAD;
    size_t len;
    char *value;
}
Nst_StrObj;

// Creates a new string from a C string of unknown lenght
Nst_Obj *nst_new_string_raw(const char *val, bool allocated);
// Creates a new string from a char * of known lenght
Nst_Obj *nst_new_string(char *val, size_t len, bool allocated);

// Creates a new Type object
Nst_Obj *nst_new_type_obj(const char *val, size_t len);

// Creates a copy of the string
Nst_Obj *_nst_copy_string(Nst_StrObj *src);
// Creates new string that is the representation of `src`
Nst_Obj *_nst_repr_string(Nst_StrObj *src);
// Creates a one-character string with the character at `idx` of idx
Nst_Obj *_nst_string_get_idx(Nst_StrObj *str, Nst_Int idx);

// Parses a Nst_IntObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj *nst_parse_int(Nst_StrObj *str, Nst_OpErr *err);
// Parses a Nst_ByteObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj* nst_parse_byte(Nst_StrObj* str, Nst_OpErr* err);
// Parses a Nst_RealObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj *nst_parse_real(Nst_StrObj *str, Nst_OpErr *err);
void nst_destroy_string(Nst_StrObj *str);

enum Nst_StrFlags {
    NST_FLAG_STR_IS_ALLOC = 0b1
};

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H