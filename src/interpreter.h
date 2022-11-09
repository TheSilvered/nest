/* Bytecode interpreter */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "error.h"
#include "str.h"
#include "map.h"
#include "simple_types.h"
#include "llist.h"
#include "var_table.h"
#include "compiler.h"
#include "runtime_stack.h"
#include "ggc.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_ExecutionState
{
    Nst_Traceback *traceback;
    Nst_VarTable **vt;
    Nst_Int *idx; // Instruction index
    Nst_GarbageCollector *ggc; // Generational garbage collector
    Nst_StrObj **curr_path; // Current working directory
    Nst_SeqObj *argv;
    Nst_ValueStack *v_stack; // Value stack
    Nst_CallStack *f_stack; // Call stack
    LList *loaded_libs; // C imports list
    LList *lib_paths; // Import stack
    LList *lib_handles; // Maps of the libraries
}
Nst_ExecutionState;

typedef struct _Nst_LibHandle
{
    Nst_MapObj *val;
    char *path;
    Nst_SourceText *text;
}
Nst_LibHandle;

// Runs the main program, must never be called
void nst_run(Nst_FuncObj *main_func, int argc, char **argv, char *filename, int opt_level);
// Runs an external Nest file, returns -1 on fail and 0 on success
int nst_run_module(char *file_name, Nst_SourceText *lib_src);
// Calls a Nst_FuncObj, it can be both a C function or a bytecode function
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
/* Returns the full path of a file
`file_path` is the relative path of the file
`buf` is the pointer where the full path is stored, the memory is allocated by the function
`file_part` is the pointer where the filename begins */
size_t nst_get_full_path(char *file_path, char **buf, char **file_part);
// Destoys a Nst_LibHandle
void nst_destroy_lib_handle(Nst_LibHandle *handle);

// The state of the interpreter
extern Nst_ExecutionState nst_state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H