#include <errno.h>
#include <stdlib.h>
#include "function.h"
#include "global_consts.h"

Nst_Obj *new_func(size_t arg_num, Nst_InstructionList *bytecode)
{
    Nst_FuncObj *func = FUNC(nst_alloc_obj(
        sizeof(Nst_FuncObj),
        nst_t.Func,
        nst_destroy_func
    ));
    Nst_Obj **args = (Nst_Obj **)malloc(sizeof(Nst_Obj *) * arg_num);
    if ( func == NULL || args == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body.bytecode = bytecode;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    NST_GGC_SUPPORT_INIT(func, nst_traverse_func, nst_track_func);

    return (Nst_Obj *)func;
}

Nst_Obj *new_cfunc(size_t arg_num, Nst_Obj *(*cbody)(size_t arg_num,
                                                     Nst_Obj **args,
                                                     Nst_OpErr *err))
{
    Nst_FuncObj *func = FUNC(nst_alloc_obj(
        sizeof(Nst_FuncObj),
        nst_t.Func,
        nst_destroy_func
    ));
    Nst_Obj **args = NULL;
    if ( func == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    func->body.c_func = cbody;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    NST_SET_FLAG(func, NST_FLAG_FUNC_IS_C);

    // Functions with a C body never have mod_globals set
    // so there is not need to track them in the ggc

    return (Nst_Obj *)func;
}

void nst_traverse_func(Nst_FuncObj *func)
{
    if ( func->mod_globals != NULL )
        nst_traverse_map(func->mod_globals);
}

void nst_track_func(Nst_FuncObj *func)
{
    if ( func->mod_globals != NULL )
        nst_add_tracked_object((Nst_GGCObj *)func->mod_globals);
}

void nst_destroy_func(Nst_FuncObj *func)
{
    if ( func->args != NULL )
        free(func->args);
    if ( !NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
        nst_destroy_inst_list(func->body.bytecode);
    if ( func->mod_globals != NULL )
        nst_dec_ref(func->mod_globals);
}

void _nst_set_global_vt(Nst_FuncObj *func, Nst_MapObj *map)
{
    if ( NST_HAS_FLAG(func, NST_FLAG_FUNC_IS_C) )
        return;

    func->mod_globals = MAP(nst_inc_ref(map));

    Nst_RuntimeInstruction *instructions = func->body.bytecode->instructions;
    for ( size_t i = 0, n = func->body.bytecode->total_size; i < n; i++ )
    {
        if ( instructions[i].val != NULL && instructions[i].val->type == nst_t.Func )
            nst_set_global_vt(instructions[i].val, nst_inc_ref(map));
    }
}
