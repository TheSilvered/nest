/**
 * @file runtime_stack.h
 *
 * @brief Stacks used for the VM during runtime
 *
 * @author TheSilvered
 */


#ifndef RUNTIME_STACK_H
#define RUNTIME_STACK_H

#include "function.h"
#include "var_table.h"

/* Alias for `_Nst_vstack_push` that casts val to `Nst_Obj *`. */
#define Nst_vstack_push(v_stack, val) _Nst_vstack_push(v_stack, OBJ(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * A structure representing the value stack.
 *
 * @param stack: the objects in the stack
 * @param len: the current size of the stack
 * @param cap: the maximum size of the stack before it needs to be
 * expanded
 */
NstEXP typedef struct _Nst_ValueStack {
    Nst_Obj **stack;
    usize len;
    usize cap;
} Nst_ValueStack;

/**
 * A structure representing a function call.
 *
 * @param func: the function being called
 * @param cwd: the current working directory, changed when the call is back on
 * top of the stack; nothing is done if it is `NULL`
 * @param start: the start position of the call
 * @param end: the end position of the call
 * @param vt: the variable table of the call
 * @param idx: the instruction index of the call
 * @param cstack_len: the size of the catch stack when the function was called
 */
NstEXP typedef struct _Nst_FuncCall {
    Nst_Obj *func;
    Nst_Obj *cwd;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    i64 idx;
    usize cstack_len;
} Nst_FuncCall;

/**
 * A structure representing the call stack.
 *
 * @param stack: the calls in the stack
 * @param len: the current size of the stack
 * @param cap: the maximum size of the stack before it needs to be
 * expanded
 */
NstEXP typedef struct _Nst_CallStack {
    Nst_FuncCall *stack;
    usize len;
    usize cap;
    usize max_recursion_depth;
} Nst_CallStack;

/**
 * A structure representing the state of the stack when a catch was pushed and
 * that is restored if an error occurs.
 *
 * @param f_stack_len: the size of the call stack
 * @param v_stack_len: the size of the value stack
 * @param inst_idx: the instruction index
 */
NstEXP typedef struct _Nst_CatchFrame {
    usize f_stack_len;
    usize v_stack_len;
    i64 inst_idx;
} Nst_CatchFrame;

/**
 * A structure representing the catch stack.
 *
 * @param stack: the catch frames in the stack
 * @param len: the current size of the stack
 * @param cap: the maximum size of the stack before it needs to be expanded
 */
NstEXP typedef struct _Nst_CatchStack {
    Nst_CatchFrame *stack;
    usize len;
    usize cap;
} Nst_CatchStack;

/**
 * A structure representing a generic stack.
 *
 * @param stack: the pointer to the array of elements in the stack
 * @param len: the current size of the stack
 * @param cap: the maximum size of the stack before it needs to be expanded
 */
NstEXP typedef struct _Nst_GenericStack {
    void *stack;
    usize len;
    usize cap;
} Nst_GenericStack;

/**
 * Initializes a value stack.
 *
 * @param v_stack: the value stack to initialize
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_vstack_init(Nst_ValueStack *v_stack);
/**
 * Pushes a value on a value stack.
 *
 * @param v_stack: the value stack push the object onto
 * @param obj: the value to be pushed, if not `NULL` its refcount is increased
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC _Nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj);
/**
 * Pops the top value from a value stack.
 *
 * @param v_stack: the value stack to pop the value from
 *
 * @return The popped value. If the stack is empty `NULL` is returned. No error
 * is set.
 */
NstEXP Nst_Obj *NstC Nst_vstack_pop(Nst_ValueStack *v_stack);
/**
 * Peeks at the top value of a value stack.
 *
 * @param v_stack: the value stack to peek from
 *
 * @return The top value from the value stack. If the stack is empty `NULL` is
 * returned. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_vstack_peek(Nst_ValueStack *v_stack);
/**
 * Duplicates the top value of a value stack.
 *
 * @brief If the stack is empty nothing is done.
 *
 * @param v_stack: the value stack to duplicate the value of
 *
 * @return `true` on success and `false` on failure. If the stack is empty the
 * function always succeeds. The error is set.
 */
NstEXP bool NstC Nst_vstack_dup(Nst_ValueStack *v_stack);
/* Destroys the contents of a value stack. */
NstEXP void NstC Nst_vstack_destroy(Nst_ValueStack *v_stack);

/**
 * Initializes a call stack.
 *
 * @param f_stack: the call stack to initialize
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_fstack_init(Nst_CallStack *f_stack);
/**
 * Pushes a call on a call stack.
 *
 * @brief Note: the reference count of the function inside `call` is
 * automatically increased. `func` may still be `NULL`.
 *
 * @param f_stack: the call stack to push the call onto
 * @param call: the call to push on the stack
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_fstack_push(Nst_CallStack *f_stack, Nst_FuncCall call);
/**
 * Pops the top call from a call stack
 *
 * @param f_stack: the call stack to pop the value from
 *
 * @return The popped value. If the stack is empty, a `Nst_FuncCall` with a
 * `NULL` `func` and `vt` is returned. No error is set.
 */
NstEXP Nst_FuncCall NstC Nst_fstack_pop(Nst_CallStack *f_stack);
/**
 * Peeks at the top call of a call stack.
 *
 * @param f_stack: the call stack to peek from
 *
 * @return The top function in the call stack. If the stack is empty, a
 * `Nst_FuncCall` with a `NULL` `func` and `vt` is returned. No error is set.
 */
NstEXP Nst_FuncCall NstC Nst_fstack_peek(Nst_CallStack *f_stack);
/* Destroys the contents of a call stack. */
NstEXP void NstC Nst_fstack_destroy(Nst_CallStack *f_stack);

/**
 * Initializes a catch stack.
 *
 * @param c_stack: the catch stack to initialize
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_cstack_init(Nst_CatchStack *c_stack);
/**
 * Pushes a frame on a catch stack.
 *
 * @param c_stack: the catch stack to push the frame onto
 * @param frame: the `Nst_CatchFrame` to push on the stack
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_cstack_push(Nst_CatchStack *c_stack, Nst_CatchFrame frame);
/**
 * Peeks at the top frame of a catch stack.
 *
 * @param c_stack: the catch stack to peek from
 *
 * @return The top value of the catch stack. If the stack is empty a
 * `Nst_CatchFrame` with an `inst_idx` of `-1` is returned. No error is set.
 */
NstEXP Nst_CatchFrame NstC Nst_cstack_peek(Nst_CatchStack *c_stack);
/**
 * Pops the top value of a catch stack.
 *
 * @param c_stack: the catch stack to pop the frame from
 *
 * @return The popped frame. If the stack is empty a `Nst_CatchFrame` with an
 * `inst_idx` of `-1` is returned. No error is set.
 */
NstEXP Nst_CatchFrame NstC Nst_cstack_pop(Nst_CatchStack *c_stack);
/* Destroys the contents of a catch stack. */
NstEXP void NstC Nst_cstack_destroy(Nst_CatchStack *c_stack);

/**
 * Initializes a new generic stack.
 *
 * @param g_stack: the stack to initialize
 * @param unit_size: the size in bytes of one element in the stack
 * @param starting_size: the initial number of elements in the stack
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_stack_init(Nst_GenericStack *g_stack, usize unit_size,
                                usize starting_size);
/**
 * Expands a generic stack if needed.
 *
 * @param g_stack: the stack to expand
 * @param unit_size: the size of one element in the stack
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size);
/**
 * Shrinks a runtime stack if needed.
 *
 * @param g_stack: the stack to shrink
 * @param min_size: the minimum size that the stack can reach
 * @param unit_size: the size of one element in the stack
 */
NstEXP void NstC Nst_stack_shrink(Nst_GenericStack *g_stack, usize min_size,
                                  usize unit_size);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNTIME_STACK_H
