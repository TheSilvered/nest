#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#ifdef WINDOWS

#include <direct.h>
#include <windows.h>

#define PATH_MAX 4096

#else

#include <unistd.h>
#include <cerrno>
#include <limits.h>

#endif

#define FUNC_COUNT 14

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_StrObj *version_obj;
static Nst_StrObj *platform_obj;
static Nst_Obj *is_debug;

bool lib_init()
{
    usize idx = 0;

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
    func_list_[idx++] = NST_MAKE_FUNCDECLR(endianness_,    0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_color_,    0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_set_cwd_,      1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_cwd_,      0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_version_,  0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_platform_, 0);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_raw_exit,      1);
    func_list_[idx++] = NST_MAKE_NAMED_OBJDECLR(is_debug, "_DEBUG");

#if __LINE__ - FUNC_COUNT != 40
#error
#endif

    version_obj = STR(nst_string_new_c_raw(NST_VERSION, false));

#ifdef WINDOWS
    platform_obj = STR(nst_string_new_c("windows", 7, false));
#else
    platform_obj = STR(nst_string_new_c("linux", 5, false));
#endif

    lib_init_ = !nst_error_occurred();
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

#ifdef WINDOWS
    wchar_t *wide_command = nst_char_to_wchar_t(command->value, command->len);
    if ( wide_command == nullptr )
    {
        return nullptr;
    }
    int command_result = _wsystem(wide_command);
    nst_free(wide_command);
    return nst_int_new(command_result);
#else
    return nst_int_new(system(command->value));
#endif
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

    nst_set_error(nst_inc_ref(nst_null()), exit_code_obj);
    return nullptr;
}

NST_FUNC_SIGN(getenv_)
{
    Nst_StrObj *name;

    NST_DEF_EXTRACT("s", &name);

#ifdef WINDOWS
    wchar_t *wide_name = nst_char_to_wchar_t(name->value, name->len);
    if ( wide_name == nullptr )
    {
        return nullptr;
    }
    wchar_t *wide_env_name = _wgetenv(wide_name);
    nst_free(wide_name);
    if ( wide_env_name == nullptr )
    {
        NST_RETURN_NULL;
    }
    i8 *env_name = nst_wchar_t_to_char(wide_env_name, 0);
    if ( env_name == nullptr )
    {
        return nullptr;
    }
#else
    i8 *env_name_str = getenv(name->value);
    if ( env_name_str == nullptr )
    {
        NST_RETURN_NULL;
    }

    i8 *env_name = nst_malloc_c(strlen(env_name_str) + 1, i8);
    if ( env_name == nullptr )
    {
        return nullptr;
    }
    strcpy(env_name, env_name_str);
#endif

    return nst_string_new_c_raw(env_name, true);
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

NST_FUNC_SIGN(endianness_)
{
#if ENDIANNESS == BIG_ENDIAN
    return nst_string_new_c("big", 3, false);
#elif ENDIANNESS == LITTLE_ENDIAN
    return nst_string_new_c("little", 6, false);
#endif
}

NST_FUNC_SIGN(_get_color_)
{
    NST_RETURN_COND(nst_supports_color());
}

NST_FUNC_SIGN(_set_cwd_)
{
    Nst_StrObj *new_cwd;
    NST_DEF_EXTRACT("s", &new_cwd);

#ifdef WINDOWS
    wchar_t *wide_cwd = nst_char_to_wchar_t(new_cwd->value, new_cwd->len);
    if ( wide_cwd == nullptr )
    {
        return nullptr;
    }

    if ( _wchdir(wide_cwd) == -1 )
    {
        errno = 0;
        nst_free(wide_cwd);
        nst_set_value_error_c("invalid path");
    }
    nst_free(wide_cwd);
#else
    if ( chdir(new_cwd->value) == -1 )
    {
        errno = 0;
        nst_set_value_error_c("invalid path");
    }
#endif
    NST_RETURN_NULL;
}

NST_FUNC_SIGN(_get_cwd_)
{
#ifdef WINDOWS
    wchar_t *cwd = nst_malloc_c(PATH_MAX, wchar_t);
    if ( cwd == nullptr )
    {
        nst_failed_allocation();
        return nullptr;
    }
    wchar_t *result = _wgetcwd(cwd, PATH_MAX);
    if ( result == nullptr )
    {
        nst_failed_allocation();
        nst_free(cwd);
        return nullptr;
    }
    i8 *cwd_str = nst_wchar_t_to_char(cwd, 0);
    nst_free(cwd);
    if ( cwd_str == nullptr )
    {
        return nullptr;
    }
    return nst_string_new_c_raw((const i8*)cwd_str, true);
#else
    i8 *cwd = nst_malloc_c(PATH_MAX, i8);
    if ( cwd == nullptr )
    {
        nst_failed_allocation();
        return nullptr;
    }
    i8 *result = getcwd(cwd, PATH_MAX);
    if ( result == nullptr )
    {
        nst_failed_allocation();
        nst_free(cwd);
        return nullptr;
    }
    return nst_string_new_c_raw((const i8 *)result, true);
#endif
}

NST_FUNC_SIGN(_get_version_)
{
    return nst_inc_ref(version_obj);
}

NST_FUNC_SIGN(_get_platform_)
{
    return nst_inc_ref(platform_obj);
}

NST_FUNC_SIGN(_raw_exit)
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
