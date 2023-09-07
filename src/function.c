#include <errno.h>
#include "function.h"
#include "global_consts.h"
#include "mem.h"

Nst_Obj *Nst_func_new(usize arg_num, Nst_InstList *bytecode)
{
    Nst_FuncObj *func = Nst_obj_alloc(Nst_FuncObj, Nst_t.Func);
    Nst_Obj **args = Nst_malloc_c(arg_num, Nst_Obj *);
    if (func == NULL || args == NULL)
        return NULL;

    Nst_GGC_OBJ_INIT(func);

    func->body.bytecode = bytecode;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    return OBJ(func);
}

Nst_Obj *Nst_func_new_c(usize arg_num, Nst_Obj *(*cbody)(usize, Nst_Obj **))
{
    Nst_FuncObj *func = Nst_obj_alloc(Nst_FuncObj, Nst_t.Func);
    Nst_Obj **args = NULL;
    if (func == NULL)
        return NULL;

    func->body.c_func = cbody;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;

    Nst_SET_FLAG(func, Nst_FLAG_FUNC_IS_C);

    // Functions with a C body never have mod_globals set
    // so there is not need to track them in the ggc

    return OBJ(func);
}

void _Nst_func_traverse(Nst_FuncObj *func)
{
    if (func->mod_globals != NULL)
        Nst_ggc_obj_reachable(func->mod_globals);
}

void _Nst_func_destroy(Nst_FuncObj *func)
{
    if (func->args != NULL) {
        for (usize i = 0, n = func->arg_num; i < n; i++)
            Nst_dec_ref(func->args[i]);
        Nst_free(func->args);
    }
    if (!Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C))
        Nst_inst_list_destroy(func->body.bytecode);
    if (func->mod_globals != NULL)
        Nst_dec_ref(func->mod_globals);
}

void _Nst_func_set_vt(Nst_FuncObj *func, Nst_MapObj *map)
{
    if (Nst_HAS_FLAG(func, Nst_FLAG_FUNC_IS_C) || func->mod_globals != NULL)
        return;

    func->mod_globals = MAP(Nst_inc_ref(map));

    for (Nst_LLNode *n = func->body.bytecode->functions->head;
         n != NULL;
         n = n->next)
    {
        Nst_func_set_vt(n->value, map);
    }
}
