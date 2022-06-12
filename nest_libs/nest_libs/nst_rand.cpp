#include "pch.h"
#include <random>
#include "nst_rand.h"

#define FUNC_COUNT 1

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == NULL )
        return false;

    func_list_[0] = { randint, 0, new_string_raw("randint", false) };
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : NULL;
}

Nst_Obj *randint(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    return new_int_obj(rand());
}
