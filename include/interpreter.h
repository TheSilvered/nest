/**
 * @file interpreter.h
 *
 * @brief Bytecode interpreter
 *
 * @author TheSilvered
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "runner.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Global state of the Nest interpreter.
 *
 * @param ggc: generational garbage collector
 * @param opt_level: maximum optimization level when importing libraries
 * @param loaded_libs: dynamic library handles
 * @param lib_paths: import stack
 * @param lib_handles: maps of imported libraries
 * @param lib_srcs: sources of imported Nest libraries
 * @param es: the program that is being executed
 */
NstEXP typedef struct _Nst_IntrState {
    Nst_GarbageCollector ggc;
    i32 opt_level;
    Nst_LList *loaded_libs;
    Nst_LList *lib_paths;
    Nst_Obj *lib_handles;
    Nst_LList *lib_srcs;
    Nst_ExecutionState *es;
    Nst_Traceback global_traceback;
} Nst_IntrState;

/**
 * Initializes the Nest libraray.
 *
 * @brief Note: `Nst_set_color` is called with the value returned by
 * `Nst_supports_color`.
 *
 * @param args: the options for the libraray, currently only `opt_level` is
 * used; if `NULL` is passed `opt_level` is set to 3.
 *
 * @return `true` on success and `false` on failure. No error is set as it
 * failed to initialize.
 */
NstEXP bool NstC Nst_init(Nst_CLArgs *args);

/**
 * @brief Destroys all the components of the libraray. It is not safe to access
 * any object created while the library was initialized after this function is
 * called. Any destructors that may access Nest objects must be called before.
 */
NstEXP void NstC Nst_quit(void);

/**
 * Runs the main program.
 *
 * @brief Warning: it must never be called inside a library.
 *
 * @param main_func: the function object of the main program
 *
 * @return The exit code of the program.
 */
NstEXP i32 NstC Nst_run(Nst_Obj *main_func);
/**
 * Runs an external Nest file.
 *
 * @param file_name: the name of the file to run, must exist since no checking
 * is done
 * @param lib_src: the pointer where to store the source of the file, if an
 * error occurs the source of the library is expected to be on the lib_srcs
 * list of the global state
 *
 * @return `true` on success and `false` on failure. If the function succeeds,
 * the result of the module is on top of the value stack. The global operation
 * error is not set but an internal one is, hence the caller must not set the
 * error.
 */
NstEXP bool NstC Nst_run_module(i8 *file_name, Nst_SourceText *lib_src);
/**
 * Calls a `Func` object.
 *
 * @brief Note: if the function is passed less arguments than it expects, the
 * extra ones are filled with `null` objects.
 *
 * @param func: the function to call
 * @param arg_num: the number of arguments passed
 * @param args: the array of arguments to pass to it
 *
 * @return The result of the function or `NULL` on failure. The error is set.
 */
NstEXP Nst_Obj *NstC Nst_func_call(Nst_Obj *func, i64 arg_num, Nst_Obj **args);

/**
 * Executes the body of a `Func` object that has a Nest body using a given
 * context.
 *
 * @brief The context is set according to the arguments passed.
 *
 * @param func: the function to execute
 * @param idx: the instruction index from which to start the execution of the
 * body
 * @param vars: the local variable table
 * @param globals: the global variable table, it may be `NULL`, in which case
 * it is determined automatically
 *
 * @return The result of the function or `NULL` on failure. The error is set
 * internally and must not be set by the caller.
 */
NstEXP Nst_Obj *NstC Nst_run_paused_coroutine(Nst_Obj *func, i64 idx,
                                              Nst_VarTable *vt);
/**
 * Returns the absolute path to a file system object.
 *
 * @brief Note: the absolute path is allocated on the heap and should be freed
 * with `Nst_free` when appropriate.
 *
 * @param file_path: the relative path to the object
 * @param buf: the buf where the absolute path is placed
 * @param file_part: where the start of the file name inside the file path is
 * put, this may be `NULL` in which case it is ignored
 *
 * @return The length in bytes of the absolute path or 0 on failure. The error
 * is set.
 */
NstEXP usize NstC Nst_get_full_path(i8 *file_path, i8 **buf, i8 **file_part);

/**
 * @brief Returns a pointer to the current instruction being executed. On
 * failure `NULL` is returned. No error is set.
 */
NstEXP Nst_Inst *NstC Nst_current_inst(void);

/* Returns `true` if the state was initialized and `false` otherwise. */
NstEXP bool NstC Nst_was_init(void);

/* Gets the current execution state. */
NstEXP Nst_ExecutionState *NstC Nst_state_get_es(void);
/**
 * Sets a new execution state in the global interpreter state.
 *
 * @brief Note: the current working directory is changed according to
 * `curr_path` in `es`.
 *
 * @param es: the new `Nst_ExecutionState` to set
 *
 * @return The previous execution state. The error is set but it is not
 * reflected in the return value. Use `Nst_error_occurred` to check.
 */
NstEXP Nst_ExecutionState *NstC Nst_state_set_es(Nst_ExecutionState *es);

// The global state of the interpreter.
extern Nst_IntrState Nst_state;

/* [docs:link Nst_state Nst_state_get] */
/* Returns a pointer to the global `Nst_IntrState`. */
NstEXP Nst_IntrState *NstC Nst_state_get(void);

/**
 * Changes the current working directory using a `Nst_StrObj`.
 *
 * @return `0` on success and `-1` on failure. The error is set.
 */
NstEXP i32 NstC Nst_chdir(Nst_StrObj *str);
/**
 * Gets the current working directory as a `Nst_StrObj`.
 *
 * @return the new string or `NULL` on failure. The error is set.
 */
NstEXP Nst_StrObj *NstC Nst_getcwd(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H
