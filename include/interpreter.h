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
    int *opt_level; // Optimization level for importing libraries
    Nst_ValueStack *v_stack; // Value stack
    Nst_CallStack  *f_stack; // Call stack
    Nst_CatchStack *c_stack; // Catch stack
    Nst_LList *loaded_libs; // C imports list
    Nst_LList *lib_paths; // Import stack
    Nst_MapObj *lib_handles; // Maps of the libraries
    Nst_LList *lib_srcs;
}
Nst_ExecutionState;

// Runs the main program, must never be called
int nst_run(Nst_FuncObj *main_func,
            int          argc,
            char       **argv,
            char        *filename,
            int          opt_lvl,
            bool         no_default);
// Runs an external Nest file, returns -1 on fail and 0 on success
int nst_run_module(char *file_name, Nst_SourceText *lib_src);
// Calls a Nst_FuncObj, it can be both a C function or a bytecode function
Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args, Nst_OpErr *err);
/* Calls a function that has NOT a C body with the given start indexand var table.
The NULL value MUST be added on the stack manually */
Nst_Obj *nst_run_func_context(Nst_FuncObj *func,
                              Nst_Int      idx,
                              Nst_MapObj  *vars,
                              Nst_MapObj  *globals);
// Returns the full path of a file
// `file_path` is the relative path of the file
// `buf` is the pointer where the full path is stored, the memory is allocated
// by the function
// `file_part` is the pointer where the filename begins
size_t nst_get_full_path(char *file_path, char **buf, char **file_part);

// Frees all the variables inside nst_state
void nst_state_free();

// The state of the interpreter
extern Nst_ExecutionState nst_state;

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H