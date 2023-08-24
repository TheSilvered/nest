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

/* Alias for `_Nst_fstack_push` that casts func to `Nst_FuncObj *`. */
#define Nst_fstack_push(func, start, end, vt, idx, cstack_size) \
        _Nst_fstack_push(FUNC(func), start, end, vt, idx, cstack_size)
/* Alias for `_Nst_vstack_push` that casts val to `Nst_Obj *`. */
#define Nst_vstack_push(val) _Nst_vstack_push(OBJ(val))

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
 * @param start: the start position of the call
 * @param end: the end position of the call
 * @param vt: the variable table of the call
 * @param idx: the instruction index of the call
 * @param cstack_len: the size of the catch stack when the function was called
 */
NstEXP typedef struct _Nst_FuncCall {
    Nst_FuncObj *func;
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
 * Initializes the value stack.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_vstack_init(void);
/**
 * Pushes a value on the value stack.
 *
 * @param obj: the value to be pushed, if not `NULL` its refcount is increased
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC _Nst_vstack_push(Nst_Obj *obj);
/**
 * @brief Pops the top value from the value stack and returns it. If the stack
 * is empty `NULL` is returned. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_vstack_pop(void);
/**
 * @return The top value from the value stack. If the stack is empty `NULL` is
 * returned. No error is set.
 */
NstEXP Nst_Obj *NstC Nst_vstack_peek(void);
/**
 * Duplicates the top value of the stack.
 *
 * @brief If the stack is empty nothing is done.
 *
 * @return `true` on success and `false` on failure. If the stack is empty the
 * function always succeeds. The error is set.
 */
NstEXP bool NstC Nst_vstack_dup(void);
/* Destroys the value stack. */
NstEXP void NstC Nst_vstack_destroy(void);

/**
 * Initializes the call stack.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_fstack_init(void);
/**
 * Pushes a call on the call stack.
 *
 * @param func: the function of the call
 * @param call_start: the start position of the call
 * @param call_end: the end position of the call
 * @param vt: the current variable table
 * @param idx: the current instruction index
 * @param cstack_size: the current size of the catch stack
 *
 * @return `true` on success and `false` on failure. The error is not always
 * set.
 */
NstEXP bool NstC _Nst_fstack_push(Nst_FuncObj *func, Nst_Pos call_start,
                                  Nst_Pos call_end, Nst_VarTable *vt,
                                  i64 idx, usize cstack_size);
/**
 * @brief Pops the top call from the call stack and returns it. If the stack
 * is empty, a `Nst_FuncCall` with a `NULL` `func` and `vt` is returned. No
 * error is set.
 */
NstEXP Nst_FuncCall NstC Nst_fstack_pop(void);
/**
 * @brief Returns the top function in the call stack. If the stack is empty, a
 * `Nst_FuncCall` with a `NULL` `func` and `vt` is returned. No error is set.
 */
NstEXP Nst_FuncCall NstC Nst_fstack_peek(void);
/* Destroys the call stack. */
NstEXP void NstC Nst_fstack_destroy(void);

/**
 * Initializes the catch stack.
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_cstack_init(void);
// Pushes a value on the catch stack
NstEXP bool NstC Nst_cstack_push(i64 inst_idx, usize v_stack_size,
                                 usize f_stack_size);
/**
 * @brief Returns the top value of the catch stack. If the stack is empty a
 * `Nst_CatchFrame` with an `inst_idx` of `-1` is returned. No error is set.
 */
NstEXP Nst_CatchFrame NstC Nst_cstack_peek(void);
/**
 * @brief Pops the top value of the catch stack and returns it. If the stack is
 * empty a `Nst_CatchFrame` with an `inst_idx` of `-1` is returned. No error is
 * set.
 */
NstEXP Nst_CatchFrame NstC Nst_cstack_pop(void);
/* Destroys the catch stack. */
NstEXP void NstC Nst_cstack_destroy(void);

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
