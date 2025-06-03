#include <errno.h>
#include <string.h>
#include "nest.h"

typedef union _Nst_FuncBody {
    Nst_Bytecode *bytecode;
    Nst_Obj *(*c_func)(usize arg_num, Nst_Obj **args);
} Nst_FuncBody;

typedef struct _Nst_FuncObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
    Nst_FuncBody body;
    Nst_Obj **args;
    isize arg_num;
    Nst_Obj *mod_globals;
    Nst_Obj *outer_vars;
} Nst_FuncObj;

#define FUNC(ptr) ((Nst_FuncObj *)(ptr))

Nst_Obj *_Nst_func_new(Nst_Obj **arg_names, usize arg_num, Nst_Bytecode *bc)
{
    Nst_Obj **args = Nst_malloc_c(arg_num, Nst_Obj *);
    if (args == NULL)
        return NULL;

    Nst_FuncObj *func = Nst_obj_alloc(Nst_FuncObj, Nst_t.Func);
    if (func == NULL) {
        Nst_free(args);
        return NULL;
    }

    Nst_GGC_OBJ_INIT(func);

    for (usize i = 0; i < arg_num; i++)
        args[i] = Nst_inc_ref(arg_names[i]);

    func->body.bytecode = bc;
    func->args = args;
    func->arg_num = arg_num;
    func->mod_globals = NULL;
    func->outer_vars = NULL;

    return NstOBJ(func);
}

Nst_Obj *Nst_func_new_c(usize arg_num, Nst_NestCallable cbody)
{
    Nst_FuncObj *func = Nst_obj_alloc(Nst_FuncObj, Nst_t.Func);
    if (func == NULL)
        return NULL;

    func->body.c_func = cbody;
    func->args = NULL;
    func->arg_num = arg_num;
    func->mod_globals = NULL;
    func->outer_vars = NULL;

    Nst_SET_FLAG(func, Nst_FLAG_FUNC_IS_C);

    // Functions with a C body never have mod_globals set
    // so there is not need to track them in the ggc

    return NstOBJ(func);
}

Nst_Obj *_Nst_func_new_outer_vars(Nst_Obj *func, Nst_Obj *vars)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_assert(vars->type == Nst_t.Map);

    if (Nst_FUNC_IS_C(func))
        return Nst_inc_ref(func);

    usize arg_num = FUNC(func)->arg_num;
    Nst_Obj **args = Nst_malloc_c(arg_num, Nst_Obj *);
    if (args == NULL)
        return NULL;

    Nst_FuncObj *new_func = Nst_obj_alloc(Nst_FuncObj, Nst_t.Func);
    if (new_func == NULL) {
        Nst_free(args);
        return NULL;
    }

    memcpy(args, FUNC(func)->args, sizeof(Nst_Obj *) * arg_num);
    for (usize i = 0, n = arg_num; i < n; i++)
        Nst_inc_ref(args[i]);

    new_func->body.bytecode = Nst_bc_copy(FUNC(func)->body.bytecode);
    new_func->args = args;
    new_func->arg_num = FUNC(func)->arg_num;
    new_func->mod_globals = Nst_ninc_ref(FUNC(func)->mod_globals);
    new_func->outer_vars = Nst_inc_ref(vars);

    Nst_GGC_OBJ_INIT(new_func);

    return NstOBJ(new_func);
}

void _Nst_func_traverse(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    if (FUNC(func)->mod_globals != NULL)
        Nst_ggc_obj_reachable(FUNC(func)->mod_globals);
    if (FUNC(func)->outer_vars != NULL)
        Nst_ggc_obj_reachable(FUNC(func)->outer_vars);
}

void _Nst_func_destroy(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    if (FUNC(func)->args != NULL) {
        for (usize i = 0, n = FUNC(func)->arg_num; i < n; i++)
            Nst_dec_ref(FUNC(func)->args[i]);
        Nst_free(FUNC(func)->args);
    }
    if (!Nst_FUNC_IS_C(func))
        Nst_bc_destroy(FUNC(func)->body.bytecode);
    if (FUNC(func)->mod_globals != NULL)
        Nst_dec_ref(FUNC(func)->mod_globals);
    if (FUNC(func)->outer_vars != NULL)
        Nst_dec_ref(FUNC(func)->outer_vars);
}

void _Nst_func_set_mod_globals(Nst_Obj *func, Nst_Obj *globals)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_assert(globals->type == Nst_t.Map);

    if (Nst_FUNC_IS_C(func) || FUNC(func)->mod_globals != NULL)
        return;

    FUNC(func)->mod_globals = Nst_inc_ref(globals);
}

usize Nst_func_arg_num(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    return FUNC(func)->arg_num;
}

Nst_Obj **Nst_func_args(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    return FUNC(func)->args;
}

Nst_NestCallable Nst_func_c_body(Nst_Obj *func)
{
    Nst_assert_c(func->type == Nst_t.Func);
    Nst_assert_c(Nst_FUNC_IS_C(func));
    return FUNC(func)->body.c_func;
}

Nst_Bytecode *Nst_func_nest_body(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    Nst_assert(!Nst_FUNC_IS_C(func));
    return FUNC(func)->body.bytecode;
}

Nst_Obj *Nst_func_mod_globals(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    return FUNC(func)->mod_globals;
}

Nst_Obj *Nst_func_outer_vars(Nst_Obj *func)
{
    Nst_assert(func->type == Nst_t.Func);
    return FUNC(func)->outer_vars;
}
