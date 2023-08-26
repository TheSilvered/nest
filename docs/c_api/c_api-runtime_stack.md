# `runtime_stack.h`

Stacks used for the VM during runtime.

## Authors

TheSilvered

## Macros

### `Nst_fstack_push`

**Synopsis:**

```better-c
Nst_fstack_push(func, start, end, vt, idx, cstack_size)
```

**Description:**

Alias for [`_Nst_fstack_push`](c_api-runtime_stack.md#_nst_fstack_push) that
casts func to [`Nst_FuncObj *`](c_api-function.md#nst_funcobj).

---

### `Nst_vstack_push`

**Synopsis:**

```better-c
Nst_vstack_push(val)
```

**Description:**

Alias for [`_Nst_vstack_push`](c_api-runtime_stack.md#_nst_vstack_push) that
casts val to [`Nst_Obj *`](c_api-obj.md#nst_obj).

---

## Structs

### `Nst_ValueStack`

**Synopsis:**

```better-c
typedef struct _Nst_ValueStack {
    Nst_Obj **stack;
    usize len;
    usize cap;
} Nst_ValueStack
```

**Description:**

A structure representing the value stack.

**Fields:**

- `stack`: the objects in the stack
- `len`: the current size of the stack
- `cap`: the maximum size of the stack before it needs to be expanded

---

### `Nst_FuncCall`

**Synopsis:**

```better-c
typedef struct _Nst_FuncCall {
    Nst_FuncObj *func;
    Nst_Pos start;
    Nst_Pos end;
    Nst_VarTable *vt;
    i64 idx;
    usize cstack_len;
} Nst_FuncCall
```

**Description:**

A structure representing a function call.

**Fields:**

- `func`: the function being called
- `start`: the start position of the call
- `end`: the end position of the call
- `vt`: the variable table of the call
- `idx`: the instruction index of the call
- `cstack_len`: the size of the catch stack when the function was called

---

### `Nst_CallStack`

**Synopsis:**

```better-c
typedef struct _Nst_CallStack {
    Nst_FuncCall *stack;
    usize len;
    usize cap;
} Nst_CallStack
```

**Description:**

A structure representing the call stack.

**Fields:**

- `stack`: the calls in the stack
- `len`: the current size of the stack
- `cap`: the maximum size of the stack before it needs to be expanded

---

### `Nst_CatchFrame`

**Synopsis:**

```better-c
typedef struct _Nst_CatchFrame {
    usize f_stack_len;
    usize v_stack_len;
    i64 inst_idx;
} Nst_CatchFrame
```

**Description:**

A structure representing the state of the stack when a catch was pushed and that
is restored if an error occurs.

**Fields:**

- `f_stack_len`: the size of the call stack
- `v_stack_len`: the size of the value stack
- `inst_idx`: the instruction index

---

### `Nst_CatchStack`

**Synopsis:**

```better-c
typedef struct _Nst_CatchStack {
    Nst_CatchFrame *stack;
    usize len;
    usize cap;
} Nst_CatchStack
```

**Description:**

A structure representing the catch stack.

**Fields:**

- `stack`: the catch frames in the stack
- `len`: the current size of the stack
- `cap`: the maximum size of the stack before it needs to be expanded

---

### `Nst_GenericStack`

**Synopsis:**

```better-c
typedef struct _Nst_GenericStack {
    void *stack;
    usize len;
    usize cap;
} Nst_GenericStack
```

**Description:**

A structure representing a generic stack.

**Fields:**

- `stack`: the pointer to the array of elements in the stack
- `len`: the current size of the stack
- `cap`: the maximum size of the stack before it needs to be expanded

---

## Functions

### `Nst_vstack_init`

**Synopsis:**

```better-c
bool Nst_vstack_init(void)
```

**Description:**

Initializes the value stack.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `_Nst_vstack_push`

**Synopsis:**

```better-c
bool _Nst_vstack_push(Nst_Obj *obj)
```

**Description:**

Pushes a value on the value stack.

**Parameters:**

- `obj`: the value to be pushed, if not `NULL` its refcount is increased

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_vstack_pop`

**Synopsis:**

```better-c
Nst_Obj *Nst_vstack_pop(void)
```

**Description:**

Pops the top value from the value stack and returns it. If the stack is empty
`NULL` is returned. No error is set.

---

### `Nst_vstack_peek`

**Synopsis:**

```better-c
Nst_Obj *Nst_vstack_peek(void)
```

**Returns:**

The top value from the value stack. If the stack is empty `NULL` is returned. No
error is set.

---

### `Nst_vstack_dup`

**Synopsis:**

```better-c
bool Nst_vstack_dup(void)
```

**Description:**

Duplicates the top value of the stack.

If the stack is empty nothing is done.

**Returns:**

`true` on success and `false` on failure. If the stack is empty the function
always succeeds. The error is set.

---

### `Nst_vstack_destroy`

**Synopsis:**

```better-c
void Nst_vstack_destroy(void)
```

**Description:**

Destroys the value stack.

---

### `Nst_fstack_init`

**Synopsis:**

```better-c
bool Nst_fstack_init(void)
```

**Description:**

Initializes the call stack.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `_Nst_fstack_push`

**Synopsis:**

```better-c
bool _Nst_fstack_push(Nst_FuncObj *func, Nst_Pos call_start, Nst_Pos call_end,
                      Nst_VarTable *vt, i64 idx, usize cstack_size)
```

**Description:**

Pushes a call on the call stack.

**Parameters:**

- `func`: the function of the call
- `call_start`: the start position of the call
- `call_end`: the end position of the call
- `vt`: the current variable table
- `idx`: the current instruction index
- `cstack_size`: the current size of the catch stack

**Returns:**

`true` on success and `false` on failure. The error is not always set.

---

### `Nst_fstack_pop`

**Synopsis:**

```better-c
Nst_FuncCall Nst_fstack_pop(void)
```

**Description:**

Pops the top call from the call stack and returns it. If the stack is empty, a
[`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) with a `NULL` `func` and
`vt` is returned. No error is set.

---

### `Nst_fstack_peek`

**Synopsis:**

```better-c
Nst_FuncCall Nst_fstack_peek(void)
```

**Description:**

Returns the top function in the call stack. If the stack is empty, a
[`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) with a `NULL` `func` and
`vt` is returned. No error is set.

---

### `Nst_fstack_destroy`

**Synopsis:**

```better-c
void Nst_fstack_destroy(void)
```

**Description:**

Destroys the call stack.

---

### `Nst_cstack_init`

**Synopsis:**

```better-c
bool Nst_cstack_init(void)
```

**Description:**

Initializes the catch stack.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_cstack_peek`

**Synopsis:**

```better-c
Nst_CatchFrame Nst_cstack_peek(void)
```

**Description:**

Returns the top value of the catch stack. If the stack is empty a
[`Nst_CatchFrame`](c_api-runtime_stack.md#nst_catchframe) with an `inst_idx` of
`-1` is returned. No error is set.

---

### `Nst_cstack_pop`

**Synopsis:**

```better-c
Nst_CatchFrame Nst_cstack_pop(void)
```

**Description:**

Pops the top value of the catch stack and returns it. If the stack is empty a
[`Nst_CatchFrame`](c_api-runtime_stack.md#nst_catchframe) with an `inst_idx` of
`-1` is returned. No error is set.

---

### `Nst_cstack_destroy`

**Synopsis:**

```better-c
void Nst_cstack_destroy(void)
```

**Description:**

Destroys the catch stack.

---

### `Nst_stack_init`

**Synopsis:**

```better-c
bool Nst_stack_init(Nst_GenericStack *g_stack, usize unit_size,
                    usize starting_size)
```

**Description:**

Initializes a new generic stack.

**Parameters:**

- `g_stack`: the stack to initialize
- `unit_size`: the size in bytes of one element in the stack
- `starting_size`: the initial number of elements in the stack

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_stack_expand`

**Synopsis:**

```better-c
bool Nst_stack_expand(Nst_GenericStack *g_stack, usize unit_size)
```

**Description:**

Expands a generic stack if needed.

**Parameters:**

- `g_stack`: the stack to expand
- `unit_size`: the size of one element in the stack

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_stack_shrink`

**Synopsis:**

```better-c
void Nst_stack_shrink(Nst_GenericStack *g_stack, usize min_size,
                      usize unit_size)
```

**Description:**

Shrinks a runtime stack if needed.

**Parameters:**

- `g_stack`: the stack to shrink
- `min_size`: the minimum size that the stack can reach
- `unit_size`: the size of one element in the stack

