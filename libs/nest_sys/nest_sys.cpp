#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#ifdef WINDOWS

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

#define FUNC_COUNT 12

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *version_obj;
static Nst_StrObj *platform_obj;
static Nst_Obj *is_debug;

bool lib_init()
{
    usize idx = 0;
    Nst_OpErr err = { nullptr, nullptr };

#ifdef _DEBUG
    is_debug = nst_true();
#else
    is_debug = nst_false();
#endif

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
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_raw_exit_,     1);
    func_list_[idx++] = NST_MAKE_NAMED_OBJDECLR(is_debug, "_DEBUG");

#if __LINE__ - FUNC_COUNT != 44
#error
#endif

    version_obj = STR(nst_string_new_c_raw(NST_VERSION, false, &err));

#ifdef WINDOWS
    platform_obj = STR(nst_string_new_c("windows", 7, false, &err));
#else
    platform_obj = STR(nst_string_new_c("linux", 5, false, &err));
#endif

    lib_init_ = err.name == nullptr;
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
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
    return nst_int_new(system(command->value), err);
}

NST_FUNC_SIGN(exit_)
{
    Nst_Obj *exit_code_obj;

    NST_DEF_EXTRACT("?i", &exit_code_obj);

    if ( exit_code_obj == nst_null() )
    {
        exit_code_obj = nst_inc_ref(nst_const()->Int_0);
    }
    else
    {
        nst_inc_ref(exit_code_obj);
    }

    NST_SET_ERROR(nst_null(), exit_code_obj);
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

    return nst_string_new_c_raw(env_name, false, err);
}

NST_FUNC_SIGN(get_ref_count_)
{
    return nst_int_new(args[0]->ref_count, err);
}

NST_FUNC_SIGN(get_addr_)
{
    return nst_int_new((usize)args[0], err);
}

NST_FUNC_SIGN(hash_)
{
    return nst_int_new(nst_obj_hash(args[0]), err);
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
    i8 *cwd = (i8 *)nst_malloc(PATH_MAX, sizeof(i8), err);
    if ( cwd == nullptr )
    {
        NST_FAILED_ALLOCATION;
        return nullptr;
    }
    return nst_string_new_c_raw(_getcwd(cwd, PATH_MAX), true, err);
}

NST_FUNC_SIGN(_get_version_)
{
    return nst_inc_ref(version_obj);
}

NST_FUNC_SIGN(_get_platform_)
{
    return nst_inc_ref(platform_obj);
}

NST_FUNC_SIGN(_raw_exit_)
{
    Nst_Obj *exit_code_obj;

    NST_DEF_EXTRACT("?i", &exit_code_obj);

    if ( exit_code_obj == nst_null() )
    {
        exit(0);
    }
    else
    {
        exit((int)AS_INT(exit_code_obj));
    }
    NST_RETURN_NULL;
}
