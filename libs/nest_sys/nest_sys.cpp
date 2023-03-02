#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#if defined(_WIN32) || defined(WIN32)

#include <direct.h>
#include <windows.h>

#define PATH_MAX MAX_PATH

#else

#include <unistd.h>
#include <cerrno>
#include <limits.h>

#define _chdir chdir
#define _getcwd getcwd

#endif

#define FUNC_COUNT 10

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_StrObj *version_obj;
static Nst_StrObj *platform_obj;

bool lib_init()
{
    if ( (func_list_ = nst_func_list_new(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(system_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(exit_,          1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(getenv_,        1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_ref_count_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(get_addr_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(hash_,          1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_set_cwd_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_cwd_,      0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_version_,  0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_platform_, 0);

#if __LINE__ - FUNC_COUNT != 40
#error FUNC_COUNT does not match the number of lines
#endif

    lib_init_ = true;

    version_obj = STR(nst_string_new_c_raw(NST_VERSION, false));

#if defined(_WIN32) || defined(WIN32)
    platform_obj = STR(nst_string_new_c("windows", 7, false));
#else
    platform_obj = STR(nst_string_new_c("linux", 5, false));
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
    nst_dec_ref(platform_obj);
}

NST_FUNC_SIGN(system_)
{
    Nst_StrObj *command;
    NST_DEF_EXTRACT("s", &command);
    return nst_int_new(system(command->value));
}

NST_FUNC_SIGN(exit_)
{
    Nst_Obj *status;

    NST_DEF_EXTRACT("?i", &status);
    Nst_Int code;

    if ( status == nst_null() )
    {
        code = 0;
    }
    else
    {
        code = AS_INT(status);
    }

    nst_state_free();
    _nst_types_del();
    _nst_strs_del();
    _nst_consts_del();
    _nst_streams_del();
    _nst_unload_libs();
    exit((i32)code);

    return nullptr;
}

NST_FUNC_SIGN(getenv_)
{
    Nst_StrObj *name;

    NST_DEF_EXTRACT("s", &name);

    i8 *env_name = getenv(name->value);

    if ( env_name == nullptr )
    {
        NST_RETURN_NULL;
    }

    return nst_string_new_c_raw(env_name, false);
}

NST_FUNC_SIGN(get_ref_count_)
{
    return nst_int_new(args[0]->ref_count);
}

NST_FUNC_SIGN(get_addr_)
{
    return nst_int_new((usize)args[0]);
}

NST_FUNC_SIGN(hash_)
{
    return nst_int_new(nst_obj_hash(args[0]));
}

NST_FUNC_SIGN(_set_cwd_)
{
    Nst_StrObj *new_cwd;
    NST_DEF_EXTRACT("s", &new_cwd);
    if ( _chdir(new_cwd->value) == -1 )
    {
        errno = 0;
        NST_SET_RAW_VALUE_ERROR("invalid path");
    }
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_get_cwd_)
{
    i8 *cwd = (i8 *)malloc(PATH_MAX * sizeof(i8));
    if ( cwd == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }
    return nst_string_new_c_raw(_getcwd(cwd, PATH_MAX), true);
}

NST_FUNC_SIGN(_get_version_)
{
    return nst_inc_ref(version_obj);
}

NST_FUNC_SIGN(_get_platform_)
{
    return nst_inc_ref(platform_obj);
}
