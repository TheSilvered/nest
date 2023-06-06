#include <errno.h>
#include "function.h"
#include "global_consts.h"
#include "mem.h"

Nst_Obj *nst_func_new(usize arg_num, Nst_InstList *bytecode, Nst_OpErr *err)
{
    Nst_FuncObj *func = nst_obj_alloc(
        Nst_FuncObj,
        nst_t.Func,
        _nst_func_destroy,
        err);
    Nst_Obj **args = (Nst_Obj **)nst_malloc(arg_num, sizeof(Nst_Obj *), err);
    if ( func == NULL || args == NULL )
    {
        return NULL;
    }

    func->body.bytecode = bytecode;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    NST_GGC_OBJ_INIT(func, _nst_func_traverse, _nst_func_track);

    return OBJ(func);
}

Nst_Obj *nst_func_new_c(usize arg_num,
                        Nst_Obj *(*cbody)(usize     arg_num,
                                          Nst_Obj  **args,
                                          Nst_OpErr *err),
                        Nst_OpErr *err)
{
    Nst_FuncObj *func = nst_obj_alloc(
        Nst_FuncObj,
        nst_t.Func,
        _nst_func_destroy,
        err);
    Nst_Obj **args = NULL;
    if ( func == NULL )
    {
        return NULL;
    }

    func->body.c_func = cbody;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    NST_FLAG_SET(func, NST_FLAG_FUNC_IS_C);

    // Functions with a C body never have mod_globals set
    // so there is not need to track them in the ggc

    return OBJ(func);
}

void _nst_func_traverse(Nst_FuncObj *func)
{
    if ( func->mod_globals != NULL )
    {
        NST_FLAG_SET(func->mod_globals, NST_FLAG_GGC_REACHABLE);
    }
}

void _nst_func_track(Nst_FuncObj *func)
{
    if ( func->mod_globals != NULL )
    {
        nst_ggc_track_obj((Nst_GGCObj*)func->mod_globals);
    }
}

void _nst_func_destroy(Nst_FuncObj *func)
{
    if ( func->args != NULL )
    {
        for ( usize i = 0, n = func->arg_num; i < n; i++ )
        {
            nst_dec_ref(func->args[i]);
        }
        nst_free(func->args);
    }
    if ( !NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) )
    {
        nst_inst_list_destroy(func->body.bytecode);
    }
    if ( func->mod_globals != NULL )
    {
        nst_dec_ref(func->mod_globals);
    }
}

void _nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map)
{
    if ( NST_FLAG_HAS(func, NST_FLAG_FUNC_IS_C) ||
         func->mod_globals != NULL )
    {
        return;
    }

    func->mod_globals = MAP(nst_inc_ref(map));

    for ( Nst_LLNode *n = func->body.bytecode->functions->head;
          n != NULL;
          n = n->next )
    {
        nst_func_set_vt(n->value, map);
    }
}
