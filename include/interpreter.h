/* Bytecode interpreter */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "runtime_stack.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_ExecutionState
{
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    Nst_Int idx; // Instruction index
    Nst_GarbageCollector ggc; // Generational garbage collector
    Nst_StrObj *curr_path; // Current working directory
    Nst_SeqObj *argv;
    i32 opt_level; // Optimization level for importing libraries
    Nst_ValueStack v_stack; // Value stack
    Nst_CallStack  f_stack; // Call stack
    Nst_CatchStack c_stack; // Catch stack
    Nst_LList *loaded_libs; // C imports list
    Nst_LList *lib_paths; // Import stack
    Nst_MapObj *lib_handles; // Maps of the libraries
    Nst_LList *lib_srcs;
}
Nst_ExecutionState;

// Runs the main program, must never be called
EXPORT i32 nst_run(Nst_FuncObj *main_func,
                   i32          argc,
                   i8         **argv,
                   i8          *filename,
                   i32          opt_lvl,
                   bool         no_default);
// Runs an external Nest file, returns -1 on fail and 0 on success
EXPORT i32 nst_run_module(i8 *file_name, Nst_SourceText *lib_src);
// Calls a Nst_FuncObj, it can be both a C function or a bytecode function
EXPORT Nst_Obj *nst_call_func(Nst_FuncObj *func, Nst_Obj **args);
/* Calls a function that has NOT a C body with the given start indexand var table.
The NULL value MUST be added on the stack manually */
EXPORT Nst_Obj *nst_run_func_context(Nst_FuncObj *func,
                                     Nst_Int      idx,
                                     Nst_MapObj  *vars,
                                     Nst_MapObj  *globals);
// Returns the full path of a file
// `file_path` is the relative path of the file
// `buf` is the pointer where the full path is stored, the memory is allocated
// by the function
// `file_part` is the pointer where the filename begins
EXPORT usize nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part);

// Frees all the variables inside nst_state except for loaded_libs
EXPORT void nst_state_free(void);

// Frees loaded_libs, must be called after _nst_streams_del
EXPORT void _nst_unload_libs(void);

// The state of the interpreter
extern Nst_ExecutionState nst_state;

EXPORT Nst_ExecutionState *nst_get_state(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H
