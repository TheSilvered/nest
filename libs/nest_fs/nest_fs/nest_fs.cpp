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
#define FUNC_COUNT 14

namespace fs = std::filesystem;

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;
static Nst_MapObj *CPO = nullptr;

bool lib_init()
{
    if ( (func_list_ = nst_new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

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
    func_list_[idx++] = NST_MAKE_FUNCDECLR(equivalent_, 2);
    func_list_[idx++] = NST_MAKE_FUNCDECLR(_get_copy_options_, 0);

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
        nst_dec_ref(CPO);
}

Nst_StrObj *heap_str(std::string str)
{
    char *heap_s = new char[str.length() + 1];
    memcpy(heap_s, str.c_str(), str.length());
    heap_s[str.length()] = 0;
    return STR(nst_new_string(heap_s, str.length(), true));
}

NST_FUNC_SIGN(isdir_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    NST_RETURN_COND(fs::is_directory(path->value));
}

NST_FUNC_SIGN(mkdir_)
{
    Nst_StrObj *path;
    std::error_code ec;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = fs::create_directory(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
        return nst_new_int(0);
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(mkdirs_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    bool success = fs::create_directories(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
        return nst_new_int(0);
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(rmdir_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    if ( !fs::is_directory(path->value) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = fs::remove(path->value);

    if ( success )
        return nst_new_int(0);
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(rmdir_recursive_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    if ( !fs::is_directory(path->value) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = fs::remove_all(path->value, ec);

    if ( success )
        return nst_new_int(0);
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(isfile_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    FILE* file = fopen(path->value, "rb");

    if ( file == nullptr )
        NST_RETURN_FALSE;
    else
    {
        fclose(file);
        NST_RETURN_TRUE;
    }
}

NST_FUNC_SIGN(rmfile_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;
    // if it's not a file
    if ( !fs::is_regular_file(path->value, ec) && !fs::is_other(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }

    bool success = fs::remove(path->value, ec);

    if ( success )
        return nst_new_int(0);
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(copy_)
{
    Nst_StrObj *path_from;
    Nst_StrObj *path_to;
    Nst_Int options;

    if ( !nst_extract_arg_values("ssi", arg_num, args, err, &path_from, &path_to, &options) )
        return nullptr;

    std::error_code ec;

    fs::copy(path_from->value, path_to->value, (fs::copy_options)options, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(rename_)
{
    Nst_StrObj *old_path;
    Nst_StrObj *new_path;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &old_path, &new_path) )
        return nullptr;

    std::error_code ec;

    fs::rename(old_path->value, new_path->value, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        NST_SET_RAW_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return nst_new_int(ec.value());
}

NST_FUNC_SIGN(list_dir_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;
    if ( !fs::is_directory(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = SEQ(nst_new_vector(0));

    for ( fs::directory_entry const &entry : fs::directory_iterator{ path->value } )
    {
        Nst_StrObj *str =  heap_str(entry.path().string());
        nst_append_value_vector(vector, OBJ(str));
        nst_dec_ref(str);
    }

    return OBJ(vector);
}

NST_FUNC_SIGN(list_dir_recursive_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;
    if ( !fs::is_directory(path->value, ec) )
    {
        NST_SET_RAW_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = SEQ(nst_new_vector(0));

    for ( fs::directory_entry const &entry : fs::recursive_directory_iterator{ path->value } )
    {
        Nst_StrObj *str =  heap_str(entry.path().string());
        nst_append_value_vector(vector, OBJ(str));
        nst_dec_ref(str);
    }

    return OBJ(vector);
}

NST_FUNC_SIGN(absolute_path_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    return OBJ(heap_str(fs::absolute(path->value).string()));
}

NST_FUNC_SIGN(equivalent_)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &path_1, &path_2) )
        return nullptr;

    std::error_code ec;
    NST_RETURN_COND(fs::equivalent(path_1->value, path_2->value, ec));
}

NST_FUNC_SIGN(_get_copy_options_)
{
    if ( CPO != nullptr )
        return nst_inc_ref(CPO);

    Nst_MapObj *options = MAP(nst_new_map());

    Nst_Obj *none_opt            = nst_new_int((Nst_Int)fs::copy_options::none);
    Nst_Obj *skip_opt            = nst_new_int((Nst_Int)fs::copy_options::skip_existing);
    Nst_Obj *overwrite_opt       = nst_new_int((Nst_Int)fs::copy_options::overwrite_existing);
    Nst_Obj *update_opt          = nst_new_int((Nst_Int)fs::copy_options::update_existing);
    Nst_Obj *recursive_opt       = nst_new_int((Nst_Int)fs::copy_options::recursive);
    Nst_Obj *copy_symlinks_opt   = nst_new_int((Nst_Int)fs::copy_options::copy_symlinks);
    Nst_Obj *skip_symlinks_opt   = nst_new_int((Nst_Int)fs::copy_options::skip_symlinks);
    Nst_Obj *dirs_only_opt       = nst_new_int((Nst_Int)fs::copy_options::directories_only);
    Nst_Obj *make_symlinks_opt   = nst_new_int((Nst_Int)fs::copy_options::create_symlinks);
    Nst_Obj *make_hard_links_opt = nst_new_int((Nst_Int)fs::copy_options::create_hard_links);

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
