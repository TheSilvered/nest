/* Runtime value and call stack interface */

#ifndef RUNTIME_STACK
#define RUNTIME_STACK

#include "obj.h"
#include "error.h"
#include "function.h"
#include "var_table.h"

#define nst_push_func(f_stack, func, start, end, vt, idx) \
        _nst_push_func(f_stack, AS_FUNC(func), start, end, vt, idx)

#define nst_push_val(v_stack, val) \
        _nst_push_val(v_stack, (Nst_Obj *)(val))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Obj **stack;
    size_t current_size;
    size_t max_size;
}
Nst_ValueStack;

typedef struct
{
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    Nst_Int idx;
}
Nst_FuncCall;

typedef struct
{
    Nst_FuncCall *stack;
    size_t current_size;
    size_t max_size;
}
Nst_CallStack;

// New value stack on the heap
Nst_ValueStack *nst_new_val_stack();
// Push a value on the value stack
// returns false on failing to reallocate the memory
bool _nst_push_val(Nst_ValueStack *v_stack, Nst_Obj *obj);
// Pop a value from the value stack and return it
Nst_Obj *nst_pop_val(Nst_ValueStack *v_stack);
// Returns the top value of the stack
Nst_Obj *nst_peek_val(Nst_ValueStack *v_stack);
// Duplicates the top value of the stack
bool nst_dup_val(Nst_ValueStack *v_stack);
// Destroys the value stack
void nst_destroy_v_stack(Nst_ValueStack *v_stack);

// New call stack on the heap
Nst_CallStack *nst_new_call_stack();
// Pushes a function on the call stack
bool _nst_push_func(Nst_CallStack *f_stack,
                    Nst_FuncObj *func,
                    Nst_Pos call_start,
                    Nst_Pos call_end,
                    Nst_VarTable *vt,
                    Nst_Int idx);
// Pops a function from the call stack
Nst_FuncCall nst_pop_func(Nst_CallStack *f_stack);
// Returns the top function in the call stack
Nst_FuncCall nst_peek_func(Nst_CallStack *f_stack);
// Destroys the call stack
void nst_destroy_f_stack(Nst_CallStack *f_stack);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !RUNTIME_STACK
