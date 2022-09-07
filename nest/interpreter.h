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
    Nst_VarTable **vt;
    Nst_Int *idx;
    bool *error_occurred;
    Nst_StrObj **curr_path;
    Nst_SeqObj *argv;
    Nst_ValueStack *v_stack;
    Nst_CallStack *f_stack;
    LList *loaded_libs; // C imports list
    LList *lib_paths; // Import stack
    LList *lib_handles; // Libraries' maps
}
Nst_ExecutionState;

typedef struct
{
    Nst_MapObj *val;
    char *path;
}
Nst_LibHandle;

void nst_run(Nst_FuncObj *main_func, int argc, char **argv);
bool nst_run_module(char *file_name);
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
size_t nst_get_full_path(char *file_path, char **buf, char **file_part);

extern Nst_ExecutionState nst_state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H