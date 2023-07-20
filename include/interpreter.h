/**
 * @file interpreter.h
 *
 * @brief Bytecode interpreter
 *
 * @author TheSilvered
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "runtime_stack.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/** Global execution state of Nest.
 *
 * @param traceback: traceback of the current running program
 * @param vt: current variable table
 * @param idx: current instruction index
 * @param ggc: generational garbage collector
 * @param curr_path: current working directory
 * @param argv: arguments passed to the program
 * @param opt_level: maximum optimization level when importing libraries
 * @param v_stack: value stack
 * @param f_stack: call stack
 * @param c_stack: catch stack
 * @param loaded_libs: dynamic library handles
 * @param lib_paths: import stack
 * @param lib_handles: maps of the imported libraries
 * @param lib_srcs: sources of the imported Nest libraries
 */
NstEXP typedef struct _Nst_ExecutionState {
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    Nst_Int idx;
    Nst_GarbageCollector ggc;
    Nst_StrObj *curr_path;
    Nst_SeqObj *argv;
    i32 opt_level;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
    Nst_LList *loaded_libs;
    Nst_LList *lib_paths;
    Nst_MapObj *lib_handles;
    Nst_LList *lib_srcs;
} Nst_ExecutionState;

/** Runs the main program.
 *
 * @brief Must never be called inside a library.
 *
 * @param main_func: the function object of the main program
 * @param argc: the command line argument count
 * @param argv: the command line arguments
 * @param filename: the name of the file of the main program
 * @param opt_level: the maximum optimization level
 * @param no_default: whether to initialize the variable table of the main
 * program with built-in values
 *
 * @return The exit code of the program.
 */
NstEXP i32 NstC Nst_run(Nst_FuncObj *main_func, i32 argc, i8 **argv,
                        i8 *filename, i32 opt_lvl, bool no_default);
/** Runs an external Nest file.
 *
 * @brief If the function succeedes, the result of the module is on top of the
 * value stack.
 *
 * @param file_name: the name of the file to run, must exist since no checking
 * is done
 * @param lib_src: the pointer where to store the source of the file, if an
 * error occurs the source of the library is expected to be on the lib_srcs
 * list of the global state
 *
 * @return -1 on failure and 0 on success.
 */
NstEXP i32 NstC Nst_run_module(i8 *file_name, Nst_SourceText *lib_src);
/** Calls a Nst_FuncObj.
 * @brief It can have both a Nest or C body. No checking is done on the number
 * of arguments.
 * 
 * @param func: the function to call
 * @param args: the array of arguments to pass to it, the correct number of
 * arguments must be ginven, no null arguments are added
 * 
 * @return The result of the function or NULL on failure. When a function with
 * a Nest body is called the error may not be set. When a function with a C
 * body is called, the error is always set.
 */
NstEXP Nst_Obj *NstC Nst_call_func(Nst_FuncObj *func, Nst_Obj **args);

/** Executes the body of a Nst_FuncObj that has a Nest body using a given
 * context.
 * 
 * @brief The context is set according to the arguments passed.
 * 
 * @param func: the function to execute
 * @param idx: the instruction index from which to start the execution of the
 * body
 * @param vars: the local variable table
 * @param globals: the global variable table, it may be NULL, in which case it
 * is determined automatically
 * 
 * @return The result of the function or NULL on failure. The error may not be
 * set.
 */
NstEXP Nst_Obj *NstC Nst_run_func_context(Nst_FuncObj *func, Nst_Int idx,
                                          Nst_MapObj *vars,
                                          Nst_MapObj *globals);
/** Returns the absolute path to a file system object.
 * 
 * @brief The absolute path is allocated on the heap.
 * 
 * @param file_path: the relative path to the object
 * @param buf: the buf where the absolute path is placed
 * @param file_part: where the start of the file name inside the file path is
 * put, this may be NULL in which case it is ignored
 * 
 * @return The length in bytes of the absolute path.
 */
NstEXP usize NstC Nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part);

/** Initializes the global state.
 * 
 * @param argc: the command line argument count
 * @param argv: the command line arguments
 * @param filename: the name of the file of the main program
 * @param opt_level: the maximum optimization level
 * @param no_default: whether to initialize the variable table of the main
 * program with built-in values
 * 
 * @return true if the state initialized succesfully and false otherwise. No
 * error is set.
 */
NstEXP bool NstC Nst_state_init(i32 argc, i8 **argv, i8 *filename,
                                i32 opt_level, bool no_default);
/**
 * @brief Frees the variables inside the global state, calls free_lib in the
 * libraries that define it and deletes the objects inside the garbage
 * collector.
 */
NstEXP void NstC Nst_state_free(void);

/* Frees loaded_libs, must be called after Nst_state_free. */
NstEXP void NstC _Nst_unload_libs(void);

/* The global state of the interpreter. */
extern Nst_ExecutionState Nst_state;

/* Returns a pointer to the global state of the interpreter. */
NstEXP Nst_ExecutionState *NstC Nst_get_state(void);

/** Changes the current working directory using a Nst_StrObj.
 * 
 * @return 0 on success and -1 on failure. The error is set.
 */
NstEXP i32 NstC Nst_chdir(Nst_StrObj *str);
/** Gets the current working directory as a Nest string.
 * 
 * @return the new string or NULL on failure. The error is set.
 */
NstEXP Nst_StrObj *NstC Nst_getcwd(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H
