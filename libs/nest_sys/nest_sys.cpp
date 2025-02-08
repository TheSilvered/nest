#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#ifdef Nst_MSVC

#include <direct.h>
#include <windows.h>

#define PATH_MAX 4096

#else

#include <unistd.h>
#include <cerrno>
#include <limits.h>

#endif

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(system_,          1),
    Nst_FUNCDECLR(exit_,            1),
    Nst_FUNCDECLR(get_env_,         1),
    Nst_FUNCDECLR(set_env_,         3),
    Nst_FUNCDECLR(del_env_,         1),
    Nst_FUNCDECLR(get_ref_count_,   1),
    Nst_FUNCDECLR(get_addr_,        1),
    Nst_FUNCDECLR(get_capacity_,    1),
    Nst_FUNCDECLR(hash_,            1),
    Nst_FUNCDECLR(set_cwd_,         1),
    Nst_FUNCDECLR(get_cwd_,         0),
    Nst_FUNCDECLR(_get_color_,      0),
    Nst_FUNCDECLR(_get_endianness_, 0),
    Nst_FUNCDECLR(_get_version_,    0),
    Nst_FUNCDECLR(_get_platform_,   0),
    Nst_FUNCDECLR(_raw_exit,        1),
    Nst_NAMED_CONSTDECLR(_DEBUG_, "_DEBUG"),
    Nst_DECLR_END
};
static Nst_Obj *version_obj;
static Nst_Obj *platform_obj;

Nst_Declr *lib_init()
{
    version_obj = Nst_str_new_c_raw(Nst_VERSION, false);

#ifdef Nst_MSVC
    platform_obj = Nst_str_new_c("windows", 7, false);
#else
    platform_obj = Nst_str_new_c("linux", 5, false);
#endif

    return Nst_error_occurred() ? nullptr : obj_list_;
}

void lib_quit()
{
    Nst_dec_ref(version_obj);
    Nst_dec_ref(platform_obj);
}

Nst_Obj *NstC system_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *command;
    if (!Nst_extract_args("s", arg_num, args, &command))
        return nullptr;

#ifdef Nst_MSVC
    wchar_t *wide_command = Nst_char_to_wchar_t(
        Nst_str_value(command),
        Nst_str_len(command));
    if (wide_command == nullptr)
        return nullptr;
    int command_result = _wsystem(wide_command);
    Nst_free(wide_command);
    return Nst_int_new(command_result);
#else
    return Nst_int_new(system(Nst_str_value(command)));
#endif
}

Nst_Obj *NstC exit_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *exit_code_obj;

    if (!Nst_extract_args("?i", arg_num, args, &exit_code_obj))
        return nullptr;

    if (exit_code_obj == Nst_null())
        exit_code_obj = Nst_inc_ref(Nst_const()->Int_0);
    else
        Nst_inc_ref(exit_code_obj);

    Nst_set_error(Nst_inc_ref(Nst_null()), exit_code_obj);
    return nullptr;
}

Nst_Obj *NstC get_env_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *name;

    if (!Nst_extract_args("s", arg_num, args, &name))
        return nullptr;

    if (strlen(Nst_str_value(name)) != Nst_str_len(name)) {
        Nst_set_value_error_c("the name cannot contain NUL characters");
        return nullptr;
    }

    if (strchr((const char *)Nst_str_value(name), '=') != nullptr) {
        Nst_set_value_error_c("the name cannot contain an equals sign (=)");
        return nullptr;
    }

#ifdef Nst_MSVC
    wchar_t *wide_name = Nst_char_to_wchar_t(
        Nst_str_value(name),
        Nst_str_len(name));
    if (wide_name == nullptr)
        return nullptr;

    usize buf_size = GetEnvironmentVariable(wide_name, nullptr, 0);
    if (buf_size == 0) {
        Nst_free(wide_name);
        Nst_RETURN_NULL;
    }

    wchar_t *wide_env_name = Nst_malloc_c(buf_size, wchar_t);
    usize buf_len = GetEnvironmentVariable(
        wide_name,
        wide_env_name,
        (DWORD)buf_size);
    Nst_free(wide_name);
    if (buf_len == 0) {
        Nst_free(wide_env_name);
        Nst_RETURN_NULL;
    }
    i8 *env_name = Nst_wchar_t_to_char(wide_env_name, buf_len);
    Nst_free(wide_env_name);
    if (env_name == nullptr)
        return nullptr;
#else
    i8 *env_name_str = getenv(Nst_str_value(name));
    if (env_name_str == nullptr)
        Nst_RETURN_NULL;

    i8 *env_name = Nst_malloc_c(strlen(env_name_str) + 1, i8);
    if (env_name == nullptr)
        return nullptr;
    strcpy(env_name, env_name_str);
#endif

    return Nst_str_new_c_raw(env_name, true);
}

Nst_Obj *NstC set_env_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *name;
    Nst_Obj *value;
    Nst_Obj *overwrite_obj;
    bool overwrite;

    if (!Nst_extract_args(
            "s s o",
            arg_num, args,
            &name, &value, &overwrite_obj))
    {
        return nullptr;
    }

    if (overwrite_obj == Nst_null())
        overwrite = true;
    else
        overwrite = Nst_obj_to_bool(overwrite_obj);

    if (strlen(Nst_str_value(name)) != Nst_str_len(name)
        || strlen(Nst_str_value(value)) != Nst_str_len(value))
    {
        Nst_set_value_error_c("the strings cannot contain NUL characters");
        return nullptr;
    }

    if (strchr((const char *)Nst_str_value(name), '=') != nullptr) {
        Nst_set_value_error_c("the name cannot contain an equals sign (=)");
        return nullptr;
    }

#ifdef Nst_MSVC
    wchar_t *name_buf = Nst_char_to_wchar_t(
        Nst_str_value(name),
        Nst_str_len(name));
    if (name_buf == nullptr)
        return nullptr;

    if (!overwrite) {
        if (GetEnvironmentVariable(name_buf, nullptr, 0) != 0) {
            Nst_free(name_buf);
            Nst_RETURN_NULL;
        }
        if (GetLastError() != ERROR_ENVVAR_NOT_FOUND) {
            Nst_free(name_buf);
            Nst_RETURN_NULL;
        }
    }

    wchar_t *value_buf = Nst_char_to_wchar_t(
        Nst_str_value(value),
        Nst_str_len(value));
    if (value_buf == nullptr) {
        Nst_free(name_buf);
        return nullptr;
    }

    if (SetEnvironmentVariable(name_buf, value_buf) == 0) {
        Nst_set_call_error_c("failed to set the environment variable");
        Nst_free(name_buf);
        Nst_free(value_buf);
        return nullptr;
    }
    Nst_free(name_buf);
    Nst_free(value_buf);
#else
    i32 result = setenv(
        (const i8 *)Nst_str_value(name),
        (const i8 *)Nst_str_value(value),
        overwrite);

    if (result != 0) {
        Nst_set_call_error_c("failed to set the environment variable");
        return nullptr;
    }
#endif
    Nst_RETURN_NULL;
}

Nst_Obj *NstC del_env_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *name;

    if (!Nst_extract_args("s", arg_num, args, &name))
        return nullptr;

    if (strlen(Nst_str_value(name)) != Nst_str_len(name)) {
        Nst_set_value_error_c("the name cannot contain NUL characters");
        return nullptr;
    }

    if (strchr((const char *)Nst_str_value(name), '=') != nullptr) {
        Nst_set_value_error_c("the name cannot contain an equals sign (=)");
        return nullptr;
    }

#ifdef Nst_MSVC
    wchar_t *name_buf = Nst_char_to_wchar_t(
        Nst_str_value(name),
        Nst_str_len(name));
    if (name_buf == nullptr)
        return nullptr;

    if (SetEnvironmentVariable(name_buf, NULL) == 0) {
        Nst_set_call_error_c("failed to delete the environment variable");
        Nst_free(name_buf);
        return nullptr;
    }
    Nst_free(name_buf);
#else
    if (unsetenv(Nst_str_value(name)) != 0) {
        Nst_set_call_error_c("failed to delete the environment variable");
        return nullptr;
    }
#endif
    Nst_RETURN_NULL;
}

Nst_Obj *NstC get_ref_count_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new(args[0]->ref_count);
}

Nst_Obj *NstC get_addr_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new((usize)args[0]);
}

Nst_Obj *NstC get_capacity_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *container;

    if (!Nst_extract_args("v|m", arg_num, args, &container))
        return nullptr;

    if (Nst_T(container, Map))
        return Nst_int_new((i64)Nst_map_cap(container));

    return Nst_int_new((i64)Nst_vector_cap(container));
}

Nst_Obj *NstC hash_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new(Nst_obj_hash(args[0]));
}

Nst_Obj *NstC _get_endianness_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
#if Nst_BYTEORDER == Nst_BIG_ENDIAN
    return Nst_str_new_c("big", 3, false);
#elif Nst_BYTEORDER == Nst_LITTLE_ENDIAN
    return Nst_str_new_c("little", 6, false);
#endif
}

Nst_Obj *NstC set_cwd_(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *new_cwd;
    if (!Nst_extract_args("s", arg_num, args, &new_cwd))
        return nullptr;

    if (Nst_chdir(new_cwd) == -1)
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_Obj *NstC get_cwd_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);

    return Nst_getcwd();
}

Nst_Obj *NstC _get_color_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_RETURN_BOOL(Nst_supports_color());
}

Nst_Obj *NstC _get_version_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(version_obj);
}

Nst_Obj *NstC _get_platform_(usize arg_num, Nst_Obj **args)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(platform_obj);
}

Nst_Obj *NstC _raw_exit(usize arg_num, Nst_Obj **args)
{
    Nst_Obj *exit_code_obj;

    if (!Nst_extract_args("?i", arg_num, args, &exit_code_obj))
        return nullptr;

    if (exit_code_obj == Nst_null())
        exit(0);
    else
        exit((int)AS_INT(exit_code_obj));
}

Nst_Obj *NstC _DEBUG_()
{
#ifdef _DEBUG
    Nst_RETURN_TRUE;
#else
    Nst_RETURN_FALSE;
#endif
}
