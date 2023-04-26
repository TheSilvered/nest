/* C/C++ Library utilities */

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

#define NST_MAKE_FUNCDECLR(func_ptr, argc) \
    { \
        (void *)(func_ptr), \
        argc, \
        STR(nst_string_new_c_raw(#func_ptr, false, &err)) \
    }

#define NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name) \
    { \
        (void *)(func_ptr), \
        argc, \
        STR(nst_string_new_c_raw(func_name, false, &err)) \
    }

#define NST_MAKE_OBJDECLR(obj_ptr) \
    { \
        (void *)(obj_ptr), \
        -1, \
        STR(nst_string_new_c_raw(#obj_ptr, false, &err)) \
    }

#define NST_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name) \
    { \
        (void *)(obj_ptr), \
        -1, \
        STR(nst_string_new_c_raw(obj_name, false, &err)) \
    }

#define NST_SET_ERROR(err_name, err_msg) do { \
    if ( err != NULL ) { \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(err_msg); \
    }} while ( 0 )

#define NST_SET_RAW_ERROR(err_name, err_msg) do { \
    if ( err != NULL ) { \
        if ( NST_ERROR_OCCURRED ) { \
            nst_dec_ref(err->name); \
            if ( err->message != NULL ) \
                nst_dec_ref(err->message); \
        } \
    err->message = NULL; \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(nst_string_new_c_raw(err_msg, false, err)); \
    }} while ( 0 )

#define NST_SET_SYNTAX_ERROR(msg) NST_SET_ERROR(nst_str()->e_SyntaxError, msg)
#define NST_SET_MEMORY_ERROR(msg) NST_SET_ERROR(nst_str()->e_MemoryError, msg)
#define NST_SET_TYPE_ERROR(msg)   NST_SET_ERROR(nst_str()->e_TypeError,   msg)
#define NST_SET_VALUE_ERROR(msg)  NST_SET_ERROR(nst_str()->e_ValueError,  msg)
#define NST_SET_MATH_ERROR(msg)   NST_SET_ERROR(nst_str()->e_MathError,   msg)
#define NST_SET_CALL_ERROR(msg)   NST_SET_ERROR(nst_str()->e_CallError,   msg)
#define NST_SET_IMPORT_ERROR(msg) NST_SET_ERROR(nst_str()->e_ImportError, msg)

#define NST_SET_RAW_SYNTAX_ERROR(msg) NST_SET_RAW_ERROR(nst_s.e_SyntaxError, msg)
#define NST_SET_RAW_MEMORY_ERROR(msg) NST_SET_RAW_ERROR(nst_str()->e_MemoryError, msg)
#define NST_SET_RAW_TYPE_ERROR(msg)   NST_SET_RAW_ERROR(nst_str()->e_TypeError,   msg)
#define NST_SET_RAW_VALUE_ERROR(msg)  NST_SET_RAW_ERROR(nst_str()->e_ValueError,  msg)
#define NST_SET_RAW_MATH_ERROR(msg)   NST_SET_RAW_ERROR(nst_str()->e_MathError,   msg)
#define NST_SET_RAW_CALL_ERROR(msg)   NST_SET_RAW_ERROR(nst_str()->e_CallError,   msg)
#define NST_SET_RAW_IMPORT_ERROR(msg) NST_SET_RAW_ERROR(nst_str()->e_ImportError, msg)

#define NST_FAILED_ALLOCATION \
    NST_SET_ERROR(nst_str()->e_MemoryError, nst_inc_ref(nst_str()->o_failed_alloc))

#define NST_RETURN_TRUE return nst_inc_ref(nst_true())
#define NST_RETURN_FALSE return nst_inc_ref(nst_false())
#define NST_RETURN_NULL return nst_inc_ref(nst_null())
#define NST_RETURN_ZERO return nst_inc_ref(nst_const()->Int_0)
#define NST_RETURN_ONE return nst_inc_ref(nst_const()->Int_1)
#define NST_RETURN_COND(cond) \
    return (cond) ? nst_inc_ref(nst_true()) : nst_inc_ref(nst_false())

// Function signature for Nest function
#define NST_FUNC_SIGN(name) \
    Nst_Obj *name(usize arg_num, Nst_Obj **args, Nst_OpErr *err)

// Default call to nst_extract_arg_values
#define NST_DEF_EXTRACT(ltrl, ...) \
    if ( !nst_extract_arg_values(ltrl, arg_num, args, err, __VA_ARGS__) ) \
        return NULL

// Sets 'def_val' if 'obj' is null else 'val'
#define NST_DEF_VAL(obj, val, def_val) \
    ((obj) == nst_null() ? (def_val) : (val))

#define NST_ERROR_OCCURRED (err != NULL && err->name != NULL)

#define NST_RETURN_NEW_STR(val, len) do { \
    Nst_Obj *_s_ = nst_string_new(val, len, true, err); \
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
                                   Nst_OpErr *err,
                                   ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
