#include "framework.h"
#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")
#include "nest_fs.h"
#include <filesystem>

#define FUNC_COUNT 5
static FuncDeclr *func_list_;
static bool lib_init_ = false;

bool lib_init()
{
    if ( (func_list_ = new_func_list(FUNC_COUNT)) == nullptr )
        return false;

    size_t idx = 0;

    func_list_[idx++] = MAKE_FUNCDECLR(isdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(mkdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rmdir, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(isfile, 1);
    func_list_[idx++] = MAKE_FUNCDECLR(rmfile, 1);

    lib_init_ = true;
    return true;
}

FuncDeclr *get_func_ptrs()
{
    return lib_init_ ? func_list_ : nullptr;
}

Nst_Obj *isdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    if ( PathIsDirectoryA(path->value) )
        return inc_ref(nst_true);
    else
        return inc_ref(nst_false);
}

Nst_Obj *mkdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = CreateDirectoryA(path->value, nullptr);

    if ( success || GetLastError() == ERROR_ALREADY_EXISTS )
        return inc_ref(nst_null);
    else
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }
}

Nst_Obj *rmdir(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = RemoveDirectoryA(path->value);

    if ( success )
        return inc_ref(nst_null);
    else if ( GetLastError() == ERROR_PATH_NOT_FOUND )
    {
        SET_VALUE_ERROR("directory path not found");
        return nullptr;
    }
    else if ( GetLastError() == ERROR_ACCESS_DENIED )
    {
        SET_VALUE_ERROR("access denied to remove directory");
        return nullptr;
    }
    else
    {
        SET_VALUE_ERROR("failed to remove the directory");
        return nullptr;
    }
}

Nst_Obj *isfile(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    Nst_iofile *file = fopen(path->value, "rb");
    if ( file == nullptr )
        return inc_ref(nst_false);
    else
        return inc_ref(nst_true);
}

Nst_Obj *rmfile(size_t arg_num, Nst_Obj **args, OpErr *err)
{
    Nst_string *path;

    if ( !extract_arg_values("s", arg_num, args, err, &path) )
        return nullptr;

    bool success = DeleteFileA(path->value);

    if ( success )
        return inc_ref(nst_null);
    else if ( GetLastError() == ERROR_PATH_NOT_FOUND )
    {
        SET_VALUE_ERROR("file not found");
        return nullptr;
    }
    else if ( GetLastError() == ERROR_ACCESS_DENIED )
    {
        SET_VALUE_ERROR("access denied to remove file");
        return nullptr;
    }
    else
    {
        SET_VALUE_ERROR("failed to remove the file");
        return nullptr;
    }
}
