/* C/C++ Library utilities */

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

#define NST_MAKE_FUNCDECLR(func_ptr, argc) \
    { \
        (void *)(func_ptr), \
        argc, \
        STR(nst_string_new_c_raw(#func_ptr, false)) \
    }

#define NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name) \
    { \
        (void *)(func_ptr), \
        argc, \
        STR(nst_string_new_c_raw(func_name, false)) \
    }

#define NST_MAKE_OBJDECLR(obj_ptr) \
    { \
        (void *)(obj_ptr), \
        -1, \
        STR(nst_string_new_c_raw(#obj_ptr, false)) \
    }

#define NST_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name) \
    { \
        (void *)(obj_ptr), \
        -1, \
        STR(nst_string_new_c_raw(obj_name, false)) \
    }

#define NST_RETURN_TRUE return nst_inc_ref(nst_true())
#define NST_RETURN_FALSE return nst_inc_ref(nst_false())
#define NST_RETURN_NULL return nst_inc_ref(nst_null())
#define NST_RETURN_ZERO return nst_inc_ref(nst_const()->Int_0)
#define NST_RETURN_ONE return nst_inc_ref(nst_const()->Int_1)
#define NST_RETURN_COND(cond) \
    return (cond) ? nst_inc_ref(nst_true()) : nst_inc_ref(nst_false())

// Function signature for Nest function
#define NST_FUNC_SIGN(name) \
    Nst_Obj *name(usize arg_num, Nst_Obj **args)

// Default call to nst_extract_arg_values
#define NST_DEF_EXTRACT(ltrl, ...) \
    if ( !nst_extract_arg_values(ltrl, arg_num, args, __VA_ARGS__) ) \
        return NULL

// Sets 'def_val' if 'obj' is null else 'val'
#define NST_DEF_VAL(obj, val, def_val) \
    ((obj) == nst_null() ? (def_val) : (val))

#define NST_RETURN_NEW_STR(val, len) do { \
    Nst_Obj *_s_ = nst_string_new(val, len, true); \
    if ( _s_ == NULL ) nst_free(val); \
    return _s_; \
    } while (0)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_ObjDeclr
{
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
}
Nst_ObjDeclr;

EXPORT typedef struct _Nst_DeclrList
{
    Nst_ObjDeclr *objs;
    usize obj_count;
}
Nst_DeclrList;

// Extracts the C values from the arguments
// `types` is a string of letters for the types, check the full usage in
// src/lib_import.c
// `arg_num`: number of arguments the function expects
// `args`: the arguments themselves
// `err`: the `err` argument of the function
// ...: the pointers to store the values in or the custom types
EXPORT bool nst_extract_arg_values(const i8  *types,
                                   usize      arg_num,
                                   Nst_Obj  **args,
                                   ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
