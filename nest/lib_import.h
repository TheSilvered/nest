#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "error.h"
#include "interpreter.h"
#include "obj.h"
#include "str.h"

#define NST_INIT_LIB_OBJ_FUNC \
    void init_lib_obj(Nst_Obj *main_t_type, Nst_Obj *main_t_int, \
                      Nst_Obj *main_t_real, Nst_Obj *main_t_bool, \
                      Nst_Obj *main_t_null, Nst_Obj *main_t_str, \
                      Nst_Obj *main_t_arr,  Nst_Obj *main_t_vect,\
                      Nst_Obj *main_t_map,  Nst_Obj *main_t_func, \
                      Nst_Obj *main_t_iter, Nst_Obj *main_t_byte,\
                      Nst_Obj *main_t_file, Nst_Obj *main_true, \
                      Nst_Obj *main_false,  Nst_Obj *main_null, \
                      Nst_ExecutionState main_state) \
    { \
        nst_t_type = main_t_type; \
        nst_t_int = main_t_int; \
        nst_t_real = main_t_real; \
        nst_t_bool = main_t_bool; \
        nst_t_null = main_t_null; \
        nst_t_str = main_t_str; \
        nst_t_arr = main_t_arr; \
        nst_t_vect = main_t_vect; \
        nst_t_map = main_t_map; \
        nst_t_func = main_t_func; \
        nst_t_iter = main_t_iter; \
        nst_t_byte = main_t_byte; \
        nst_t_file = main_t_file; \
        nst_true = main_true; \
        nst_false = main_false; \
        nst_null = main_null; \
        nst_state = main_state; \
    }

#define NST_MAKE_FUNCDECLR(func_ptr, argc) \
    { \
        func_ptr, \
        argc, \
        AS_STR(nst_new_string_raw(#func_ptr, false)) \
    }

#define NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, name) \
    { \
        func_ptr, \
        argc, \
        AS_STR(nst_new_string_raw(name, false)) \
    }

#define NST_SET_ERROR_CUSTOM(err_name, msg) do { \
    err->name = (char *)err_name; \
    err->message = (char *)err_msg; \
    } while ( 0 )

#define NST_SET_SYNTAX_ERROR(msg) do { \
    err->name = NST_E_SYNTAX_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_MEMORY_ERROR(msg) do { \
    err->name = NST_E_MEMORY_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_TYPE_ERROR(msg) do { \
    err->name = NST_E_TYPE_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_VALUE_ERROR(msg) do { \
    err->name = NST_E_VALUE_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_MATH_ERROR(msg) do { \
    err->name = NST_E_MATH_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_CALL_ERROR(msg) do { \
    err->name = NST_E_CALL_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_SET_GENERAL_ERROR(msg) do { \
    err->name = NST_E_GENERAL_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define NST_RETURN_TRUE return inc_ref(nst_true)
#define NST_RETURN_FALSE return inc_ref(nst_false)
#define NST_RETURN_NULL return inc_ref(nst_null)
#define NST_RETURN_COND(cond) \
    return (cond) ? inc_ref(nst_true) : inc_ref(nst_false)

#define NST_FUNC_SIGN(name) \
    Nst_Obj *name(size_t arg_num, Nst_Obj **args, Nst_OpErr *err)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Obj *(*func_ptr)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err);
    size_t arg_num;
    Nst_StrObj *name;
}
FuncDeclr;

FuncDeclr *nst_new_func_list(size_t count);
bool nst_extract_arg_values(const char *types,
                            size_t arg_num,
                            Nst_Obj **args,
                            Nst_OpErr *err,
                            ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H