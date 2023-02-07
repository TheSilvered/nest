#include "nest_utf8.h"

#define FUNC_COUNT 3

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_valid_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_at_,   2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(to_iter_,  1);

#if __LINE__ - FUNC_COUNT != 18
#error
#endif

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(is_valid_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(get_at_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(to_iter_)
{
    NST_RETURN_NULL;
}
