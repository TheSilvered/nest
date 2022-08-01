#include <errno.h>
#include <stdlib.h>
#include "function.h"

Nst_Obj *new_func(size_t arg_num)
{
    Nst_FuncObj *func = AS_FUNC(alloc_obj(sizeof(Nst_FuncObj), nst_t_func, destroy_func));
    Nst_Obj **args = malloc(sizeof(Nst_Obj *) * arg_num);
    if ( func == NULL || args == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = NULL;
    func->args = args;
    func->arg_num = arg_num;

    return (Nst_Obj *)func;
}

Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err))
{
    Nst_FuncObj *func = AS_FUNC(alloc_obj(sizeof(Nst_FuncObj), nst_t_func, destroy_func));
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

    return (Nst_Obj *)func;
}

Nst_Obj *new_bfunc(size_t arg_num)
{
    Nst_BcFuncObj *func = AS_BFUNC(alloc_obj(sizeof(Nst_BcFuncObj), nst_t_func, destroy_bfunc));
    Nst_Obj **args = malloc(sizeof(Nst_Obj *) * arg_num);
    if ( func == NULL || args == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = NULL;
    func->args = args;
    func->arg_num = arg_num;

    return (Nst_Obj *)func;
}

Nst_Obj *new_bcfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, Nst_OpErr *err))
{
    Nst_BcFuncObj *func = AS_BFUNC(alloc_obj(sizeof(Nst_BcFuncObj), nst_t_func, destroy_bfunc));
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

    return (Nst_Obj *)func;
}

void destroy_func(Nst_FuncObj *func)
{
    if ( func->args != NULL )
        free(func->args);
}

void destroy_bfunc(Nst_BcFuncObj *func)
{
    if ( func->args != NULL )
        free(func->args);
    if ( func->body != NULL )
        destroy_inst_list(func->body);
}
