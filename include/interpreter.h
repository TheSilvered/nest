/**
 * @file interpreter.h
 *
 * @brief Bytecode interpreter
 *
 * @author TheSilvered
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "program.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing the current state of the Nest interpreter.
 *
 * @param prog: the program being run, if NULL all other fields could have
 * invalid data and are not safe to read
 * @param v_stack: the value stack
 * @param f_stack: the call stack
 * @param c_stack: the catch frame stack
 * @param func: the function currently being executed
 * @param vt: the current variable table
 * @param idx: the index of the current bytecode instruction
 */
NstEXP typedef struct _Nst_InterpreterState {
    Nst_Program *prog;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
    Nst_Obj *func;
    Nst_VarTable vt;
    i64 idx;
} Nst_InterpreterState;

/**
 * Initialize the Nest libraray.
 *
 * @brief Note: `Nst_error_set_color` is called with the value returned by
 * `Nst_supports_color`.
 *
 * @return `true` on success and `false` on failure. The error cannot be
 * accessed if the library fails to initialize.
 */
NstEXP bool NstC Nst_init(void);

/**
 * Destroy all the components of the libraray. It is not safe to access any
 * object created while the library was initialized after this function is
 * called. Any destructors that may access Nest objects must be called before
 * calling this function.
 */
NstEXP void NstC Nst_quit(void);

/* Returns `true` if the state was initialized and `false` otherwise. */
NstEXP bool NstC Nst_was_init(void);

/**
 * Run a program.
 *
 * @brief Warning: it must never be called inside a library.
 *
 * @param prog: the program to run
 *
 * @return The exit code of the program.
 */
NstEXP i32 NstC Nst_run(Nst_Program *prog);
/**
 * Run an external Nest file.
 *
 * @param file_name: the name of the file to run
 *
 * @return A map containing the variables of the module or NULL on failure. The
 * error is set.
 */
NstEXP Nst_Obj *NstC Nst_run_module(const char *file_name);

/**
 * Call a `Func` object.
 *
 * @brief Note: if the function is passed less arguments than it expects, the
 * extra ones are filled with `null` objects.
 *
 * @param func: the function to call
 * @param arg_num: the number of arguments passed
 * @param args: the array of arguments to pass to it
 *
 * @return The return value of the function or `NULL` on failure. The error is
 * set.
 */
NstEXP Nst_ObjRef *NstC Nst_func_call(Nst_Obj *func, usize arg_num,
                                      Nst_Obj **args);

/**
 * Yield a coroutine.
 *
 * @brief If `stack` is `NULL` the function will only set `out_stack_size` and
 * return without modifying the state of the interpreter. This is to allow the
 * caller to allocate enough memory to hold the stack.
 *
 * @param out_stack: buffer filled with the values from the top function call,
 * each object added is a reference to be handled by the caller
 * @param out_stack_size: the number of object added to `out_stack`
 * @param out_idx: the index of the current instruction
 * @param out_vt: the current variable table
 *
 * @return The paused function.
 */
NstEXP Nst_Obj *NstC Nst_coroutine_yield(Nst_ObjRef **out_stack,
                                         usize *out_stack_size,
                                         i64 *out_idx, Nst_VarTable *out_vt);
/**
 * Execute the body of a `Func` object that has a Nest body using a given
 * context.
 *
 * @brief The context is set according to the arguments passed.
 *
 * @param func: the function to execute
 * @param idx: the instruction index from which to start the execution of the
 * body
 * @param value_stack: the values to push on the value stack
 * @param value_stack_len: the length of `value_stack`
 * @param vt: variable table to use
 *
 * @return The result of the function or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_coroutine_resume(Nst_Obj *func, i64 idx,
                                             Nst_ObjRef **value_stack,
                                             usize value_stack_len,
                                             Nst_VarTable vt);

/**
 * @return The position of the current operation.
 */
NstEXP Nst_Span NstC Nst_state_span(void);
/**
 * @return The current state of the interpreter.
 */
NstEXP const Nst_InterpreterState *NstC Nst_state(void);

/**
 * Change the current working directory using a Nest `Str` object.
 *
 * @return `0` on success and `-1` on failure. The error is set.
 */
NstEXP i32 NstC Nst_chdir(Nst_Obj *str);
/**
 * Get the current working directory as a Nest `Str` object.
 *
 * @return the new string or `NULL` on failure. The error is set.
 */
NstEXP Nst_ObjRef *NstC Nst_getcwd(void);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !INTERPRETER_H
