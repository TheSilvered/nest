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

void nst_run(Nst_Node *node, int argc, char **argv);
Nst_MapObj *nst_run_module(char *file_name);
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
size_t nst_get_full_path(char *file_path, char **buf, char **file_part);

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
Nst_ExecutionState;

typedef struct
{
    Nst_MapObj *val;
    char *path;
}
Nst_LibHandle;

extern Nst_ExecutionState *nst_state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H