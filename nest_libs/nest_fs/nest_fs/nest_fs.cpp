#include "framework.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "nest_fs.h"
#include <filesystem>

#define FUNC_COUNT 14

namespace fs = std::filesystem;

static Nst_FuncDeclr *func_list_;
static bool lib_init_ = false;

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

Nst_StrObj *heap_str(std::string str)
{
    char *heap_s = new char[str.length() + 1];
    memcpy(heap_s, str.c_str(), str.length());
    heap_s[str.length()] = 0;
    return AS_STR(nst_new_string(heap_s, str.length(), true));
}

NST_FUNC_SIGN(isdir_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    NST_RETURN_COND(PathIsDirectoryA(path->value));
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

    if ( !PathIsDirectoryA(path->value) )
    {
        NST_SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = RemoveDirectoryA(path->value);

    if ( success )
        return nst_new_int(0);
    else
        return nst_new_int(GetLastError());
}

NST_FUNC_SIGN(rmdir_recursive_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    if ( !PathIsDirectoryA(path->value) )
    {
        NST_SET_VALUE_ERROR("directory path not found");
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

    bool success = DeleteFileA(path->value);

    if ( success )
        return nst_new_int(0);
    else if ( GetLastError() == ERROR_FILE_NOT_FOUND )
    {
        NST_SET_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return nst_new_int(GetLastError());
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
        NST_SET_VALUE_ERROR("file not found");
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
        NST_SET_VALUE_ERROR("file not found");
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

    if ( !PathIsDirectoryA(path->value) )
    {
        NST_SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = AS_SEQ(nst_new_vector(0));

    for ( fs::directory_entry const &entry : fs::directory_iterator{ path->value } )
    {
        nst_append_value_vector(vector, (Nst_Obj *)heap_str(entry.path().string()));
    }

    return (Nst_Obj *)vector;
}

NST_FUNC_SIGN(list_dir_recursive_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    if ( !PathIsDirectoryA(path->value) )
    {
        NST_SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = AS_SEQ(nst_new_vector(0));

    for ( fs::directory_entry const &entry : fs::recursive_directory_iterator{ path->value } )
    {
        nst_append_value_vector(vector, (Nst_Obj *)heap_str(entry.path().string()));
    }

    return (Nst_Obj *)vector;
}

NST_FUNC_SIGN(absolute_path_)
{
    Nst_StrObj *path;

    if ( !nst_extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    return (Nst_Obj *)heap_str(fs::absolute(path->value).string());
}

NST_FUNC_SIGN(equivalent_)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    if ( !nst_extract_arg_values("ss", arg_num, args, err, &path_1, &path_2) )
        return nullptr;

    HANDLE f1 = CreateFileA(path_1->value, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( f1 == INVALID_HANDLE_VALUE )
        NST_RETURN_FALSE;
    HANDLE f2 = CreateFileA(path_2->value, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ( f2 == INVALID_HANDLE_VALUE )
    {
        CloseHandle(f1);
        NST_RETURN_FALSE;
    }

    BY_HANDLE_FILE_INFORMATION info1;
    BY_HANDLE_FILE_INFORMATION info2;

    bool are_equal = GetFileInformationByHandle(f1, &info1) &&
        GetFileInformationByHandle(f2, &info2) &&
        info1.dwVolumeSerialNumber == info2.dwVolumeSerialNumber &&
        info1.nFileIndexLow == info2.nFileIndexLow &&
        info1.nFileIndexHigh == info2.nFileIndexHigh;

    CloseHandle(f1);
    CloseHandle(f2);

    NST_RETURN_COND(are_equal);
}

NST_FUNC_SIGN(_get_copy_options_)
{
    Nst_MapObj *options = AS_MAP(nst_new_map());

    nst_map_set_str(options, "none",            nst_new_int((Nst_Int)fs::copy_options::none));
    nst_map_set_str(options, "skip",            nst_new_int((Nst_Int)fs::copy_options::skip_existing));
    nst_map_set_str(options, "overwrite",       nst_new_int((Nst_Int)fs::copy_options::overwrite_existing));
    nst_map_set_str(options, "update",          nst_new_int((Nst_Int)fs::copy_options::update_existing));
    nst_map_set_str(options, "recursive",       nst_new_int((Nst_Int)fs::copy_options::recursive));
    nst_map_set_str(options, "copy_symlinks",   nst_new_int((Nst_Int)fs::copy_options::copy_symlinks));
    nst_map_set_str(options, "skip_symlinks",   nst_new_int((Nst_Int)fs::copy_options::skip_symlinks));
    nst_map_set_str(options, "dirs_only",       nst_new_int((Nst_Int)fs::copy_options::directories_only));
    nst_map_set_str(options, "make_symlinks",   nst_new_int((Nst_Int)fs::copy_options::create_symlinks));
    nst_map_set_str(options, "make_hard_links", nst_new_int((Nst_Int)fs::copy_options::create_hard_links));

    return (Nst_Obj *)options;
}
