#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#define FUNC_COUNT 8

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_StrObj *version_obj;
static Nst_StrObj *platform_obj;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    size_t idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(system_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(exit_,          1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(getenv_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_ref_count_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_addr_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(hash_,          1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_version_,  0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_platform_, 0);

    lib_init_ = true;

    version_obj = STR(nst_new_cstring_raw(NEST_VERSION, false));

#if defined(_WIN32) || defined(WIN32)
    platform_obj = STR(nst_new_cstring("windows", 7, false));
#else
    platform_obj = STR(nst_new_cstring("linux", 5, false));
#endif

    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

void free_lib()
{
    nst_dec_ref(version_obj);
}

NST_FUNC_SIGN(system_)
{
    Nst_StrObj *command;
    NST_D_EXTRACT("s", &command);
    return nst_new_int(system(command->value));
}

NST_FUNC_SIGN(exit_)
{
    Nst_Obj *status;

    NST_D_EXTRACT("?i", &status);

    if ( status == nst_c.null )
    {
        exit(0);
    }
    else
    {
        exit((int)AS_INT(status));
    }
    return nullptr;
}

NST_FUNC_SIGN(getenv_)
{
    Nst_StrObj *name;

    NST_D_EXTRACT("s", &name);

    char *env_name = getenv(name->value);

    if ( env_name == nullptr )
    {
        NST_RETURN_NULL;
    }

    return nst_new_cstring_raw(env_name, false);
}

NST_FUNC_SIGN(get_ref_count_)
{
    return nst_new_int(args[0]->ref_count);
}

NST_FUNC_SIGN(get_addr_)
{
    return nst_new_int((size_t)args[0]);
}

NST_FUNC_SIGN(hash_)
{
    return nst_new_int(nst_hash_obj(args[0]));
}

NST_FUNC_SIGN(_get_version_)
{
    return nst_inc_ref(version_obj);
}

NST_FUNC_SIGN(_get_platform_)
{
    return nst_inc_ref(platform_obj);
}
