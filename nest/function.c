#include <errno.h>
#include <stdlib.h>
#include "function.h"
#include "obj.h"

Nst_Obj *new_func_obj(Nst_func *func)
{
    return make_obj(func, nst_t_func, destroy_func);
}

Nst_func *new_func(size_t arg_num)
{
    Nst_func *func = malloc(sizeof(Nst_func));
    Nst_string **args = malloc(sizeof(Nst_string *) * arg_num);
    if ( func == NULL || args == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = NULL;
    func->args = args;
    func->arg_num = arg_num;

    return func;
}

Nst_func *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num, Nst_Obj **args, OpErr *err))
{
    Nst_func *func = malloc(sizeof(Nst_func));
    Nst_string **args = NULL;
    if ( func == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body = NULL;
    func->cbody = cbody;
    func->args = args;
    func->arg_num = arg_num;

    return func;
}

void destroy_func(Nst_func *func)
{
    if ( func->args != NULL )
        free(func->args);
    free(func);
}
