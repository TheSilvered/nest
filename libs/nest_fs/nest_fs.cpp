#include "nest_fs.h"

#ifdef Nst_MSVC

#pragma warning(push)
#pragma warning(disable: 4995)
#include <windows.h>

#else

// This error does not get thrown on UNIX
#define ERROR_ALREADY_EXISTS 0
// ERROR_PATH_NOT_FOUND and ERROR_FILE_NOT_FOUND are the same on UNIX
#define ERROR_PATH_NOT_FOUND ENOENT

#include <sys/stat.h> // needed for file time

#endif

#include <filesystem>
#include <cerrno>
#include <cstring>

#ifdef Nst_MSVC
#pragma warning(pop)
#endif

#define COPY_OPT(name) Nst_int_new((i64)fs::copy_options::name)

namespace fs = std::filesystem;

static Nst_Declr obj_list_[] = {
    Nst_FUNCDECLR(is_dir_, 1),
    Nst_FUNCDECLR(is_file_, 1),
    Nst_FUNCDECLR(is_symlink_, 1),
    Nst_FUNCDECLR(is_socket_, 1),
    Nst_FUNCDECLR(is_block_device_, 1),
    Nst_FUNCDECLR(is_char_device_, 1),
    Nst_FUNCDECLR(is_named_pipe_, 1),
    Nst_FUNCDECLR(make_dir_, 1),
    Nst_FUNCDECLR(make_dirs_, 1),
    Nst_FUNCDECLR(remove_, 1),
    Nst_FUNCDECLR(remove_all_, 1),
    Nst_FUNCDECLR(make_dir_symlink_, 2),
    Nst_FUNCDECLR(make_file_symlink_, 2),
    Nst_FUNCDECLR(read_symlink_, 1),
    Nst_FUNCDECLR(make_hard_link_, 2),
    Nst_FUNCDECLR(exists_, 1),
    Nst_FUNCDECLR(copy_, 3),
    Nst_FUNCDECLR(rename_, 2),
    Nst_FUNCDECLR(list_dir_, 1),
    Nst_FUNCDECLR(list_dirs_, 1),
    Nst_FUNCDECLR(absolute_path_, 1),
    Nst_FUNCDECLR(canonical_path_, 1),
    Nst_FUNCDECLR(relative_path_, 2),
    Nst_FUNCDECLR(equivalent_, 2),
    Nst_FUNCDECLR(path_join_, 2),
    Nst_FUNCDECLR(path_normalize_, 1),
    Nst_FUNCDECLR(path_parent_, 1),
    Nst_FUNCDECLR(path_filename_, 1),
    Nst_FUNCDECLR(path_extension_, 1),
    Nst_FUNCDECLR(path_stem_, 1),
    Nst_FUNCDECLR(time_creation_, 1),
    Nst_FUNCDECLR(time_last_access_, 1),
    Nst_FUNCDECLR(time_last_write_, 1),
    Nst_CONSTDECLR(CPO_),
    Nst_DECLR_END
};

Nst_Declr *lib_init()
{
    return obj_list_;
}

static Nst_StrObj *heap_str(const i8 *str, usize len)
{
    i8 *heap_s = Nst_malloc_c(len + 1, i8);
    if (heap_s == nullptr)
        return nullptr;

    memcpy(heap_s, str, len);
    heap_s[len] = 0;
    return STR(Nst_str_new(heap_s, len, true));
}

static Nst_StrObj *heap_str(fs::path path)
{
    std::u8string str = path.u8string();
    return heap_str((const i8 *)str.c_str(), str.length());
}

static Nst_StrObj *error_str(std::string str)
{
#ifdef Nst_MSVC
    i8 *val;
    usize len;
    bool result = Nst_encoding_translate(
        Nst_encoding(Nst_acp()), Nst_encoding(Nst_EID_UTF8),
        (void *)str.c_str(), str.length(), (void **)&val, &len);
    if (!result)
        return STR(Nst_inc_ref(Nst_str()->o_failed_alloc));

    return STR(Nst_str_new_allocated(val, len));
#else
    return heap_str(str.c_str(), str.length());
#endif
}

static Nst_Obj *throw_system_error(std::error_code ec)
{
    if (ec.value() == 0) {
        Nst_set_error(
            Nst_str_new_c_raw("System Error <unknown>", false),
            Nst_str_new_c_raw("an unknown error occurred", false));
        return nullptr;
    }

    Nst_set_error(
        Nst_sprintf("System Error %d", ec.value()),
        error_str(ec.message()));
    return nullptr;
}

static Nst_Obj *throw_c_error(void)
{
#ifdef Nst_MSVC
    DWORD error = GetLastError();
    wchar_t *wide_msg;
    FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        nullptr,
        error,
        LANG_USER_DEFAULT,
        (LPWSTR)&wide_msg,
        0,
        nullptr);
    i8 *msg_str = Nst_wchar_t_to_char(wide_msg, wcslen(wide_msg));
    LocalFree(wide_msg);
    Nst_StrObj *msg = STR(Nst_str_new_allocated(msg_str, strlen(msg_str)));
#else
    i8 *val = strerror(errno);
    int error = errno;
    Nst_StrObj *msg = heap_str((const i8 *)val, strlen(val));
#endif // !Nst_MSVC
    Nst_set_error(
        Nst_sprintf("System Error %d", error),
        msg);
    return nullptr;
}

static fs::path utf8_path(Nst_StrObj *str)
{
    std::u8string s { (char8_t *)str->value };
    return fs::path(s);
}

bool check_path(Nst_StrObj *path, bool (*func)(const fs::path&,
                                               std::error_code&))
{
    std::error_code ec;
    bool check = func(utf8_path(path), ec);
    ec.clear();
    return check;
}

Nst_Obj *NstC is_dir_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_directory));
}

Nst_Obj *NstC is_file_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_regular_file));
}

Nst_Obj *NstC is_symlink_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_symlink));
}

Nst_Obj *NstC is_socket_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_socket));
}

Nst_Obj *NstC is_block_device_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_block_file));
}

Nst_Obj *NstC is_char_device_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_character_file));
}

Nst_Obj *NstC is_named_pipe_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::is_fifo));
}

Nst_Obj *NstC make_dir_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    std::error_code ec;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    bool success = fs::create_directory(utf8_path(path), ec);

    if (success || ec.value() == ERROR_ALREADY_EXISTS || ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC make_dirs_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    std::error_code ec;
    bool success = fs::create_directories(utf8_path(path), ec);

    if (success || ec.value() == ERROR_ALREADY_EXISTS || ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC remove_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    bool success = fs::remove(utf8_path(path), ec);

    if (success)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC remove_all_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    bool success = fs::remove_all(utf8_path(path), ec);

    if (success)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC make_dir_symlink_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    if (!Nst_extract_args("s s", arg_num, args, &target, &link))
        return nullptr;

    fs::create_directory_symlink(utf8_path(target), utf8_path(link), ec);
    if (ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC make_file_symlink_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    if (!Nst_extract_args("s s", arg_num, args, &target, &link))
        return nullptr;

    fs::create_symlink(utf8_path(target), utf8_path(link), ec);
    if (ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC read_symlink_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    std::error_code ec;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    if (!check_path(path, fs::is_symlink)) {
        Nst_set_value_errorf("symlink '%.4096s' not found", path->value);
        return nullptr;
    }

    fs::path result = fs::read_symlink(utf8_path(path), ec);
    if (ec.value() == 0)
        return OBJ(heap_str(result));
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC make_hard_link_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    if (!Nst_extract_args("s s", arg_num, args, &target, &link))
        return nullptr;

    fs::create_hard_link(utf8_path(target), utf8_path(link), ec);
    if (ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC exists_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_RETURN_BOOL(check_path(path, fs::exists));
}

Nst_Obj *NstC copy_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path_from;
    Nst_StrObj *path_to;
    Nst_Obj *options;

    if (!Nst_extract_args(
            "s s ?i",
            arg_num, args,
            &path_from, &path_to, &options))
    {
        return nullptr;
    }

    fs::copy_options cp_options = Nst_DEF_VAL(
        options,
        (fs::copy_options)AS_INT(options),
        fs::copy_options::none);

    std::error_code ec;
    fs::copy(utf8_path(path_from), utf8_path(path_to), cp_options, ec);

    if (ec.value() == ERROR_PATH_NOT_FOUND) {
        Nst_set_value_errorf(
            "'%.4096s' or '%.4096s' not found",
            path_from->value,
            path_to->value);
        return nullptr;
    } else if (ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC rename_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *old_path;
    Nst_StrObj *new_path;

    if (!Nst_extract_args("s s", arg_num, args, &old_path, &new_path))
        return nullptr;

    std::error_code ec;

    fs::rename(utf8_path(old_path), utf8_path(new_path), ec);

    if (ec.value() == ERROR_PATH_NOT_FOUND) {
        Nst_set_value_errorf(
            "file '%.4096s' or directory '%.4096s' not found",
            old_path->value,
            new_path->value);
        return nullptr;
    } else if (ec.value() == 0)
        Nst_RETURN_NULL;
    else
        return throw_system_error(ec);
}

Nst_Obj *NstC list_dir_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    if (!fs::is_directory(utf8_path(path), ec)) {
        Nst_set_value_errorf(
            "directory '%.4096s' not found",
            path->value);
        return nullptr;
    }
    if (ec.value() != 0)
        return throw_system_error(ec);

    Nst_Obj *vector = Nst_vector_new(0);

    for (fs::directory_entry const &entry
         : fs::directory_iterator{ utf8_path(path) })
    {
        Nst_StrObj *str = heap_str(entry.path());
        if (str == nullptr)
            return nullptr;
        Nst_vector_append(vector, OBJ(str));
        Nst_dec_ref(str);
    }

    return OBJ(vector);
}

Nst_Obj *NstC list_dirs_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    if (!fs::is_directory(utf8_path(path), ec)) {
        Nst_set_value_errorf(
            "directory '%.4096s' not found",
            path->value);
        return nullptr;
    }
    if (ec.value() != 0)
        return throw_system_error(ec);

    Nst_Obj *vector = Nst_vector_new(0);

    for (fs::directory_entry const &entry
        : fs::recursive_directory_iterator{ utf8_path(path) })
    {
        Nst_StrObj *str = heap_str(entry.path());
        if (str == nullptr)
            return nullptr;
        Nst_vector_append(vector, OBJ(str));
        Nst_dec_ref(str);
    }

    return OBJ(vector);
}

Nst_Obj *NstC absolute_path_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    fs::path result = fs::absolute(utf8_path(path), ec);

    if (ec.value() == 0)
        return OBJ(heap_str(result));
    else
        Nst_RETURN_NULL;
}

Nst_Obj *NstC canonical_path_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

    std::error_code ec;
    fs::path result = fs::canonical(utf8_path(path), ec);

    if (ec.value() == 0)
        return OBJ(heap_str(result));
    else
        Nst_RETURN_NULL;
}

Nst_Obj *NstC relative_path_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    Nst_StrObj *base;

    if (!Nst_extract_args("s ?s", arg_num, args, &path, &base))
        return nullptr;

    if (OBJ(base) == Nst_null()) {
        base = Nst_getcwd();
        if (base == nullptr)
            return nullptr;
    } else
        Nst_inc_ref(base);

    std::error_code ec;
    fs::path result = fs::relative(utf8_path(path), utf8_path(base), ec);

    Nst_dec_ref(base);

    if (ec.value() == 0)
        return OBJ(heap_str(result));
    else
        Nst_RETURN_NULL;
}

Nst_Obj *NstC equivalent_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    if (!Nst_extract_args("s s", arg_num, args, &path_1, &path_2))
        return nullptr;

    std::error_code ec;
    Nst_RETURN_BOOL(fs::equivalent(utf8_path(path_1), utf8_path(path_2), ec));
}

static void normalize_path(i8 *path, usize len)
{
    if (len >= 4 && strncmp((const i8 *)path, "\\\\?\\", 4) == 0) {
        path += 4;
        len -= 4;
    }

    for (usize i = 0; i < len; i++) {
#ifdef Nst_MSVC
        if (path[i] == '/')
            path[i] = '\\';
#else
        if (path[i] == '\\')
            path[i] = '/';
#endif // !Nst_MSVC
    }
}

Nst_Obj *NstC path_join_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    if (!Nst_extract_args("s s", arg_num, args, &path_1, &path_2))
        return nullptr;

    i8 *p1 = path_1->value;
    i8 *p2 = path_2->value;
    usize p1_len = path_1->len;
    usize p2_len = path_2->len;

    if (p2_len == 0) {
        Nst_StrObj *norm_path = STR(Nst_str_copy(path_1));
        if (norm_path == nullptr)
            return nullptr;
        normalize_path(norm_path->value, norm_path->len);
        return OBJ(norm_path);
    }

    // These conditions work on:
    // - Unix absolute paths (/dir)
    // - Windows absolute paths (C:\dir or C:/dir)
    // - Windows drive-relative paths (C:dir)
    // - Windows current drive absolute paths (\dir or /dir)
    // - Windows extended paths (\\?\C:\dir)
    // All of which should not be joined after another path

    if (p2[0] == '/' || p2[0] == '\\' || p2[1] == ':' || p1_len == 0) {
        Nst_StrObj *norm_path = STR(Nst_str_copy(path_2));
        if (norm_path == nullptr)
            return nullptr;
        normalize_path(norm_path->value, norm_path->len);
        return OBJ(norm_path);
    }

    usize new_len = p1_len + p2_len;
    bool add_slash = false;

    if (p1[p1_len - 1] != '/' && p1[p1_len - 1] != '\\') {
        new_len++;
        add_slash = true;
    }

    i8 *new_str = Nst_malloc_c(new_len + 1, i8);
    if (new_str == nullptr)
        return nullptr;

    memcpy(new_str, p1, p1_len);

    if (add_slash)
        new_str[p1_len] = '/';
    memcpy(new_str + p1_len + (add_slash ? 1 : 0), p2, p2_len);
    new_str[new_len] = 0;
    normalize_path(new_str, new_len);
    return Nst_str_new(new_str, new_len, true);
}

Nst_Obj *NstC path_normalize_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;

    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    Nst_StrObj *norm_path = STR(Nst_str_copy(path));
    if (norm_path == nullptr)
        return nullptr;
    normalize_path(norm_path->value, norm_path->len);
    return OBJ(norm_path);
}

Nst_Obj *NstC path_parent_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    return OBJ(heap_str(utf8_path(path).parent_path()));
}

Nst_Obj *NstC path_filename_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    return OBJ(heap_str(utf8_path(path).filename()));
}

Nst_Obj *NstC path_stem_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    return OBJ(heap_str(utf8_path(path).stem()));
}

Nst_Obj *NstC path_extension_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;
    return OBJ(heap_str(utf8_path(path).extension()));
}

#ifdef Nst_MSVC
static i64 FILETIME_to_unix_time(FILETIME time)
{
    // January 1, 1970 (start of Unix epoch) in "ticks"
    const i64 UNIX_TIME_START = 0x019DB1DED53E8000;
    // a tick is 100ns
    const i64 TICKS_PER_SECOND = 10000000;

    ULARGE_INTEGER large_int;
    large_int.HighPart = time.dwHighDateTime;
    large_int.LowPart = time.dwLowDateTime;

    return (large_int.QuadPart - UNIX_TIME_START) / TICKS_PER_SECOND;
}
#endif // !Nst_MSVC

Nst_Obj *NstC time_creation_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

#ifdef Nst_MSVC
    wchar_t *wide_path = Nst_char_to_wchar_t(path->value, path->len);
    if (wide_path == nullptr)
        return nullptr;
    WIN32_FILE_ATTRIBUTE_DATA data;
    BOOL result = GetFileAttributesExW(wide_path, GetFileExInfoStandard, &data);
    Nst_free(wide_path);
    if (result == 0) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(FILETIME_to_unix_time(data.ftCreationTime));
#else
    struct stat file_info;
    if (stat(path->value, &file_info) == -1) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(file_info.st_ctime);
#endif // !Nst_MSVC
}

Nst_Obj *NstC time_last_access_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

#ifdef Nst_MSVC
    wchar_t *wide_path = Nst_char_to_wchar_t(path->value, path->len);
    if (wide_path == nullptr)
        return nullptr;
    WIN32_FILE_ATTRIBUTE_DATA data;
    BOOL result = GetFileAttributesExW(wide_path, GetFileExInfoStandard, &data);
    Nst_free(wide_path);
    if (result == 0) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(FILETIME_to_unix_time(data.ftLastAccessTime));
#else
    struct stat file_info;
    if (stat(path->value, &file_info) == -1) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(file_info.st_atime);
#endif // !Nst_MSVC
}

Nst_Obj *NstC time_last_write_(usize arg_num, Nst_Obj **args)
{
    Nst_StrObj *path;
    if (!Nst_extract_args("s", arg_num, args, &path))
        return nullptr;

#ifdef Nst_MSVC
    wchar_t *wide_path = Nst_char_to_wchar_t(path->value, path->len);
    if (wide_path == nullptr)
        return nullptr;
    WIN32_FILE_ATTRIBUTE_DATA data;
    BOOL result = GetFileAttributesExW(wide_path, GetFileExInfoStandard, &data);
    Nst_free(wide_path);
    if (result == 0) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(FILETIME_to_unix_time(data.ftLastWriteTime));
#else
    struct stat file_info;
    if (stat(path->value, &file_info) == -1) {
        throw_c_error();
        return nullptr;
    }
    return Nst_int_new(file_info.st_mtime);
#endif // !Nst_MSVC
}

Nst_Obj *NstC CPO_()
{
    Nst_MapObj *cpo_map = MAP(Nst_map_new());

    Nst_Obj *none_opt            = COPY_OPT(none);
    Nst_Obj *skip_opt            = COPY_OPT(skip_existing);
    Nst_Obj *overwrite_opt       = COPY_OPT(overwrite_existing);
    Nst_Obj *update_opt          = COPY_OPT(update_existing);
    Nst_Obj *recursive_opt       = COPY_OPT(recursive);
    Nst_Obj *copy_symlinks_opt   = COPY_OPT(copy_symlinks);
    Nst_Obj *skip_symlinks_opt   = COPY_OPT(skip_symlinks);
    Nst_Obj *dirs_only_opt       = COPY_OPT(directories_only);
    Nst_Obj *make_symlinks_opt   = COPY_OPT(create_symlinks);
    Nst_Obj *make_hard_links_opt = COPY_OPT(create_hard_links);

    Nst_map_set_str(cpo_map, "none",            none_opt);
    Nst_map_set_str(cpo_map, "skip",            skip_opt);
    Nst_map_set_str(cpo_map, "overwrite",       overwrite_opt);
    Nst_map_set_str(cpo_map, "update",          update_opt);
    Nst_map_set_str(cpo_map, "recursive",       recursive_opt);
    Nst_map_set_str(cpo_map, "copy_symlinks",   copy_symlinks_opt);
    Nst_map_set_str(cpo_map, "skip_symlinks",   skip_symlinks_opt);
    Nst_map_set_str(cpo_map, "dirs_only",       dirs_only_opt);
    Nst_map_set_str(cpo_map, "make_symlinks",   make_symlinks_opt);
    Nst_map_set_str(cpo_map, "make_hard_links", make_hard_links_opt);

    Nst_dec_ref(none_opt);
    Nst_dec_ref(skip_opt);
    Nst_dec_ref(overwrite_opt);
    Nst_dec_ref(update_opt);
    Nst_dec_ref(recursive_opt);
    Nst_dec_ref(copy_symlinks_opt);
    Nst_dec_ref(skip_symlinks_opt);
    Nst_dec_ref(dirs_only_opt);
    Nst_dec_ref(make_symlinks_opt);
    Nst_dec_ref(make_hard_links_opt);

    return OBJ(cpo_map);
}
