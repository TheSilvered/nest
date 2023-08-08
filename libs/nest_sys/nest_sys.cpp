#include <stdlib.h>
#include <cstring>
#include "nest_sys.h"

#ifdef Nst_WIN

#include <direct.h>
#include <windows.h>

#define PATH_MAX 4096

#else

#include <unistd.h>
#include <cerrno>
#include <limits.h>

#endif

#define FUNC_COUNT 15

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
    is_debug = Nst_true();
#else
    is_debug = Nst_false();
#endif

    func_list_[idx++] = Nst_MAKE_FUNCDECLR(system_,        1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(exit_,          1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(getenv_,        1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(putenv_,        2);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_ref_count_, 1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(get_addr_,      1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(hash_,          1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(endianness_,    0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_color_,    0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_set_cwd_,      1);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_cwd_,      0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_version_,  0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_get_platform_, 0);
    func_list_[idx++] = Nst_MAKE_FUNCDECLR(_raw_exit,      1);
    func_list_[idx++] = Nst_MAKE_NAMED_OBJDECLR(is_debug, "_DEBUG");

#if __LINE__ - FUNC_COUNT != 40
#error
#endif

    version_obj = STR(Nst_string_new_c_raw(Nst_VERSION, false));

#ifdef Nst_WIN
    platform_obj = STR(Nst_string_new_c("windows", 7, false));
#else
    platform_obj = STR(Nst_string_new_c("linux", 5, false));
#endif

    lib_init_ = !Nst_error_occurred();
    return lib_init_;
}

Nst_DeclrList *get_func_ptrs()
{
    return lib_init_ ? &obj_list_ : nullptr;
}

void free_lib()
{
    Nst_dec_ref(version_obj);
    Nst_dec_ref(platform_obj);
}

Nst_FUNC_SIGN(system_)
{
    Nst_StrObj *command;
    Nst_DEF_EXTRACT("s", &command);

#ifdef Nst_WIN
    wchar_t *wide_command = Nst_char_to_wchar_t(command->value, command->len);
    if (wide_command == nullptr)
        return nullptr;
    int command_result = _wsystem(wide_command);
    Nst_free(wide_command);
    return Nst_int_new(command_result);
#else
    return Nst_int_new(system(command->value));
#endif
}

Nst_FUNC_SIGN(exit_)
{
    Nst_Obj *exit_code_obj;

    Nst_DEF_EXTRACT("?i", &exit_code_obj);

    if (exit_code_obj == Nst_null())
        exit_code_obj = Nst_inc_ref(Nst_const()->Int_0);
    else
        Nst_inc_ref(exit_code_obj);

    Nst_set_error(Nst_inc_ref(Nst_null()), exit_code_obj);
    return nullptr;
}

Nst_FUNC_SIGN(getenv_)
{
    Nst_StrObj *name;

    Nst_DEF_EXTRACT("s", &name);

#ifdef Nst_WIN
    wchar_t *wide_name = Nst_char_to_wchar_t(name->value, name->len);
    if (wide_name == nullptr)
        return nullptr;
    wchar_t *wide_env_name = _wgetenv(wide_name);
    Nst_free(wide_name);
    if (wide_env_name == nullptr)
        Nst_RETURN_NULL;
    i8 *env_name = Nst_wchar_t_to_char(wide_env_name, 0);
    if (env_name == nullptr)
        return nullptr;
#else
    i8 *env_name_str = getenv(name->value);
    if (env_name_str == nullptr)
        Nst_RETURN_NULL;

    i8 *env_name = Nst_malloc_c(strlen(env_name_str) + 1, i8);
    if (env_name == nullptr)
        return nullptr;
    strcpy(env_name, env_name_str);
#endif

    return Nst_string_new_c_raw(env_name, true);
}

Nst_FUNC_SIGN(putenv_)
{
    Nst_StrObj *name;
    Nst_StrObj *value;

    Nst_DEF_EXTRACT("s s", &name, &value);

    if (strlen(name->value) != name->len
        || strlen(value->value) != value->len)
    {
        Nst_set_value_error_c("the strings cannot contain NUL characters");
        return nullptr;
    }

    if (strchr((const char *)name->value, '=') != nullptr) {
        Nst_set_value_error_c("the name cannot contain an euqlas sign (=)");
        return nullptr;
    }

    usize name_len = name->len;
    usize value_len = value->len;
    usize buf_len = name_len + value_len + 1;

    i8 *buf = Nst_malloc_c(buf_len + 1, i8);
    if (buf == nullptr)
        return nullptr;
    memcpy(buf, name->value, name_len);
    buf[name_len] = '=';
    memcpy(buf + name_len + 1, value->value, value_len);
    buf[buf_len] = '\0';

#ifdef Nst_WIN
    wchar_t *wide_buf = Nst_char_to_wchar_t(buf, buf_len);
    if (wide_buf == nullptr)
        return nullptr;

    if (_wputenv((const wchar_t*)wide_buf) != 0) {
        Nst_set_call_error_c("failed to set the environment variable");
        return nullptr;
    }
    Nst_free(buf);
    Nst_free(wide_buf);
#else
    if (putenv(buf) != 0) {
        Nst_set_call_error_c("failed to set the environment variable");
        return nullptr;
    }
#endif
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(get_ref_count_)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new(args[0]->ref_count);
}

Nst_FUNC_SIGN(get_addr_)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new((usize)args[0]);
}

Nst_FUNC_SIGN(hash_)
{
    Nst_UNUSED(arg_num);
    return Nst_int_new(Nst_obj_hash(args[0]));
}

Nst_FUNC_SIGN(endianness_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
#if Nst_ENDIANNESS == Nst_BIG_ENDIAN
    return Nst_string_new_c("big", 3, false);
#elif Nst_ENDIANNESS == Nst_LITTLE_ENDIAN
    return Nst_string_new_c("little", 6, false);
#endif
}

Nst_FUNC_SIGN(_get_color_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    Nst_RETURN_COND(Nst_supports_color());
}

Nst_FUNC_SIGN(_set_cwd_)
{
    Nst_StrObj *new_cwd;
    Nst_DEF_EXTRACT("s", &new_cwd);

    if (Nst_chdir(new_cwd) == -1)
        return nullptr;
    Nst_RETURN_NULL;
}

Nst_FUNC_SIGN(_get_cwd_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);

    return OBJ(Nst_getcwd());
}

Nst_FUNC_SIGN(_get_version_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(version_obj);
}

Nst_FUNC_SIGN(_get_platform_)
{
    Nst_UNUSED(arg_num);
    Nst_UNUSED(args);
    return Nst_inc_ref(platform_obj);
}

Nst_FUNC_SIGN(_raw_exit)
{
    Nst_Obj *exit_code_obj;

    Nst_DEF_EXTRACT("?i", &exit_code_obj);

    if (exit_code_obj == Nst_null())
        exit(0);
    else
        exit((int)AS_INT(exit_code_obj));
}
