# `runtime_stack.h`

Stacks used for the VM during runtime.

## Authors

TheSilvered

---

## Structs

### `Nst_ValueStack`

**Synopsis:**

```better-c
typedef struct _Nst_ValueStack {
    Nst_ObjRef **stack;
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
    Nst_ObjRef *func;
    Nst_Span span;
    Nst_VarTable vt;
    i64 idx;
    usize cstack_len;
} Nst_FuncCall
```

**Description:**

A structure representing a function call.

**Fields:**

- `func`: the function being called
- `span`: the position of the call
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
    usize max_recursion_depth;
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
    i64 idx;
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

## Functions

### `Nst_vstack_init`

**Synopsis:**

```better-c
bool Nst_vstack_init(Nst_ValueStack *v_stack)
```

**Description:**

Initialize a value stack.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_vstack_push`

**Synopsis:**

```better-c
bool Nst_vstack_push(Nst_ValueStack *v_stack, Nst_Obj *obj)
```

**Description:**

Push a value on a value stack.

**Parameters:**

- `v_stack`: the value stack push the object onto
- `obj`: the value to be pushed, if not `NULL` its refcount is increased

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_vstack_pop`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_vstack_pop(Nst_ValueStack *v_stack)
```

**Description:**

Pop the top value from a value stack.

**Parameters:**

- `v_stack`: the value stack to pop the value from

**Returns:**

The popped value. If the stack is empty `NULL` is returned. No error is set.

---

### `Nst_vstack_peek`

**Synopsis:**

```better-c
Nst_Obj *Nst_vstack_peek(Nst_ValueStack *v_stack)
```

**Description:**

Peek at the top value of a value stack.

**Parameters:**

- `v_stack`: the value stack to peek from

**Returns:**

The top value from the value stack. If the stack is empty `NULL` is returned. No
error is set.

---

### `Nst_vstack_dup`

**Synopsis:**

```better-c
bool Nst_vstack_dup(Nst_ValueStack *v_stack)
```

**Description:**

Duplicate the top value of a value stack.

If the stack is empty nothing is done.

**Parameters:**

- `v_stack`: the value stack to duplicate the value of

**Returns:**

`true` on success and `false` on failure. If the stack is empty the function
always succeeds. The error is set.

---

### `Nst_vstack_destroy`

**Synopsis:**

```better-c
void Nst_vstack_destroy(Nst_ValueStack *v_stack)
```

**Description:**

Destroy the contents of a value stack.

---

### `Nst_fstack_init`

**Synopsis:**

```better-c
bool Nst_fstack_init(Nst_CallStack *f_stack)
```

**Description:**

Initialize a call stack.

**Parameters:**

- `f_stack`: the call stack to initialize

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_fstack_push`

**Synopsis:**

```better-c
bool Nst_fstack_push(Nst_CallStack *f_stack, Nst_FuncCall call)
```

**Description:**

Push a call on a call stack.

!!!note
    The reference count of the function inside `call` is automatically
    increased. `func` may still be `NULL`.

**Parameters:**

- `f_stack`: the call stack to push the call onto
- `call`: the call to push on the stack

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_fstack_pop`

**Synopsis:**

```better-c
Nst_FuncCall Nst_fstack_pop(Nst_CallStack *f_stack)
```

**Description:**

Pop the top call from a call stack

**Parameters:**

- `f_stack`: the call stack to pop the value from

**Returns:**

The popped value. If the stack is empty, a
[`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) with a `NULL` `func` and
`vt` is returned. No error is set.

---

### `Nst_fstack_peek`

**Synopsis:**

```better-c
Nst_FuncCall Nst_fstack_peek(Nst_CallStack *f_stack)
```

**Description:**

Peek at the top call of a call stack.

**Parameters:**

- `f_stack`: the call stack to peek from

**Returns:**

The top function in the call stack. If the stack is empty, a
[`Nst_FuncCall`](c_api-runtime_stack.md#nst_funccall) with a `NULL` `func` and
`vt` is returned. No error is set.

---

### `Nst_fstack_destroy`

**Synopsis:**

```better-c
void Nst_fstack_destroy(Nst_CallStack *f_stack)
```

**Description:**

Destroy the contents of a call stack.

---

### `Nst_cstack_init`

**Synopsis:**

```better-c
bool Nst_cstack_init(Nst_CatchStack *c_stack)
```

**Description:**

Initialize a catch stack.

**Parameters:**

- `c_stack`: the catch stack to initialize

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_cstack_push`

**Synopsis:**

```better-c
bool Nst_cstack_push(Nst_CatchStack *c_stack, Nst_CatchFrame frame)
```

**Description:**

Push a frame on a catch stack.

**Parameters:**

- `c_stack`: the catch stack to push the frame onto
- `frame`: the [`Nst_CatchFrame`](c_api-runtime_stack.md#nst_catchframe) to push
  on the stack

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_cstack_peek`

**Synopsis:**

```better-c
Nst_CatchFrame Nst_cstack_peek(Nst_CatchStack *c_stack)
```

**Description:**

Peek at the top frame of a catch stack.

**Parameters:**

- `c_stack`: the catch stack to peek from

**Returns:**

The top value of the catch stack. If the stack is empty a
[`Nst_CatchFrame`](c_api-runtime_stack.md#nst_catchframe) with an `inst_idx` of
`-1` is returned. No error is set.

---

### `Nst_cstack_pop`

**Synopsis:**

```better-c
Nst_CatchFrame Nst_cstack_pop(Nst_CatchStack *c_stack)
```

**Description:**

Pop the top value of a catch stack.

**Parameters:**

- `c_stack`: the catch stack to pop the frame from

**Returns:**

The popped frame. If the stack is empty a
[`Nst_CatchFrame`](c_api-runtime_stack.md#nst_catchframe) with an `inst_idx` of
`-1` is returned. No error is set.

---

### `Nst_cstack_destroy`

**Synopsis:**

```better-c
void Nst_cstack_destroy(Nst_CatchStack *c_stack)
```

**Description:**

Destroy the contents of a catch stack.
