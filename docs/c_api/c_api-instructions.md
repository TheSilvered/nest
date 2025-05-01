# `instructions.h`

Bytecode instruction interface.

## Authors

TheSilvered

---

## Structs

### `Nst_Inst`

**Synopsis:**

```better-c
typedef struct _Nst_Inst {
    Nst_InstCode code;
    Nst_Span span;
    i64 val;
} Nst_Inst
```

**Description:**

A structure representing a Nest instruction.

**Fields:**

- `code`: the code of the instruction
- `span`: the position of the instruction in the code
- `val`: the integer value of the instruction

---

### `Nst_InstList`

**Synopsis:**

```better-c
typedef struct _Nst_InstList {
    Nst_DynArray instructions;
    Nst_PtrArray objects;
    Nst_DynArray functions;
} Nst_InstList
```

**Description:**

A structure representing a list of instructions.

**Fields:**

- `instructions`: the list of instructions
- `objects`: the list of objects that the instructions reference
- `functions`: the list of
  [`Nst_FuncPrototype`](c_api-instructions.md#nst_funcprototype) of the
  functions defined in the file

---

### `Nst_FuncPrototype`

**Synopsis:**

```better-c
typedef struct _Nst_FuncPrototype {
    Nst_InstList ilist;
    Nst_ObjRef **arg_names;
    usize arg_num;
} Nst_FuncPrototype
```

**Description:**

A structure representing a function before it is assembled.

**Fields:**

- `ilist`: the body of the function
- `arg_names`: the names of the arguments as an array of `Str` objects
- `arg_num`: the number of arguments the function can accept

---

## Functions

### `Nst_ic_is_jump`

**Synopsis:**

```better-c
bool Nst_ic_is_jump(Nst_InstCode code)
```

**Description:**

Check if an instruction code is a jump instruction.

---

### `Nst_ilist_init`

**Synopsis:**

```better-c
bool Nst_ilist_init(Nst_InstList *list)
```

**Description:**

Initialize an [`Nst_InstList`](c_api-instructions.md#nst_instlist).

---

### `Nst_ilist_destroy`

**Synopsis:**

```better-c
void Nst_ilist_destroy(Nst_InstList *list)
```

**Description:**

Destroy the contents of an [`Nst_InstList`](c_api-instructions.md#nst_instlist).

---

### `Nst_ilist_add`

**Synopsis:**

```better-c
bool Nst_ilist_add(Nst_InstList *list, Nst_InstCode code, Nst_Span span)
```

**Description:**

Add an instruction to an [`Nst_InstList`](c_api-instructions.md#nst_instlist).
The `val` field of the instruction is set to `0`.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_ilist_add_ex`

**Synopsis:**

```better-c
bool Nst_ilist_add_ex(Nst_InstList *list, Nst_InstCode code, i64 val,
                      Nst_Span span)
```

**Description:**

Add an instruction to an [`Nst_InstList`](c_api-instructions.md#nst_instlist)
specifying its `val` field.

**Returns:**

`true` on success and `false` on failure. The error is set.

---

### `Nst_ilist_add_obj`

**Synopsis:**

```better-c
isize Nst_ilist_add_obj(Nst_InstList *list, Nst_ObjRef *obj)
```

**Description:**

Append an object to an [`Nst_InstList`](c_api-instructions.md#nst_instlist). A
reference is taken from `obj`.

**Returns:**

The index of the added object or `-1` on failure. The error is set.

---

### `Nst_ilist_add_func`

**Synopsis:**

```better-c
isize Nst_ilist_add_func(Nst_InstList *list, Nst_FuncPrototype *fp)
```

**Description:**

Append a function prototype to an
[`Nst_InstList`](c_api-instructions.md#nst_instlist).

**Returns:**

The index of the added prototype or `-1` on failure. The error is set.

---

### `Nst_ilist_get_inst`

**Synopsis:**

```better-c
Nst_Inst *Nst_ilist_get_inst(Nst_InstList *list, usize idx)
```

**Returns:**

The instruction at index `idx` of an
[`Nst_InstList`](c_api-instructions.md#nst_instlist). If the index is out of
bounds `NULL` is returned. No error is set.

---

### `Nst_ilist_get_obj`

**Synopsis:**

```better-c
Nst_Obj *Nst_ilist_get_obj(Nst_InstList *list, usize idx)
```

**Returns:**

The object at index `idx` of an
[`Nst_InstList`](c_api-instructions.md#nst_instlist). If the index is out of
bounds `NULL` is returned. No error is set.

---

### `Nst_ilist_get_func`

**Synopsis:**

```better-c
Nst_FuncPrototype *Nst_ilist_get_func(Nst_InstList *list, usize idx)
```

**Returns:**

The function prototype at index `idx` of an
[`Nst_InstList`](c_api-instructions.md#nst_instlist). If the index is out of
bounds `NULL` is returned. No error is set.

---

### `Nst_ilist_get_inst_obj`

**Synopsis:**

```better-c
Nst_Obj *Nst_ilist_get_inst_obj(Nst_InstList *list, usize idx)
```

**Returns:**

The object associated with the instruction at `idx`. The equivalent of getting
an instruction with
[`Nst_ilist_get_inst`](c_api-instructions.md#nst_ilist_get_inst) and then
getting the object with
[`Nst_ilist_get_obj`](c_api-instructions.md#nst_ilist_get_obj) using the value
of the instruction as the index.

---

### `Nst_ilist_get_inst_func`

**Synopsis:**

```better-c
Nst_FuncPrototype *Nst_ilist_get_inst_func(Nst_InstList *list, usize idx)
```

**Returns:**

The function prototype associated with the instruction at `idx`. The equivalent
of getting an instruction with
[`Nst_ilist_get_inst`](c_api-instructions.md#nst_ilist_get_inst) and then
getting the prototype with
[`Nst_ilist_get_func`](c_api-instructions.md#nst_ilist_get_func) using the value
of the instruction as the index.

---

### `Nst_ilist_set`

**Synopsis:**

```better-c
void Nst_ilist_set(Nst_InstList *list, usize idx, Nst_InstCode code)
```

**Description:**

Set the code of the instruction at index `idx`. Its value is set to `0`.

---

### `Nst_ilist_set_ex`

**Synopsis:**

```better-c
void Nst_ilist_set_ex(Nst_InstList *list, usize idx, Nst_InstCode code,
                      i64 val)
```

**Description:**

Set the code and the value of the instruction at index `idx`.

---

### `Nst_ilist_len`

**Synopsis:**

```better-c
usize Nst_ilist_len(Nst_InstList *list)
```

**Returns:**

The number of instructions in an
[`Nst_InstList`](c_api-instructions.md#nst_instlist).

---

### `Nst_ilist_print`

**Synopsis:**

```better-c
void Nst_ilist_print(Nst_InstList *list)
```

**Description:**

Print an [`Nst_InstList`](c_api-instructions.md#nst_instlist) to the standard
output.

---

### `Nst_fprototype_init`

**Synopsis:**

```better-c
bool Nst_fprototype_init(Nst_FuncPrototype *fp, Nst_InstList ls, usize arg_num)
```

**Description:**

Initialize a [`Nst_FuncPrototype`](c_api-instructions.md#nst_funcprototype).

---

### `Nst_fprototype_destroy`

**Synopsis:**

```better-c
void Nst_fprototype_destroy(Nst_FuncPrototype *fp)
```

**Description:**

Destroy the contents of a
[`Nst_FuncPrototype`](c_api-instructions.md#nst_funcprototype).

---

## Enums

### `Nst_InstCode`

**Synopsis:**

```better-c
typedef enum _Nst_InstCode {
    Nst_IC_NO_OP,
    Nst_IC_POP_VAL,
    Nst_IC_FOR_START,
    Nst_IC_FOR_NEXT,
    Nst_IC_RETURN_VAL,
    Nst_IC_RETURN_VARS,
    Nst_IC_SET_VAL_LOC,
    Nst_IC_SET_CONT_LOC,
    Nst_IC_THROW_ERR,
    Nst_IC_POP_CATCH,
    Nst_IC_SET_VAL,
    Nst_IC_GET_VAL,
    Nst_IC_PUSH_VAL,
    Nst_IC_SET_CONT_VAL,
    Nst_IC_OP_CALL,
    Nst_IC_OP_SEQ_CALL,
    Nst_IC_OP_CAST,
    Nst_IC_OP_RANGE,
    Nst_IC_STACK_OP,
    Nst_IC_LOCAL_OP,
    Nst_IC_OP_IMPORT,
    Nst_IC_OP_EXTRACT,
    Nst_IC_DEC_INT,
    Nst_IC_NEW_INT,
    Nst_IC_DUP,
    Nst_IC_ROT_2,
    Nst_IC_ROT_3,
    Nst_IC_MAKE_ARR,
    Nst_IC_MAKE_ARR_REP,
    Nst_IC_MAKE_VEC,
    Nst_IC_MAKE_VEC_REP,
    Nst_IC_MAKE_MAP,
    Nst_IC_MAKE_FUNC,
    Nst_IC_SAVE_ERROR,
    Nst_IC_UNPACK_SEQ,
    Nst_IC_JUMP,
    Nst_IC_JUMPIF_T,
    Nst_IC_JUMPIF_F,
    Nst_IC_JUMPIF_ZERO,
    Nst_IC_JUMPIF_IEND,
    Nst_IC_PUSH_CATCH
} Nst_InstCode
```

**Description:**

The code of a [`Nst_Inst`](c_api-instructions.md#nst_inst).
