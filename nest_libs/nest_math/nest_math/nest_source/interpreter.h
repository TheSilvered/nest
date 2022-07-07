#ifndef INTERPRETER_H
#define INTERPRETER_H

#include <stdbool.h>
#include <windows.h>
#include "error.h"
#include "obj.h"
#include "nodes.h"
#include "var_table.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void run(Node *node, int argc, char **argv);
Nst_map *run_module(char *file_name);
Nst_Obj *call_func(Nst_func *func, Nst_Obj **args, OpErr *err);
size_t get_full_path(char *file_path, char **buf, char **file_part);

typedef struct
{
    Nst_Traceback *traceback;
    Nst_Obj *value;
    VarTable *vt;
    bool error_occurred;
    bool must_return;
    bool must_continue;
    bool must_break;
    Nst_string *curr_path;
    LList *loaded_libs;
    LList *lib_paths;
    LList *lib_handles;
}
ExecutionState;

typedef union
{
    Node *ast;
    HMODULE dll;
}
LibHandleVal;

typedef struct
{
    LibHandleVal *val;
    char *path;
}
LibHandle;

extern ExecutionState *state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H