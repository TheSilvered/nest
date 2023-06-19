/* Runtime value and call stack interface */

#ifndef RUNTIME_STACK_H
#define RUNTIME_STACK_H

#include "function.h"
#include "var_table.h"

// Pushes a function on the call stack
#define nst_fstack_push(func, start, end, vt, idx) \
        _nst_fstack_push(FUNC(func), start, end, vt, idx)
// Push a value on the value stack
// returns false on failing to reallocate the memory
#define nst_vstack_push(val) _nst_vstack_push(OBJ(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef struct _Nst_ValueStack
{
    Nst_Obj **stack;
    usize current_size;
    usize max_size;
}
Nst_ValueStack;

EXPORT typedef struct _Nst_FuncCall
{
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    Nst_Int idx;
}
Nst_FuncCall;

EXPORT typedef struct _Nst_CallStack
{
    Nst_FuncCall *stack;
    usize current_size;
    usize max_size;
}
Nst_CallStack;

EXPORT typedef struct _Nst_CatchFrame
{
    usize f_stack_size;
    usize v_stack_size;
    Nst_Int inst_idx;
}
Nst_CatchFrame;

EXPORT typedef struct _Nst_CatchStack
{
    Nst_CatchFrame *stack;
    usize current_size;
    usize max_size;
}
Nst_CatchStack;

EXPORT typedef struct _Nst_GenericStack
{
    void *stack;
    usize current_size;
    usize max_size;
}
Nst_GenericStack;

// Initializes a value stack
EXPORT bool nst_vstack_init(Nst_OpErr *err);
EXPORT bool _nst_vstack_push(Nst_Obj *obj);
// Pop a value from the value stack and return it
EXPORT Nst_Obj *nst_vstack_pop();
// Returns the top value of the stack
EXPORT Nst_Obj *nst_vstack_peek();
// Duplicates the top value of the stack
EXPORT bool nst_vstack_dup();
// Destroys the value stack
EXPORT void nst_vstack_destroy();

// New call stack on the heap
EXPORT bool nst_fstack_init(Nst_OpErr *err);
EXPORT bool _nst_fstack_push(Nst_FuncObj   *func,
                             Nst_Pos        call_start,
                             Nst_Pos        call_end,
                             Nst_VarTable  *vt,
                             Nst_Int        idx);
// Pops a function from the call stack
EXPORT Nst_FuncCall nst_fstack_pop();
// Returns the top function in the call stack
EXPORT Nst_FuncCall nst_fstack_peek();
// Destroys the call stack
EXPORT void nst_fstack_destroy();

// New catch stack on the heap
EXPORT bool nst_cstack_init(Nst_OpErr *err);
// Pushes a value to the catch stack
EXPORT bool nst_cstack_push(Nst_Int inst_idx,
                            usize   v_stack_size,
                            usize   f_stack_size);
// Peeks the top value of the catch stack
EXPORT Nst_CatchFrame nst_cstack_peek();
// Returns the top value of the catch stack
EXPORT Nst_CatchFrame nst_cstack_pop();
// Destroys the catch stack
EXPORT void nst_cstack_destroy();

// Initializes a new runtime stack
EXPORT bool nst_stack_init(Nst_GenericStack *g_stack,
                           usize             unit_size,
                           usize             starting_size,
                           Nst_OpErr        *err);
// Expands a generic stack
EXPORT bool nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size);
// Shrinks a runtime stack
EXPORT void nst_stack_shrink(Nst_GenericStack *g_stack,
                             usize             min_size,
                             usize             unit_size);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNTIME_STACK_H
