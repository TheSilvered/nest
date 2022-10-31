#include <errno.h>
#include <stdlib.h>
#include "function.h"

Nst_Obj *new_func(size_t arg_num)
{
    Nst_FuncObj *func = AS_FUNC(nst_alloc_obj(
        sizeof(Nst_FuncObj),
        nst_t_func,
        nst_destroy_func
    ));
    Nst_Obj **args = (Nst_Obj **)malloc(sizeof(Nst_Obj *) * arg_num);
    if ( func == NULL || args == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = NULL;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    NST_GGC_SUPPORT_INIT(func, nst_traverse_func);

    return (Nst_Obj *)func;
}

Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                     Nst_Obj **args,
                                                     Nst_OpErr *err))
{
    Nst_FuncObj *func = AS_FUNC(nst_alloc_obj(
        sizeof(Nst_FuncObj),
        nst_t_func,
        nst_destroy_func
    ));
    Nst_Obj **args = NULL;
    if ( func == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = cbody;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    // Functions with a C body never have mod_globals set
    // so there is not need to track them in the ggc

    return (Nst_Obj *)func;
}

void nst_traverse_func(Nst_FuncObj *func)
{
    if ( func->mod_globals != NULL )
        nst_traverse_map(func->mod_globals);
}

void nst_destroy_func(Nst_FuncObj *func)
{
    if ( func->args != NULL )
        free(func->args);
    if ( func->body != NULL )
        nst_destroy_inst_list(func->body);
    if ( func->mod_globals != NULL )
        nst_dec_ref(func->mod_globals);
}

void _nst_set_global_vt(Nst_FuncObj *func, Nst_MapObj *map)
{
    if ( func->body == NULL )
        return;

    func->mod_globals = AS_MAP(nst_inc_ref(map));

    Nst_RuntimeInstruction *instructions = func->body->instructions;
    for ( size_t i = 0, n = func->body->total_size; i < n; i++ )
    {
        if ( instructions[i].val != NULL && instructions[i].val->type == nst_t_func )
            nst_set_global_vt(instructions[i].val, nst_inc_ref(map));
    }
}
