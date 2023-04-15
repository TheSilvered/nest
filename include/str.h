/* Nst_StrObj interface, implements Str and Type */

#ifndef STR_H
#define STR_H

#include "obj.h"

#define STR(ptr) ((Nst_StrObj *)(ptr))
#define TYPE(ptr) ((Nst_TypeObj *)(ptr))
#define TYPE_NAME(obj) (STR(obj->type)->value)
#define NST_STR_IS_ALLOC(str) ((str)->flags & NST_FLAG_STR_IS_ALLOC)

// Creates a copy of the string
#define nst_string_copy(src, err) _nst_string_copy(STR(src), err)
// Creates new string that is the representation of `src`
#define nst_string_repr(src, err) _nst_string_repr(STR(src), err)
// Creates a one-character string with the character at `idx` of idx
#define nst_string_get(str, idx, err) _nst_string_get(STR(str), idx, err)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

struct _Nst_OpErr;

EXPORT typedef struct _Nst_StrObj
{
    NST_OBJ_HEAD;
    usize len;
    i8 *value;
}
Nst_StrObj;

EXPORT typedef Nst_StrObj Nst_TypeObj;

// Creates a new string from a C string of unknown lenght
EXPORT Nst_Obj *nst_string_new_c_raw(const i8 *val, bool allocated, struct _Nst_OpErr *err);
// Creates a string from a string literal
EXPORT Nst_Obj *nst_string_new_c(const i8 *val, usize len, bool allocated, struct _Nst_OpErr *err);
// Creates a new string from a i8 * of known lenght
EXPORT Nst_Obj *nst_string_new(i8 *val, usize len, bool allocated, struct _Nst_OpErr *err);

// Creates a new Type object
EXPORT Nst_TypeObj *nst_type_new(const i8 *val, usize len, struct _Nst_OpErr *err);

EXPORT Nst_Obj *_nst_string_copy(Nst_StrObj *src, struct _Nst_OpErr *err);
EXPORT Nst_Obj *_nst_string_repr(Nst_StrObj *src, struct _Nst_OpErr *err);
EXPORT Nst_Obj *_nst_string_get(Nst_StrObj *str, i64 idx, struct _Nst_OpErr *err);

// Parses a Nst_IntObj from a string, any NUL character in the middle does not
// intefere with the parsing
EXPORT Nst_Obj *nst_string_parse_int(Nst_StrObj *str, i32 base, struct _Nst_OpErr *err);
// Parses a Nst_ByteObj from a string, any NUL character in the middle does not
// intefere with the parsing
EXPORT Nst_Obj *nst_string_parse_byte(Nst_StrObj* str, struct _Nst_OpErr* err);
// Parses a Nst_RealObj from a string, any NUL character in the middle does not
// intefere with the parsing
EXPORT Nst_Obj *nst_string_parse_real(Nst_StrObj *str, struct _Nst_OpErr *err);
// The same as strcmp but uses the string's length instead of the NUL
// byte for the end.
// Return value:
//     0: the strings are equal
//   > 0: str1 is greater than str2
//   < 0: str2 is greater than str1
EXPORT i32 nst_string_compare(Nst_StrObj *str1, Nst_StrObj *str2);
EXPORT void _nst_string_destroy(Nst_StrObj *str);

// Returns the pointer of the start of the first occurrence of s2 in s1,
// NULL if the substring cannot be found
EXPORT i8 *nst_string_find(i8 *s1, usize l1, i8 *s2, usize l2);

EXPORT typedef enum _Nst_StrFlags
{
    NST_FLAG_STR_IS_ALLOC = 0b1
}
Nst_StrFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H
