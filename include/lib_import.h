/* C/C++ Library utilities */

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "error.h"
#include "interpreter.h"
#include "global_consts.h"

#define NST_INIT_LIB_OBJ_FUNC \
    void init_lib_obj(Nst_TypeObjs main_t, Nst_StrConsts main_s, \
                      Nst_Consts main_c, Nst_StdStreams *main_io, Nst_ExecutionState main_state) \
    { \
        nst_t = main_t; \
        nst_s = main_s; \
        nst_c = main_c; \
        nst_io = main_io; \
        nst_state = main_state; \
    }

#define NST_MAKE_FUNCDECLR(func_ptr, argc) \
    { \
        func_ptr, \
        argc, \
        STR(nst_new_cstring_raw(#func_ptr, false)) \
    }

#define NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, name) \
    { \
        func_ptr, \
        argc, \
        STR(nst_new_cstring_raw(name, false)) \
    }

#define NST_SET_ERROR(err_name, err_msg) do { \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(nst_inc_ref(err_msg)); \
    } while ( 0 )

#define NST_SET_RAW_ERROR(err_name, err_msg) do { \
    err->name = STR(nst_inc_ref(err_name)); \
    err->message = STR(nst_new_cstring_raw(err_msg, false)); \
    } while ( 0 )

#define NST_SET_SYNTAX_ERROR(msg) NST_SET_ERROR(nst_s.e_SyntaxError, msg)
#define NST_SET_MEMORY_ERROR(msg) NST_SET_ERROR(nst_s.e_MemoryError, msg)
#define NST_SET_TYPE_ERROR(msg)   NST_SET_ERROR(nst_s.e_TypeError,   msg)
#define NST_SET_VALUE_ERROR(msg)  NST_SET_ERROR(nst_s.e_ValueError,  msg)
#define NST_SET_MATH_ERROR(msg)   NST_SET_ERROR(nst_s.e_MathError,   msg)
#define NST_SET_CALL_ERROR(msg)   NST_SET_ERROR(nst_s.e_CallError,   msg)
#define NST_SET_IMPORT_ERROR(msg) NST_SET_ERROR(nst_s.e_ImportError, msg)

#define NST_SET_RAW_SYNTAX_ERROR(msg) NST_SET_RAW_ERROR(nst_s.e_SyntaxError, msg)
#define NST_SET_RAW_MEMORY_ERROR(msg) NST_SET_RAW_ERROR(nst_s.e_MemoryError, msg)
#define NST_SET_RAW_TYPE_ERROR(msg)   NST_SET_RAW_ERROR(nst_s.e_TypeError,   msg)
#define NST_SET_RAW_VALUE_ERROR(msg)  NST_SET_RAW_ERROR(nst_s.e_ValueError,  msg)
#define NST_SET_RAW_MATH_ERROR(msg)   NST_SET_RAW_ERROR(nst_s.e_MathError,   msg)
#define NST_SET_RAW_CALL_ERROR(msg)   NST_SET_RAW_ERROR(nst_s.e_CallError,   msg)
#define NST_SET_RAW_IMPORT_ERROR(msg) NST_SET_RAW_ERROR(nst_s.e_ImportError, msg)

#define NST_RETURN_TRUE return nst_inc_ref(nst_c.b_true)
#define NST_RETURN_FALSE return nst_inc_ref(nst_c.b_false)
#define NST_RETURN_NULL return nst_inc_ref(nst_c.null)
#define NST_RETURN_ZERO return nst_inc_ref(nst_c.Int_0)
#define NST_RETURN_ONE return nst_inc_ref(nst_c.Int_1)
#define NST_RETURN_COND(cond) \
    return (cond) ? nst_inc_ref(nst_c.b_true) : nst_inc_ref(nst_c.b_false)

#define NST_FUNC_SIGN(name) \
    Nst_Obj *name(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)

// Default call to nst_extract_arg_values
#define NST_D_EXTRACT(ltrl, ...) \
    if ( !nst_extract_arg_values(ltrl, arg_num, args, err, __VA_ARGS__) ) \
        return NULL

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_FuncDeclr
{
    Nst_Obj *(*func_ptr)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    size_t arg_num;
    Nst_StrObj *name;
}
Nst_FuncDeclr;

// Allocates the function list of the module
Nst_FuncDeclr *nst_new_func_list(size_t count);
/* Extracts the C values from the arguments
`types` is a string of letters for the types
`arg_num`: number of arguments the function expects
`args`: the arguments themselves
`err`: the `err` argument of the function
...: the pointers to store the values in

't': type, 'i': integer or byte, always as Nst_Int, 'r': real, 'N': real, integer or byte, always returns a real,
'b': bool, 'n': null, 's': string, 'v': vector, 'a': array, 'A': array or vector, 'S': array, vector or string,
returns a Nst_SeqObj that must be dec_ref'd, 'm': map, 'f': func, 'I': iter, 'B': byte, 'F': file, 'o': any object,
the ref_count doesn't change
*/
bool nst_extract_arg_values(const char *types,
                            size_t arg_num,
                            Nst_Obj **args,
                            Nst_OpErr *err,
                            ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H