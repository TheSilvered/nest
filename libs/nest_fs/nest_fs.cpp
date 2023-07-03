#include "nest_fs.h"

#ifdef WINDOWS
#include <winerror.h>
#else
// This error does not get thrown on UNIX
#define ERROR_ALREADY_EXISTS 0
// ERROR_PATH_NOT_FOUND and ERROR_FILE_NOT_FOUND are the same on UNIX
#define ERROR_PATH_NOT_FOUND ENOENT
#endif

#include <filesystem>
#include <cerrno>
#include <cstring>

#define FUNC_COUNT 32

namespace fs = std::filesystem;

static Nst_ObjDeclr func_list_[FUNC_COUNT];
static Nst_DeclrList obj_list_ = { func_list_, FUNC_COUNT };
static bool lib_init_ = false;
static Nst_MapObj *CPO_ = nullptr;

bool lib_init()
{
    CPO_ = MAP(nst_map_new());
    Nst_Obj *none_opt            = nst_int_new((Nst_Int)fs::copy_options::none);
    Nst_Obj *skip_opt            = nst_int_new((Nst_Int)fs::copy_options::skip_existing);
    Nst_Obj *overwrite_opt       = nst_int_new((Nst_Int)fs::copy_options::overwrite_existing);
    Nst_Obj *update_opt          = nst_int_new((Nst_Int)fs::copy_options::update_existing);
    Nst_Obj *recursive_opt       = nst_int_new((Nst_Int)fs::copy_options::recursive);
    Nst_Obj *copy_symlinks_opt   = nst_int_new((Nst_Int)fs::copy_options::copy_symlinks);
    Nst_Obj *skip_symlinks_opt   = nst_int_new((Nst_Int)fs::copy_options::skip_symlinks);
    Nst_Obj *dirs_only_opt       = nst_int_new((Nst_Int)fs::copy_options::directories_only);
    Nst_Obj *make_symlinks_opt   = nst_int_new((Nst_Int)fs::copy_options::create_symlinks);
    Nst_Obj *make_hard_links_opt = nst_int_new((Nst_Int)fs::copy_options::create_hard_links);

    nst_map_set_str(CPO_, "none",            none_opt);
    nst_map_set_str(CPO_, "skip",            skip_opt);
    nst_map_set_str(CPO_, "overwrite",       overwrite_opt);
    nst_map_set_str(CPO_, "update",          update_opt);
    nst_map_set_str(CPO_, "recursive",       recursive_opt);
    nst_map_set_str(CPO_, "copy_symlinks",   copy_symlinks_opt);
    nst_map_set_str(CPO_, "skip_symlinks",   skip_symlinks_opt);
    nst_map_set_str(CPO_, "dirs_only",       dirs_only_opt);
    nst_map_set_str(CPO_, "make_symlinks",   make_symlinks_opt);
    nst_map_set_str(CPO_, "make_hard_links", make_hard_links_opt);

    nst_dec_ref(none_opt);
    nst_dec_ref(skip_opt);
    nst_dec_ref(overwrite_opt);
    nst_dec_ref(update_opt);
    nst_dec_ref(recursive_opt);
    nst_dec_ref(copy_symlinks_opt);
    nst_dec_ref(skip_symlinks_opt);
    nst_dec_ref(dirs_only_opt);
    nst_dec_ref(make_symlinks_opt);
    nst_dec_ref(make_hard_links_opt);

    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_dir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_file_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_symlink_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_socket_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_block_device_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_char_device_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(is_named_pipe_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(make_dir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(make_dirs_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(remove_dir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(remove_dirs_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(remove_file_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(make_dir_symlink_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(make_file_symlink_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(read_symlink_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(make_hard_link_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(exists_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(copy_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rename_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(list_dir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(list_dirs_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(absolute_path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(canonical_path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(relative_path_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(equivalent_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(join_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(normalize_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(parent_path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(filename_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(extension_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_copy_options_, 0);
    func_list_[idx++] = NST_MAKE_OBJDECLR(CPO_);

#if __LINE__ - FUNC_COUNT != 64
#error
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
    nst_dec_ref(CPO_);
}

static Nst_StrObj *heap_str(const i8 *str, usize len)
{
    i8 *heap_s = nst_malloc_c(len + 1, i8);
    if ( heap_s == nullptr )
    {
        return nullptr;
    }

    memcpy(heap_s, str, len);
    heap_s[len] = 0;
    return STR(nst_string_new(heap_s, len, true));
}

static Nst_StrObj *heap_str(std::string str)
{
    return heap_str(str.c_str(), str.length());
}

static Nst_StrObj *heap_str(fs::path path)
{
    std::u8string str = path.u8string();
    return heap_str((const i8 *)str.c_str(), str.length());
}

static Nst_Obj *throw_system_error(std::error_code ec)
{
    nst_set_error(
        nst_sprintf("System Error %d", ec.value()),
        heap_str(ec.message()));
    return NULL;
}

static fs::path utf8_path(Nst_StrObj *str)
{
    std::u8string s { (char8_t *)str->value };
    return fs::path(s);
}

bool check_path(Nst_StrObj *path, bool (*func)(const fs::path&, std::error_code&))
{
    std::error_code ec;
    bool check = func(utf8_path(path), ec);
    ec.clear();
    return check;
}

NST_FUNC_SIGN(is_dir_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_directory));
}

NST_FUNC_SIGN(is_file_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_regular_file));
}

NST_FUNC_SIGN(is_symlink_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_symlink));
}

NST_FUNC_SIGN(is_socket_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_socket));
}

NST_FUNC_SIGN(is_block_device_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_block_file));
}

NST_FUNC_SIGN(is_char_device_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_character_file));
}

NST_FUNC_SIGN(is_named_pipe_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::is_fifo));
}

NST_FUNC_SIGN(make_dir_)
{
    Nst_StrObj *path;
    std::error_code ec;

    NST_DEF_EXTRACT("s", &path);

    bool success = fs::create_directory(utf8_path(path), ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS || ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(make_dirs_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    bool success = fs::create_directories(utf8_path(path), ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS || ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(remove_dir_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    if ( !fs::is_directory(utf8_path(path)) )
    {
        nst_set_value_error(nst_sprintf(
            "directory '%.4096s' not found",
            path->value));
        return nullptr;
    }

    bool success = fs::remove(utf8_path(path), ec);

    if ( success || ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(remove_dirs_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    if ( !fs::is_directory(utf8_path(path)) )
    {
        nst_set_value_error(nst_sprintf(
            "directory '%.4096s' not found",
            path->value));
        return nullptr;
    }

    bool success = fs::remove_all(utf8_path(path), ec);

    if ( success || ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(remove_file_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    if ( !check_path(path, fs::exists) || check_path(path, fs::is_directory) )
    {
        nst_set_value_error(nst_sprintf(
            "file '%.4096s' not found",
            path->value));
        return nullptr;
    }

    bool success = fs::remove(path->value, ec);

    if ( success || ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(make_dir_symlink_)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    NST_DEF_EXTRACT("ss", &target, &link);

    fs::create_directory_symlink(utf8_path(target), utf8_path(link), ec);
    if ( ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(make_file_symlink_)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    NST_DEF_EXTRACT("ss", &target, &link);

    fs::create_symlink(utf8_path(target), utf8_path(link), ec);
    if ( ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(read_symlink_)
{
    Nst_StrObj *path;
    std::error_code ec;

    NST_DEF_EXTRACT("s", &path);

    fs::path result = fs::read_symlink(utf8_path(path), ec);
    if ( ec.value() == 0 )
    {
        return OBJ(heap_str(result));
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(make_hard_link_)
{
    Nst_StrObj *target;
    Nst_StrObj *link;
    std::error_code ec;

    NST_DEF_EXTRACT("ss", &target, &link);

    fs::create_hard_link(utf8_path(target), utf8_path(link), ec);
    if ( ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(exists_)
{
    Nst_StrObj *path;
    NST_DEF_EXTRACT("s", &path);
    NST_RETURN_COND(check_path(path, fs::exists));
}

NST_FUNC_SIGN(copy_)
{
    Nst_StrObj *path_from;
    Nst_StrObj *path_to;
    Nst_Obj *options;

    NST_DEF_EXTRACT("ss?i", &path_from, &path_to, &options);

    fs::copy_options cp_options = NST_DEF_VAL(
        options,
        (fs::copy_options)AS_INT(options),
        fs::copy_options::none);

    std::error_code ec;
    fs::copy(utf8_path(path_from), utf8_path(path_to), cp_options, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        nst_set_value_error(nst_sprintf(
            "file '%.100s' or directory '%.4096s' not found",
            path_from->value,
            path_to->value));
        return nullptr;
    }
    else if ( ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(rename_)
{
    Nst_StrObj *old_path;
    Nst_StrObj *new_path;

    NST_DEF_EXTRACT("ss", &old_path, &new_path);

    std::error_code ec;

    fs::rename(utf8_path(old_path), utf8_path(new_path), ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        nst_set_value_error(nst_sprintf(
            "file '%.100s' or directory '%.4096s' not found",
            old_path->value,
            new_path->value));
        return nullptr;
    }
    else if ( ec.value() == 0 )
    {
        NST_RETURN_NULL;
    }
    else
    {
        return throw_system_error(ec);
    }
}

NST_FUNC_SIGN(list_dir_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    if ( !fs::is_directory(utf8_path(path), ec) )
    {
        nst_set_value_error(nst_sprintf(
            "directory '%.4096s' not found",
            path->value));
        return nullptr;
    }
    if ( ec.value() != 0 )
    {
        return throw_system_error(ec);
    }

    Nst_SeqObj *vector = SEQ(nst_vector_new(0));

    for ( fs::directory_entry const &entry
        : fs::directory_iterator{ path->value } )
    {
        Nst_StrObj *str = heap_str(entry.path());
        if ( str == nullptr )
        {
            return nullptr;
        }
        nst_vector_append(vector, OBJ(str));
        nst_dec_ref(str);
    }

    return OBJ(vector);
}

NST_FUNC_SIGN(list_dirs_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    if ( !fs::is_directory(utf8_path(path), ec) )
    {
        nst_set_value_error(nst_sprintf(
            "directory '%.4096s' not found",
            path->value));
        return nullptr;
    }
    if ( ec.value() != 0 )
    {
        return throw_system_error(ec);
    }

    Nst_SeqObj *vector = SEQ(nst_vector_new(0));

    for ( fs::directory_entry const &entry
        : fs::recursive_directory_iterator{ path->value } )
    {
        Nst_StrObj *str =  heap_str(entry.path());
        if ( str == nullptr )
        {
            return nullptr;
        }
        nst_vector_append(vector, OBJ(str));
        nst_dec_ref(str);
    }

    return OBJ(vector);
}

NST_FUNC_SIGN(absolute_path_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    fs::path result = fs::absolute(utf8_path(path), ec);

    if ( ec.value() == 0 )
    {
        return OBJ(heap_str(result.string()));
    }
    else
    {
        NST_RETURN_NULL;
    }
}

NST_FUNC_SIGN(canonical_path_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    fs::path result = fs::canonical(utf8_path(path), ec);

    if ( ec.value() == 0 )
    {
        return OBJ(heap_str(result.string()));
    }
    else
    {
        NST_RETURN_NULL;
    }
}

NST_FUNC_SIGN(relative_path_)
{
    Nst_StrObj *path;
    Nst_StrObj *base;

    NST_DEF_EXTRACT("ss", &path, &base);

    std::error_code ec;
    fs::path result = fs::relative(utf8_path(path), utf8_path(base), ec);

    if ( ec.value() == 0 )
    {
        return OBJ(heap_str(result.string()));
    }
    else
    {
        NST_RETURN_NULL;
    }
}

NST_FUNC_SIGN(equivalent_)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    NST_DEF_EXTRACT("ss", &path_1, &path_2);

    std::error_code ec;
    NST_RETURN_COND(fs::equivalent(utf8_path(path_1), utf8_path(path_2), ec));
}

NST_FUNC_SIGN(join_)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    NST_DEF_EXTRACT("ss", &path_1, &path_2);

    i8 *p1 = path_1->value;
    i8 *p2 = path_2->value;
    usize p1_len = path_1->len;
    usize p2_len = path_2->len;

    if ( p2_len == 0 )
    {
        return nst_inc_ref(path_1);
    }

    if ( p2[0] == '/' || p2[0] == '\\' || p2[1] == ':' )
    {
        return nst_inc_ref(path_2);
    }

    usize new_len = p1_len + p2_len;
    bool add_slash = false;

    if ( p1[p1_len - 1] != '/' && p1[p1_len - 1] != '\\' )
    {
        new_len++;
        add_slash = true;
    }

    i8 *new_str = nst_malloc_c(new_len + 1, i8);
    if ( new_str == nullptr )
    {
        nst_failed_allocation();
        return nullptr;
    }

    memcpy(new_str, p1, p1_len);

    if ( add_slash )
    {
#ifdef WINDOWS
        new_str[p1_len] = '\\';
#else
        new_str[p1_len] = '/';
#endif
    }
    memcpy(new_str + p1_len + (add_slash ? 1 : 0), p2, p2_len);
    new_str[new_len] = 0;

    for ( usize i = 0; i < new_len; i++ )
    {
#ifdef WINDOWS
        if ( new_str[i] == '/' )
        {
            new_str[i] = '\\';
        }
#else
        if ( new_str[i] == '\\' )
        {
            new_str[i] = '/';
        }
#endif
    }

    return nst_string_new(new_str, new_len, true);
}

NST_FUNC_SIGN(normalize_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);
    Nst_StrObj *norm_path = STR(nst_string_copy(path));
    if ( norm_path == nullptr )
    {
        return nullptr;
    }

    i8 *val = norm_path->value;

    for ( usize i = 0, n = norm_path->len; i < n; i++ )
    {
#ifdef WINDOWS
        if ( val[i] == '/' )
        {
            val[i] = '\\';
        }
#else
        if ( val[i] == '\\' )
        {
            val[i] = '/';
        }
#endif
    }
    return OBJ(norm_path);
}

NST_FUNC_SIGN(parent_path_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(utf8_path(path).parent_path()));
}

NST_FUNC_SIGN(filename_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(utf8_path(path).filename()));
}

NST_FUNC_SIGN(extension_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(utf8_path(path).extension()));
}

NST_FUNC_SIGN(_get_copy_options_)
{
    return nst_inc_ref(CPO_);
}
