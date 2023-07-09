/* Nst_StrObj interface, implements Str and Type */

#ifndef STR_H
#define STR_H

#include "obj.h"

#define STR(ptr) ((Nst_StrObj *)(ptr))
#define TYPE(ptr) ((Nst_TypeObj *)(ptr))
#define TYPE_NAME(obj) (STR(obj->type)->value)
#define Nst_STR_IS_ALLOC(str) ((str)->flags & Nst_FLAG_STR_IS_ALLOC)

// Creates a copy of the string
#define Nst_string_copy(src) _Nst_string_copy(STR(src))
// Creates new string that is the representation of `src`
#define Nst_string_repr(src) _Nst_string_repr(STR(src))
// Creates a one-character string with the character at `idx` of str
#define Nst_string_get(str, idx) _Nst_string_get(STR(str), idx)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_StrObj
{
    Nst_OBJ_HEAD;
    usize len;
    i8 *value;
}
Nst_StrObj;

NstEXP typedef Nst_StrObj Nst_TypeObj;

// Creates a new string from a C string of unknown lenght
NstEXP Nst_Obj *Nst_string_new_c_raw(const i8 *val, bool allocated);
// Creates a string from a string literal
NstEXP Nst_Obj *Nst_string_new_c(const i8 *val, usize len, bool allocated);
// Creates a new string from a i8 * of known lenght
NstEXP Nst_Obj *Nst_string_new(i8 *val, usize len, bool allocated);

// Creates a new Type object
NstEXP Nst_TypeObj *Nst_type_new(const i8 *val, usize len);

NstEXP Nst_Obj *_Nst_string_copy(Nst_StrObj *src);
NstEXP Nst_Obj *_Nst_string_repr(Nst_StrObj *src);
NstEXP Nst_Obj *_Nst_string_get(Nst_StrObj *str, i64 idx);

// Parses a Nst_IntObj from a string, any NUL character in the middle does not
// intefere with the parsing
NstEXP Nst_Obj *Nst_string_parse_int(Nst_StrObj *str, i32 base);
// Parses a Nst_ByteObj from a string, any NUL character in the middle does not
// intefere with the parsing
NstEXP Nst_Obj *Nst_string_parse_byte(Nst_StrObj* str);
// Parses a Nst_RealObj from a string, any NUL character in the middle does not
// intefere with the parsing
NstEXP Nst_Obj *Nst_string_parse_real(Nst_StrObj *str);
// The same as strcmp but uses the string's length instead of the NUL
// byte for the end.
// Return value:
//     0: the strings are equal
//   > 0: str1 is greater than str2
//   < 0: str2 is greater than str1
NstEXP i32 Nst_string_compare(Nst_StrObj *str1, Nst_StrObj *str2);
NstEXP void _Nst_string_destroy(Nst_StrObj *str);

// Returns the pointer of the start of the first occurrence of s2 in s1,
// NULL if the substring cannot be found
NstEXP i8 *Nst_string_find(i8 *s1, usize l1, i8 *s2, usize l2);

NstEXP typedef enum _Nst_StrFlags
{
    Nst_FLAG_STR_IS_ALLOC = 0b1
}
Nst_StrFlags;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !STR_H
