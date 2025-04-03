/**
 * @file runner.h
 *
 * @brief Functions for managing execution states
 *
 * @author TheSilvered
 */

#ifndef RUNNER_H
#define RUNNER_H

#include "argv_parser.h"
#include "runtime_stack.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

#ifdef _DEBUG
#define Nst_es_assert_vstack_size_at_least(es, size)                          \
    Nst_assert(es->v_stack.len >= size)
#else
#define Nst_es_assert_vstack_size_at_least(es, size)
#endif // !_DEBUG

/**
 * Execution state of a Nest program.
 *
 * @param traceback: traceback of the current running program
 * @param vt: current variable table
 * @param idx: current instruction index
 * @param argv: arguments passed to the program
 * @param curr_path: the current working directory
 * @param source_path: the path of the main file
 * @param v_stack: value stack
 * @param f_stack: call stack
 * @param c_stack: catch stack
 */
NstEXP typedef struct _Nst_ExecutionState {
    Nst_Traceback traceback;
    Nst_VarTable *vt;
    i64 idx;
    Nst_Obj *argv;
    Nst_Obj *curr_path;
    Nst_Obj *source_path;
    Nst_ValueStack v_stack;
    Nst_CallStack  f_stack;
    Nst_CatchStack c_stack;
} Nst_ExecutionState;

/* Initializes an execution state. */
NstEXP bool NstC Nst_es_init(Nst_ExecutionState *es);
/* Destroys the contents of an execution state. */
NstEXP void NstC Nst_es_destroy(Nst_ExecutionState *es);
/**
 * Initializes a `Nst_FuncCall` using the fields of a `Nst_ExecutionState`.
 *
 * @brief Warning: the `cwd` argument is set to `NULL` and its value must be
 * set manually.
 *
 * @param func: the function in the function call
 * @param start: the starting position of the function call
 * @param end: the ending position of the function call
 * @param es: the execution state from which to take `idx`, `cstack_len` and
 * `vt`
 *
 * @return An initialized `Nst_FuncCall` structure.
 */
NstEXP Nst_FuncCall NstC Nst_func_call_from_es(Nst_Obj *func, Nst_Span span,
                                               Nst_ExecutionState *es);
/**
 * Initializes the variable table and command-line arguments array of an
 * execution state.
 *
 * @param es: the execution state to set the value of
 * @param argc: the number of command-line arguments passed, ignoring the
 * filename; if this is `0`, `argv` is ignored
 * @param argv: an array of strings containing the command-line arguments
 * passed
 * @param filename: the name of the file being executed
 * @param no_default: whether the variable table should contain any default
 * variable (with the exception of `_vars_`)
 */
NstEXP bool NstC Nst_es_init_vt(Nst_ExecutionState *es, Nst_CLArgs *cl_args);
/**
 * Executes a Nest program given the arguments.
 *
 * @brief Note: `es` and `src` are not destroyed when the function ends and
 * must be destroyed manually with `Nst_es_destroy` and
 * `Nst_source_text_destroy` respectively.
 *
 * @param args: the arguments for the program
 * @param es: the execution state that will be filled by the program
 * @param src: the source of the opened file that will be filled by the program
 *
 * @return The exit code of the program. If it is different from zero an error
 * could have occurred, to check use `Nst_error_occurred`.
 */
NstEXP i32 NstC Nst_execute(Nst_CLArgs args, Nst_ExecutionState *es,
                            Nst_SourceText *src);
/**
 * Sets the `curr_path` field of an `Nst_ExecutionState`. This function takes a
 * reference from `cwd` and removes one from the previous value in the state.
 *
 * @param es: the execution state of which to change `curr_path`
 * @param cwd: the new value for `curr_path`
 */
NstEXP void NstC Nst_es_set_cwd(Nst_ExecutionState *es, Nst_Obj *cwd);
/**
 * Compiles a module given a path and sets up an execution state to run it.
 *
 * @param es: the execution state on which to run the module
 * @param filename: the path of the module to run
 * @param source_text: the source text filled with the source of the module
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_es_push_module(Nst_ExecutionState *es,
                                    const char *filename,
                                    Nst_SourceText *source_text);
/**
 * Pushes a function on the call stack and creates a new local variable table
 * for the function.
 *
 * @brief Note: if you pass less arguments than the function expects, the
 * remaining ones are filled with `null`.
 *
 * @param es: the execution state to push the function onto
 * @param func: the function to push on the execution state
 * @param span: the position of the call
 * @param arg_num: the number of arguments passed to the function
 * @param args: the values of the arguments to pass to the function, if `NULL`
 * `arg_num` values are taken from the value stack in reverse order
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_es_push_func(Nst_ExecutionState *es, Nst_Obj *func,
                                  Nst_Span span, usize arg_num, Nst_Obj **args);
/**
 * Pushes a coroutine that is already running on the call stack of the given
 * execution state.
 *
 * @param es: the execution state to push the coroutine onto
 * @param func: the function of the coroutine to push
 * @param span: the position of the call
 * @param idx: the instruction index where the coroutine was paused at
 * @param vt: the variable table of the coroutine
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_es_push_paused_coroutine(Nst_ExecutionState *es,
                                              Nst_Obj *func, Nst_Span span,
                                              i64 idx, Nst_VarTable *vt);
/* Forces the top function of the execution state to end. */
NstEXP void NstC Nst_es_force_function_end(Nst_ExecutionState *es);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNNER_H
