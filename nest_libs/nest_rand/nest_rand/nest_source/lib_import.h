#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "error.h"
#include "interpreter.h"
#include "obj.h"
#include "str.h"

#define INIT_LIB_OBJ_FUNC \
    void init_lib_obj(Nst_Obj *main_t_type, Nst_Obj *main_t_int, \
                      Nst_Obj *main_t_real, Nst_Obj *main_t_bool, \
                      Nst_Obj *main_t_null, Nst_Obj *main_t_str, \
                      Nst_Obj *main_t_arr,  Nst_Obj *main_t_vect,\
                      Nst_Obj *main_t_map,  Nst_Obj *main_t_func, \
                      Nst_Obj *main_t_iter, Nst_Obj *main_t_byte,\
                      Nst_Obj *main_t_file, Nst_Obj *main_true, \
                      Nst_Obj *main_false,  Nst_Obj *main_null, \
                      ExecutionState *main_state) \
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
        state = main_state; \
    }

#define MAKE_FUNCDECLR(func_ptr, argc) { func_ptr, argc, AS_STR(new_string_raw(#func_ptr, false)) }
#define MAKE_NAMED_FUNCDECLR(func_ptr, argc, name) { func_ptr, argc, AS_STR(new_string_raw(name, false)) }

#define SET_ERROR_CUSTOM(err_name, msg) do { \
    err->name = (char *)err_name; \
    err->message = (char *)err_msg; \
    } while ( 0 )

#define SET_SYNTAX_ERROR(msg) do { \
    err->name = SYNTAX_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_MEMORY_ERROR(msg) do { \
    err->name = MEMORY_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_TYPE_ERROR(msg) do { \
    err->name = TYPE_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_VALUE_ERROR(msg) do { \
    err->name = VALUE_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_MATH_ERROR(msg) do { \
    err->name = MATH_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_CALL_ERROR(msg) do { \
    err->name = CALL_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define SET_GENERAL_ERROR(msg) do { \
    err->name = GENERAL_ERROR; \
    err->message = (char *)msg; \
    } while ( 0 )

#define RETURN_TRUE return inc_ref(nst_true)
#define RETURN_FALSE return inc_ref(nst_false)
#define RETURN_NULL return inc_ref(nst_null)
#define RETURN_COND(cond) return (cond) ? inc_ref(nst_true) : inc_ref(nst_false)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Obj *(*func_ptr)(size_t arg_num, Nst_Obj **args, OpErr *err);
    size_t arg_num;
    Nst_StrObj *name;
}
FuncDeclr;

FuncDeclr *new_func_list(size_t count);
bool extract_arg_values(const char *types,
                        size_t arg_num,
                        Nst_Obj **args,
                        OpErr *err,
                        ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H