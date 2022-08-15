#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "error.h"
#include "str.h"
#include "llist.h"
#include "var_table.h"
#include "map.h"
#include "compiler.h"
#include "simple_types.h"
#include "runtime_stack.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Traceback *traceback;
    struct _varTable *vt;
    bool error_occurred;
    Nst_StrObj *curr_path;
    Nst_InstructionList *curr_bytecode;
    Nst_ValueStack *v_stack;
    Nst_CallStack *f_stack;
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

void nst_run(Nst_InstructionList *inst_ls, int argc, char **argv);
Nst_MapObj *nst_run_module(char *file_name);
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
size_t nst_get_full_path(char *file_path, char **buf, char **file_part);

extern Nst_ExecutionState *nst_state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H