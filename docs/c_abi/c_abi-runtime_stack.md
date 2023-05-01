# `runtime_stack.h`

This header contains the functions for handling runtime stacks.

## Macros

### `nst_fstack_push`

**Synopsis**:

```better-c
nst_fstack_push(f_stack, func, start, end, vt, idx)
```

**Description**:

Alias for [`_nst_fstack_push`](#_nst_fstack_push) that casts `func` to
`Nst_FuncObj *`.

---

### `nst_vstack_push`

**Synopsis**:

```better-c
nst_vstack_push(v_stack, val)
```

**Description**:

Alias for [`_nst_vstack_push`](#_nst_vstack_push) that casts `val` to
`Nst_Obj *`.

---

## Structs

### `Nst_ValueStack`

**Synopsis**:

```better-c
typedef struct _Nst_ValueStack
{
    Nst_Obj **stack;
    usize current_size;
    usize max_size;
}
Nst_ValueStack
```

**Description**:

The structure defining the value stack used at runtime.

**Fields**:

- `stack`: the objects on the stack
- `current_size`: the current size of the stack
- `max_size`: the maximum size it can reach before having to be expanded

---

### `Nst_FuncCall`

**Synopsis**:

```better-c
typedef struct _Nst_FuncCall
{
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    Nst_Int idx;
}
Nst_FuncCall
```

**Description**:

The structure defining a function call in the call stack.

**Fields**:

- `func`: the function called
- `start`: the start position of the call
- `end`: the end position of the call
- `vt`: the variable table of the interrupted function
- `idx`: the instruction index of the interrupted function

---

### `Nst_CallStack`

**Synopsis**:

```better-c
typedef struct _Nst_CallStack
{
    Nst_FuncCall *stack;
    usize current_size;
    usize max_size;
}
Nst_CallStack
```

**Description**:

The structure defining the call stack used at runtime.

**Fields**:

- `stack`: the calls on the stack
- `current_size`: the current size of the stack
- `max_size`: the maximum size it can reach before having to be expanded

---

### `Nst_CatchFrame`

**Synopsis**:

```better-c
typedef struct _Nst_CatchFrame
{
    usize f_stack_size;
    usize v_stack_size;
    Nst_Int inst_idx;
}
Nst_CatchFrame
```

**Description**:

The structure defining a catch frame that is reached once an error occurs.

**Fields**:

- `f_stack_size`: the call stack size when the frame was created
- `v_stack_size`: the value stack size when the frame was created
- `inst_idx`: the instruction index when the frame was created

---

### `Nst_CatchStack`

**Synopsis**:

```better-c
typedef struct _Nst_CatchStack
{
    Nst_CatchFrame *stack;
    usize current_size;
    usize max_size;
}
Nst_CatchStack
```
**Description**:

The structure defining the catch stack used at runtime.

**Fields**:

- `stack`: the frames on the stack
- `current_size`: the current size of the stack
- `max_size`: the maximum size it can reach before having to be expanded

---

### `Nst_GenericStack`

**Synopsis**:

```better-c
typedef struct _Nst_GenericStack
{
    void *stack;
    usize current_size;
    usize max_size;
}
Nst_GenericStack
```

**Description**:

A generalized form of a runtime stack

**Fields**:

- `stack`: the stack holding the values
- `current_size`: the current size of the stack
- `max_size`: the maximum size it can reach before having to be expanded

---

## Functions

### `nst_vstack_new`

**Synopsis**:

```better-c
Nst_ValueStack *nst_vstack_new(Nst_OpErr *err)
```

**Description**:

Creates a new value stack on the heap.

**Arguments**:

- `[out] err`: the error

**Return value**:

The function returns the new stack or `NULL` on failure.

---

### `_nst_vstack_push`

**Synopsis**:

```better-c
bool _nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj)
```

**Description**:

Pushes a value on top of the value stack, increasing its reference count.

**Arguments**:

- `[inout] v_stack`: the value stack to push onto
- `[in] obj`: the object to push

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_vstack_pop`

**Synopsis**:

```better-c
Nst_Obj *nst_vstack_pop(Nst_ValueStack *v_stack)
```

**Description**:

Pops a value from the value stack, does not decrease the reference count.

**Arguments**:

- `[inout] v_stack`: the value stack to pop the object from

**Return value**:

The function returns the object popped or `NULL` if the stack was empty.

---

### `nst_vstack_peek`

**Synopsis**:

```better-c
Nst_Obj *nst_vstack_peek(Nst_ValueStack *v_stack)
```

**Arguments**:

- `[in] v_stack`: the stack to peek at

**Return value**:

Returns the top value of the stack

---

### `nst_vstack_dup`

**Synopsis**:

```better-c
bool nst_vstack_dup(Nst_ValueStack *v_stack)
```

**Description**:

Duplicates the top value on the value stack.

**Arguments**:

- `v_stack`: the value stack

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_vstack_destroy`

**Synopsis**:

```better-c
void nst_vstack_destroy(Nst_ValueStack *v_stack)
```

**Description**:

Destroys the value stack.

**Arguments**:

- `[in] v_stack`: the value stack to destroy

---

### `nst_fstack_new`

**Synopsis**:

```better-c
Nst_CallStack *nst_fstack_new(Nst_OpErr *err)
```

**Description**:

Creates a new call stack on the heap.

**Arguments**:

- `[out] err`: the error

**Return value**:

The function returns the new stack or `NULL` on failure.

---

### `_nst_fstack_push`

**Synopsis**:

```better-c
bool _nst_fstack_push(Nst_CallStack *f_stack,
                      Nst_FuncObj   *func,
                      Nst_Pos        call_start,
                      Nst_Pos        call_end,
                      Nst_VarTable  *vt,
                      Nst_Int        idx)
```

**Description**:

Pushes a function on the call stack.

**Arguments**:

- `[inout] f_stack`: the call stack
- `[in] func, call_start, call_end, vt, idx`: the values of the fields of
  [`Nst_FuncCall`](#nst_funccall)

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_fstack_pop`

**Synopsis**:

```better-c
Nst_FuncCall nst_fstack_pop(Nst_CallStack *f_stack)
```

**Description**:

Pops a function call from the call stack.

**Arguments**:

- `[inout] f_stack`: the call stack

**Return value**:

The function returns the function call popped and sets `func` of the call to
`NULL` if the stack was empty.

---

### `nst_fstack_peek`

**Synopsis**:

```better-c
Nst_FuncCall nst_fstack_peek(Nst_CallStack *f_stack)
```

**Description**:

Returns the top call in the call stack

**Arguments**:

- `[in] f_stack`: the call stack

**Return value**:

The function returns the function top call and sets `func` of the call to
`NULL` if the stack was empty.

---

### `nst_fstack_destroy`

**Synopsis**:

```better-c
void nst_fstack_destroy(Nst_CallStack *f_stack)
```

**Description**:

Destroys the call stack.

**Arguments**:

- `[in] f_stack`: the call stack

---

### `nst_cstack_new`

**Synopsis**:

```better-c
Nst_CatchStack *nst_cstack_new(Nst_OpErr *err)
```

**Description**:

Creates a new catch stack on the heap.

**Arguments**:

- `[out] err`: the error

**Return value**:

The function returns the new stack or `NULL` on failure.

---

### `nst_cstack_push`

**Synopsis**:

```better-c
bool nst_cstack_push(Nst_CatchStack *c_stack,
                     Nst_Int         inst_idx,
                     usize           v_stack_size,
                     usize           f_stack_size)
```

**Description**:

Pushes a catch frame onto the catch stack

**Arguments**:

- `[inout] c_stack`: the catch stack
- `[in] inst_idx, v_stack_size, f_stack_size`: the values of the fields of the
  [`Nst_CatchFrame`](#nst_catchframe).

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_cstack_pop`

**Synopsis**:

```better-c
Nst_CatchFrame nst_cstack_pop(Nst_CatchStack *c_stack)
```

**Description**:

Pops the top frame of the catch stack.

**Arguments**:

- `[inout] c_stack`: the catch stack

**Return value**:

The function returns the popped frame, if the stack is empty the `idx` field is
set to `-1`.

---

### `nst_cstack_peek`

**Synopsis**:

```better-c
Nst_CatchFrame nst_cstack_peek(Nst_CatchStack *c_stack)
```

**Description**:

Peeks the top frame of the catch stack.

**Arguments**:

- `[in] c_stack`: the catch stack

**Return value**:

The function returns the top frame of the catch stack, if the stack is empty the
`idx` field is set to `-1`.

---

### `nst_cstack_destroy`

**Synopsis**:

```better-c
void nst_cstack_destroy(Nst_CatchStack *c_stack)
```

**Description**:

Destroys the catch stack.

**Arguments**:

- `[in] c_stack`: the catch stack to destroy

---

### `nst_stack_new`

**Synopsis**:

```better-c
Nst_GenericStack *nst_stack_new(usize      unit_size,
                                usize      starting_size,
                                Nst_OpErr *err)
```

**Description**:

Creates a new generic stack on the heap.

**Arguments**:

- `[in] unit_size`: the size of one element on the stack
- `[in] starting_size`: the initial number of elements on the stack
- `[out] err`: the error

**Return value**:

The function returns the new stack or `NULL` on failure.

---

### `nst_stack_expand`

**Synopsis**:

```better-c
bool nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size)
```

**Description**:

Expands the `max_size` of a generic stack if necessary.

**Arguments**:

- `[inout] g_stack`: the generic stack to expand
- `[in] the size of one element on the stack`

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_stack_shrink`

**Synopsis**:

```better-c
void nst_stack_shrink(Nst_GenericStack *g_stack,
                      usize             min_size,
                      usize             unit_size)
```

**Description**:

Shrinks the stack if necessary.

**Arguments**:

- `[inout] g_stack`: the generic stack to shrink
- `[in] min_size`: the minimum number of elements on the stack
- `[in] unit_size`: the size of one element on the stack
