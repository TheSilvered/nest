#include "nest_json.h"

#define FUNC_COUNT 5

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static bool comments = false;
static bool trailing_commas = false;
static bool split_strings = false;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(parse_s_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(parse_f_,     1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_s_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(dump_f_,      2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(set_options_, 1);
    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

NST_FUNC_SIGN(parse_s_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(parse_f_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(dump_s_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(dump_f_)
{
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(set_options_)
{
    NST_RETURN_NULL;
}
