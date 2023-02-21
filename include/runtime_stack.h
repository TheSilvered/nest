/* Runtime value and call stack interface */

#ifndef RUNTIME_STACK_H
#define RUNTIME_STACK_H

#include "function.h"
#include "var_table.h"

// Pushes a function on the call stack
#define nst_fstack_push(f_stack, func, start, end, vt, idx) \
        _nst_fstack_push(f_stack, FUNC(func), start, end, vt, idx)
// Push a value on the value stack
// returns false on failing to reallocate the memory
#define nst_vstack_push(v_stack, val) _nst_vstack_push(v_stack, OBJ(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_ValueStack
{
    Nst_Obj **stack;
    usize current_size;
    usize max_size;
}
Nst_ValueStack;

typedef struct _Nst_FuncCall
{
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    Nst_Int idx;
}
Nst_FuncCall;

typedef struct _Nst_CallStack
{
    Nst_FuncCall *stack;
    usize current_size;
    usize max_size;
}
Nst_CallStack;

typedef struct _Nst_CatchFrame
{
    usize f_stack_size;
    usize v_stack_size;
    Nst_Int inst_idx;
}
Nst_CatchFrame;

typedef struct _Nst_CatchStack
{
    Nst_CatchFrame *stack;
    usize current_size;
    usize max_size;
}
Nst_CatchStack;

// New value stack on the heap
Nst_ValueStack *nst_vstack_new();
bool _nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj);
// Pop a value from the value stack and return it
Nst_Obj *nst_vstack_pop(Nst_ValueStack *v_stack);
// Returns the top value of the stack
Nst_Obj *nst_vstack_peek(Nst_ValueStack *v_stack);
// Duplicates the top value of the stack
bool nst_vstack_dup(Nst_ValueStack *v_stack);
// Destroys the value stack
void nst_vstack_destroy(Nst_ValueStack *v_stack);

// New call stack on the heap
Nst_CallStack *nst_fstack_new();
bool _nst_fstack_push(Nst_CallStack *f_stack,
                      Nst_FuncObj   *func,
                      Nst_Pos        call_start,
                      Nst_Pos        call_end,
                      Nst_VarTable  *vt,
                      Nst_Int        idx);
// Pops a function from the call stack
Nst_FuncCall nst_fstack_pop(Nst_CallStack *f_stack);
// Returns the top function in the call stack
Nst_FuncCall nst_fstack_peek(Nst_CallStack *f_stack);
// Destroys the call stack
void nst_fstack_destroy(Nst_CallStack *f_stack);

// New catch stack on the heap
Nst_CatchStack *nst_cstack_new();
// Pushes a value to the catch stack
bool nst_cstack_push(Nst_CatchStack *c_stack,
                     Nst_Int         inst_idx,
                     usize           v_stack_size,
                     usize           f_stack_size);
// Peeks the top value of the catch stack
Nst_CatchFrame nst_cstack_peek(Nst_CatchStack *c_stack);
// Returns the top value of the catch stack
Nst_CatchFrame nst_cstack_pop(Nst_CatchStack *c_stack);
// Destroys the catch stack
void nst_cstack_destroy(Nst_CatchStack *c_stack);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNTIME_STACK_H
