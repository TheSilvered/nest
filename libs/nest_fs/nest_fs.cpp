#if defined(_WIN32) || defined(WIN32)
#include <winerror.h>
#else
// This error does not get thrown on UNIX
#define ERROR_ALREADY_EXISTS 0
// ERROR_PATH_NOT_FOUND and ERROR_FILE_NOT_FOUND are the same on UNIX
#define ERROR_PATH_NOT_FOUND ENOENT
#endif

#include "nest_fs.h"

#include <filesystem>
#include <cerrno>
#include <cstring>

#define FUNC_COUNT 20

namespace fs = std::filesystem;

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_MapObj *CPO = nullptr;

bool lib_init()
{
    if ( (func_list_ = nst_func_list_new(FUNC_COUNT)) == nullptr )
    {
        return false;
    }

    usize idx = 0;

    func_list_[idx++] = NST_MAKE_FUNCDECLR(isdir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(mkdir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(mkdirs_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rmdir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rmdir_recursive_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(isfile_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rmfile_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(copy_, 3);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(rename_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(list_dir_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(list_dir_recursive_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(absolute_path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(canonical_path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(relative_path_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(equivalent_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(join_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(path_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(filename_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(extension_, 1);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_copy_options_, 0);

#if __LINE__ - FUNC_COUNT != 34
#error FUNC_COUNT does not match the number of lines
#endif

    lib_init_ = true;
    return true;
}

Nst_FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

void free_lib()
{
    if ( CPO != nullptr )
    {
        nst_dec_ref(CPO);
    }
}

Nst_StrObj *heap_str(std::string str)
{
    i8 *heap_s = new i8[str.length() + 1];
    memcpy(heap_s, str.c_str(), str.length());
    heap_s[str.length()] = 0;
    return STR(nst_string_new(heap_s, str.length(), true));
}

NST_FUNC_SIGN(isdir_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    NST_RETURN_COND(fs::is_directory(path->value));
}

NST_FUNC_SIGN(mkdir_)
{
    Nst_StrObj *path;
    std::error_code ec;

    NST_DEF_EXTRACT("s", &path);

    bool success = fs::create_directory(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
    {
        NST_RETURN_ZERO;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(mkdirs_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    bool success = fs::create_directories(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
    {
        NST_RETURN_ZERO;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(rmdir_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    if ( !fs::is_directory(path->value) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = fs::remove(path->value, ec);

    if ( success )
    {
        NST_RETURN_ZERO;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(rmdir_recursive_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;

    if ( !fs::is_directory(path->value) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = fs::remove_all(path->value, ec);

    if ( success )
    {
        NST_RETURN_ZERO;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(isfile_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    FILE* file = fopen(path->value, "rb");

    if ( file == nullptr )
    {
        NST_RETURN_FALSE;
    }
    else
    {
        fclose(file);
        NST_RETURN_TRUE;
    }
}

NST_FUNC_SIGN(rmfile_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    // if it's not a file
    if ( !fs::is_regular_file(path->value, ec) &&
         !fs::is_other(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }

    bool success = fs::remove(path->value, ec);

    if ( success )
    {
        NST_RETURN_ZERO;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(copy_)
{
    Nst_StrObj *path_from;
    Nst_StrObj *path_to;
    Nst_Obj *options;

    NST_DEF_EXTRACT("ss?i", &path_from, &path_to, &options);

    fs::copy_options cp_options;
    NST_SET_DEF(
        options,
        cp_options,
        fs::copy_options::none,
        (fs::copy_options)AS_INT(options));

    std::error_code ec;

    fs::copy(path_from->value, path_to->value, cp_options, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(rename_)
{
    Nst_StrObj *old_path;
    Nst_StrObj *new_path;

    NST_DEF_EXTRACT("ss", &old_path, &new_path);

    std::error_code ec;

    fs::rename(old_path->value, new_path->value, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
    {
        return nst_int_new(ec.value());
    }
}

NST_FUNC_SIGN(list_dir_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    if ( !fs::is_directory(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = SEQ(nst_vector_new(0));

    for ( fs::directory_entry const &entry
        : fs::directory_iterator{ path->value } )
    {
        Nst_StrObj *str =  heap_str(entry.path().string());
        nst_vector_append(vector, OBJ(str));
        nst_dec_ref(str);
    }

    return OBJ(vector);
}

NST_FUNC_SIGN(list_dir_recursive_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    std::error_code ec;
    if ( !fs::is_directory(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = SEQ(nst_vector_new(0));

    for ( fs::directory_entry const &entry
        : fs::recursive_directory_iterator{ path->value } )
    {
        Nst_StrObj *str =  heap_str(entry.path().string());
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
    fs::path result = fs::absolute(path->value, ec);

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
    fs::path result = fs::canonical(path->value, ec);

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
    fs::path result = fs::relative(path->value, base->value, ec);

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
    NST_RETURN_COND(fs::equivalent(path_1->value, path_2->value, ec));
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

    i8 *new_str = new i8[new_len + 1];
    memcpy(new_str, p1, p1_len);

    if ( add_slash )
    {
#if defined(_WIN32) || defined(WIN32)
        new_str[p1_len] = '\\';
#else
        new_str[p1_len] = '/';
#endif
    }
    memcpy(new_str + p1_len + (add_slash ? 1 : 0), p2, p2_len);
    new_str[new_len] = 0;

    for ( usize i = 0; i < new_len; i++ )
    {
#if defined(_WIN32) || defined(WIN32)
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

NST_FUNC_SIGN(path_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(fs::path(path->value).parent_path().string()));
}

NST_FUNC_SIGN(filename_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(fs::path(path->value).filename().string()));
}

NST_FUNC_SIGN(extension_)
{
    Nst_StrObj *path;

    NST_DEF_EXTRACT("s", &path);

    return OBJ(heap_str(fs::path(path->value).extension().string()));
}

NST_FUNC_SIGN(_get_copy_options_)
{
    if ( CPO != nullptr )
    {
        return nst_inc_ref(CPO);
    }

    Nst_MapObj *options = MAP(nst_map_new());

    Nst_Obj *none_opt =
        nst_int_new((Nst_Int)fs::copy_options::none);
    Nst_Obj *skip_opt =
        nst_int_new((Nst_Int)fs::copy_options::skip_existing);
    Nst_Obj *overwrite_opt =
        nst_int_new((Nst_Int)fs::copy_options::overwrite_existing);
    Nst_Obj *update_opt =
        nst_int_new((Nst_Int)fs::copy_options::update_existing);
    Nst_Obj *recursive_opt =
        nst_int_new((Nst_Int)fs::copy_options::recursive);
    Nst_Obj *copy_symlinks_opt =
        nst_int_new((Nst_Int)fs::copy_options::copy_symlinks);
    Nst_Obj *skip_symlinks_opt =
        nst_int_new((Nst_Int)fs::copy_options::skip_symlinks);
    Nst_Obj *dirs_only_opt =
        nst_int_new((Nst_Int)fs::copy_options::directories_only);
    Nst_Obj *make_symlinks_opt =
        nst_int_new((Nst_Int)fs::copy_options::create_symlinks);
    Nst_Obj *make_hard_links_opt =
        nst_int_new((Nst_Int)fs::copy_options::create_hard_links);

    nst_map_set_str(options, "none",            none_opt);
    nst_map_set_str(options, "skip",            skip_opt);
    nst_map_set_str(options, "overwrite",       overwrite_opt);
    nst_map_set_str(options, "update",          update_opt);
    nst_map_set_str(options, "recursive",       recursive_opt);
    nst_map_set_str(options, "copy_symlinks",   copy_symlinks_opt);
    nst_map_set_str(options, "skip_symlinks",   skip_symlinks_opt);
    nst_map_set_str(options, "dirs_only",       dirs_only_opt);
    nst_map_set_str(options, "make_symlinks",   make_symlinks_opt);
    nst_map_set_str(options, "make_hard_links", make_hard_links_opt);

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

    CPO = options;

    return nst_inc_ref(options);
}
