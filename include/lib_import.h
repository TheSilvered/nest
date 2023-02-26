/* C/C++ Library utilities */

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

#define NST_MAKE_FUNCDECLR(func_ptr, argc) \
    { \
        func_ptr, \
        argc, \
        STR(nst_string_new_c_raw(#func_ptr, false)) \
    }

#define NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, name) \
    { \
        func_ptr, \
        argc, \
        STR(nst_string_new_c_raw(name, false)) \
    }

#define NST_SET_ERROR(err_name, err_msg) do { \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(nst_inc_ref(err_msg)); \
    } while ( 0 )

#define NST_SET_RAW_ERROR(err_name, err_msg) do { \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(nst_string_new_c_raw(err_msg, false)); \
    } while ( 0 )

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
    NST_SET_ERROR(nst_str()->e_MemoryError, nst_str()->o_failed_alloc)

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

// Sets 'var' to 'def_val' if obj is null and to 'val' otherwise
#define NST_SET_DEF(obj, var, def_val, val) \
    do { \
    if ( (obj) == nst_null() ) \
       var = (def_val); \
    else \
        var = (val); \
    } while ( 0 )

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_FuncDeclr
{
    NST_FUNC_SIGN((*func_ptr));
    usize arg_num;
    Nst_StrObj *name;
}
Nst_FuncDeclr;

// Allocates the function list of the module
EXPORT Nst_FuncDeclr *nst_func_list_new(usize count);
// Extracts the C values from the arguments
// `types` is a string of letters for the types, check the full usage in
// src/lib_import.c
// `arg_num`: number of arguments the function expects
// `args`: the arguments themselves
// `err`: the `err` argument of the function
// ...: the pointers to store the values in or the custom types
EXPORT
bool nst_extract_arg_values(const i8  *types,
                            usize      arg_num,
                            Nst_Obj  **args,
                            Nst_OpErr *err,
                            ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H