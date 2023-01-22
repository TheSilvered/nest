/* Nst_StrObj interface, implements Str and Type */

#ifndef STR_H
#define STR_H

#include <string.h>
#include "simple_types.h"

#define STR(ptr) ((Nst_StrObj *)(ptr))
#define TYPE(ptr) ((Nst_TypeObj *)(ptr))
#define TYPE_NAME(obj) (STR(obj->type)->value)
#define NST_STR_IS_ALLOC(str) ((str)->flags & NST_FLAG_STR_IS_ALLOC)

// Creates a copy of the string
#define nst_copy_string(src) _nst_copy_string(STR(src))
// Creates new string that is the representation of `src`
#define nst_repr_string(src) _nst_repr_string(STR(src))
// Creates a one-character string with the character at `idx` of idx
#define nst_string_get_idx(str, idx) _nst_string_get_idx(STR(str), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_OpErr;

typedef struct _Nst_StrObj
{
    NST_OBJ_HEAD;
    size_t len;
    char *value;
}
Nst_StrObj;

typedef Nst_StrObj Nst_TypeObj;

// Creates a new string from a C string of unknown lenght
Nst_Obj *nst_new_cstring_raw(const char *val, bool allocated);
// Creates a string from a string literal
Nst_Obj *nst_new_cstring(const char *val, size_t len, bool allocated);
// Creates a new string from a char * of known lenght
Nst_Obj *nst_new_string(char *val, size_t len, bool allocated);

// Creates a new Type object
Nst_TypeObj *nst_new_type_obj(const char *val, size_t len);

Nst_Obj *_nst_copy_string(Nst_StrObj *src);
Nst_Obj *_nst_repr_string(Nst_StrObj *src);
Nst_Obj *_nst_string_get_idx(Nst_StrObj *str, Nst_Int idx);

// Parses a Nst_IntObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj *nst_parse_int(Nst_StrObj *str, struct _Nst_OpErr *err);
// Parses a Nst_ByteObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj *nst_parse_byte(Nst_StrObj* str, struct _Nst_OpErr* err);
// Parses a Nst_RealObj from a string, any NUL characters in the middle
// do not intefere with the parsing
Nst_Obj *nst_parse_real(Nst_StrObj *str, struct _Nst_OpErr *err);
// The same as strcmp but uses the string's length instead of the NUL
// byte for the end.
// Return value:
//     0: the strings are equal
//   > 0: str1 is greater than str2
//   < 0: str2 is greater than str1
int nst_compare_strings(Nst_StrObj *str1, Nst_StrObj *str2);
void nst_destroy_string(Nst_StrObj *str);

enum Nst_StrFlags {
    NST_FLAG_STR_IS_ALLOC = 0b1
};

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H