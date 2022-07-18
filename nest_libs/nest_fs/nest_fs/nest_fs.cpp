#include "framework.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "nest_fs.h"
#include <filesystem>

#define FUNC_COUNT 14

namespace fs = std::filesystem;

static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(isdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(mkdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(mkdirs, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rmdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rmdir_recursive, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(isfile, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rmfile, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(copy, 3);
    func_list_[idx++] = MAKE_NAMED_FUNCDECLR(rename_file, 2, "rename");
    func_list_[idx++] = MAKE_FUNCDECLR(list_dir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(list_dir_recursive, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(absolute_path, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(equivalent, 2);
    func_list_[idx++] = MAKE_FUNCDECLR(_get_copy_options, 0);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_StrObj *heap_str(std::string str)
{
    char *heap_s = new char[str.length() + 1];
    memcpy(heap_s, str.c_str(), str.length());
    heap_s[str.length()] = 0;
    return AS_STR(new_string(heap_s, str.length(), true));
}

Nst_Obj *isdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    RETURN_COND(PathIsDirectoryA(path->value));
}

Nst_Obj *mkdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;
    std::error_code ec;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = fs::create_directory(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
        return new_int(0);
    else
        return new_int(ec.value());
}

Nst_Obj *mkdirs(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    bool success = fs::create_directories(path->value, ec);

    if ( success || ec.value() == ERROR_ALREADY_EXISTS )
        return new_int(0);
    else
        return new_int(ec.value());
}

Nst_Obj *rmdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    if ( !PathIsDirectoryA(path->value) )
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = RemoveDirectoryA(path->value);

    if ( success )
        return new_int(0);
    else
        return new_int(GetLastError());
}

Nst_Obj *rmdir_recursive(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    if ( !PathIsDirectoryA(path->value) )
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    bool success = fs::remove_all(path->value, ec);

    if ( success )
        return new_int(0);
    else
        return new_int(ec.value());
}

Nst_Obj *isfile(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    Nst_iofile file = fopen(path->value, "rb");

    if ( file == nullptr )
        RETURN_FALSE;
    else
    {
        fclose(file);
        RETURN_TRUE;
    }
}

Nst_Obj *rmfile(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = DeleteFileA(path->value);

    if ( success )
        return new_int(0);
    else if ( GetLastError() == ERROR_FILE_NOT_FOUND )
    {
        SET_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return new_int(GetLastError());
}

Nst_Obj *copy(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path_from;
    Nst_StrObj *path_to;
    Nst_int options;

    if ( !extract_arg_values("ssi", arg_num, args, err, &path_from, &path_to, &options) )
        return nullptr;

    std::error_code ec;

    fs::copy(path_from->value, path_to->value, (fs::copy_options)options, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        SET_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return new_int(ec.value());
}

Nst_Obj *rename_file(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *old_path;
    Nst_StrObj *new_path;

    if ( !extract_arg_values("ss", arg_num, args, err, &old_path, &new_path) )
        return nullptr;

    std::error_code ec;

    fs::rename(old_path->value, new_path->value, ec);

    if ( ec.value() == ERROR_PATH_NOT_FOUND )
    {
        SET_VALUE_ERROR("file not found");
        return nullptr;
    }
    else
        return new_int(ec.value());
}

Nst_Obj *list_dir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    if ( !PathIsDirectoryA(path->value) )
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = AS_SEQ(new_vector(0));

    for ( fs::directory_entry const &entry : fs::directory_iterator{ path->value } )
    {
        append_value_vector(vector, (Nst_Obj *)heap_str(entry.path().string()));
    }

    return (Nst_Obj *)vector;
}

Nst_Obj *list_dir_recursive(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    if ( !PathIsDirectoryA(path->value) )
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }

    Nst_SeqObj *vector = AS_SEQ(new_vector(0));

    for ( fs::directory_entry const &entry : fs::recursive_directory_iterator{ path->value } )
    {
        append_value_vector(vector, (Nst_Obj *)heap_str(entry.path().string()));
    }

    return (Nst_Obj *)vector;
}

Nst_Obj *absolute_path(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    std::error_code ec;

    return (Nst_Obj *)heap_str(fs::absolute(path->value).string());
}

Nst_Obj *equivalent(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_StrObj *path_1;
    Nst_StrObj *path_2;

    if ( !extract_arg_values("ss", arg_num, args, err, &path_1, &path_2) )
        return nullptr;

    HANDLE f1 = CreateFileA(path_1->value, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if ( f1 == INVALID_HANDLE_VALUE )
        RETURN_FALSE;
    HANDLE f2 = CreateFileA(path_2->value, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if ( f2 == INVALID_HANDLE_VALUE )
    {
        CloseHandle(f1);
        RETURN_FALSE;
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

    RETURN_COND(are_equal);
}

Nst_Obj *_get_copy_options(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_MapObj *options = new_map();

    map_set_str(options, "none", new_int((Nst_int)fs::copy_options::none));
    map_set_str(options, "skip", new_int((Nst_int)fs::copy_options::skip_existing));
    map_set_str(options, "overwrite", new_int((Nst_int)fs::copy_options::overwrite_existing));
    map_set_str(options, "update", new_int((Nst_int)fs::copy_options::update_existing));
    map_set_str(options, "recursive", new_int((Nst_int)fs::copy_options::recursive));
    map_set_str(options, "copy_symlinks", new_int((Nst_int)fs::copy_options::copy_symlinks));
    map_set_str(options, "skip_symlinks", new_int((Nst_int)fs::copy_options::skip_symlinks));
    map_set_str(options, "dirs_only", new_int((Nst_int)fs::copy_options::directories_only));
    map_set_str(options, "make_symlinks", new_int((Nst_int)fs::copy_options::create_symlinks));
    map_set_str(options, "make_hard_links", new_int((Nst_int)fs::copy_options::create_hard_links));

    return (Nst_Obj *)options;
}
