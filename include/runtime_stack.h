/* Runtime value and call stack interface */

#ifndef RUNTIME_STACK_H
#define RUNTIME_STACK_H

#include "function.h"
#include "var_table.h"

// Pushes a function on the call stack
#define Nst_fstack_push(func, start, end, vt, idx, cstack_size) \
        _Nst_fstack_push(FUNC(func), start, end, vt, idx, cstack_size)
// Push a value on the value stack
// returns false on failing to reallocate the memory
#define Nst_vstack_push(val) _Nst_vstack_push(OBJ(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef struct _Nst_ValueStack
{
    Nst_Obj **stack;
    usize current_size;
    usize max_size;
}
Nst_ValueStack;

NstEXP typedef struct _Nst_FuncCall
{
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    Nst_Int idx;
    usize cstack_size;
}
Nst_FuncCall;

NstEXP typedef struct _Nst_CallStack
{
    Nst_FuncCall *stack;
    usize current_size;
    usize max_size;
}
Nst_CallStack;

NstEXP typedef struct _Nst_CatchFrame
{
    usize f_stack_size;
    usize v_stack_size;
    Nst_Int inst_idx;
}
Nst_CatchFrame;

NstEXP typedef struct _Nst_CatchStack
{
    Nst_CatchFrame *stack;
    usize current_size;
    usize max_size;
}
Nst_CatchStack;

NstEXP typedef struct _Nst_GenericStack
{
    void *stack;
    usize current_size;
    usize max_size;
}
Nst_GenericStack;

// Initializes a value stack
NstEXP bool NstC Nst_vstack_init(void);
NstEXP bool NstC _Nst_vstack_push(Nst_Obj *obj);
// Pop a value from the value stack and return it
NstEXP Nst_Obj *NstC Nst_vstack_pop(void);
// Returns the top value of the stack
NstEXP Nst_Obj *NstC Nst_vstack_peek(void);
// Duplicates the top value of the stack
NstEXP bool NstC Nst_vstack_dup(void);
// Destroys the value stack
NstEXP void NstC Nst_vstack_destroy(void);

// New call stack on the heap
NstEXP bool NstC Nst_fstack_init(void);
NstEXP bool NstC _Nst_fstack_push(Nst_FuncObj *func, Nst_Pos call_start,
                                  Nst_Pos call_end, Nst_VarTable *vt,
                                  Nst_Int idx, usize cstack_size);
// Pops a function from the call stack
NstEXP Nst_FuncCall NstC Nst_fstack_pop(void);
// Returns the top function in the call stack
NstEXP Nst_FuncCall NstC Nst_fstack_peek(void);
// Destroys the call stack
NstEXP void NstC Nst_fstack_destroy(void);

// New catch stack on the heap
NstEXP bool NstC Nst_cstack_init(void);
// Pushes a value to the catch stack
NstEXP bool NstC Nst_cstack_push(Nst_Int inst_idx, usize v_stack_size,
                                 usize f_stack_size);
// Peeks the top value of the catch stack
NstEXP Nst_CatchFrame NstC Nst_cstack_peek(void);
// Returns the top value of the catch stack
NstEXP Nst_CatchFrame NstC Nst_cstack_pop(void);
// Destroys the catch stack
NstEXP void NstC Nst_cstack_destroy(void);

// Initializes a new runtime stack
NstEXP bool NstC Nst_stack_init(Nst_GenericStack *g_stack, usize unit_size,
                                usize starting_size);
// Expands a generic stack
NstEXP bool NstC Nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size);
// Shrinks a runtime stack
NstEXP void NstC Nst_stack_shrink(Nst_GenericStack *g_stack, usize min_size,
                                  usize unit_size);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNTIME_STACK_H
