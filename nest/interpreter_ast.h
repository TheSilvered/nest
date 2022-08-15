#ifndef INTERPRETER_AST_H
#define INTERPRETER_AST_H

#include <stdbool.h>
#include <windows.h>
#include "error.h"
#include "obj.h"
#include "nodes.h"
#include "var_table.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

void _nst_run_old(Nst_Node *node, int argc, char **argv);
Nst_MapObj *_nst_run_module_old(char *file_name);
Nst_Obj *_nst_call_func_old(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
size_t _nst_get_full_path_old(char *file_path, char **buf, char **file_part);

typedef struct
{
    Nst_Traceback *traceback;
    Nst_Obj *value;
    struct _varTable *vt;
    bool error_occurred;
    bool must_return;
    bool must_continue;
    bool must_break;
    Nst_StrObj *curr_path;
    LList *loaded_libs;
    LList *lib_paths;
    LList *lib_handles;
}
_Nst_OldExecutionState;

typedef struct
{
    Nst_MapObj *val;
    char *path;
}
_Nst_OldLibHandle;

extern _Nst_OldExecutionState *_nst_state_old;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_AST_H