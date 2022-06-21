#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "obj.h"
#include "error.h"
#include "str.h"

#define INIT_LIB_OBJ_FUNC \
    void init_lib_obj(Nst_Obj *main_t_type, Nst_Obj *main_t_int, \
                      Nst_Obj *main_t_real, Nst_Obj *main_t_bool, \
                      Nst_Obj *main_t_null, Nst_Obj *main_t_str, \
                      Nst_Obj *main_t_arr,  Nst_Obj *main_t_vect,\
                      Nst_Obj *main_t_map,  Nst_Obj *main_t_func, \
                      Nst_Obj *main_t_iter, Nst_Obj *main_t_byte,\
                      Nst_Obj *main_t_file, Nst_Obj *main_true, \
                      Nst_Obj *main_false,  Nst_Obj *main_null) \
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
    }

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct FuncDeclr
{
    Nst_Obj *(*func_ptr)(size_t arg_num, Nst_Obj **args, OpErr *err);
    size_t arg_num;
    Nst_string *name;
}
FuncDeclr;

FuncDeclr *new_func_list(size_t count);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H