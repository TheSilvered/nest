# `instructions.h`

This header contains the bytecode instruction interface.

## Macros

### `NST_INST_IS_JUMP`

**Synopsis**:

```better-c
NST_INST_IS_JUMP(inst)
```

**Description**:

Checks if an instruction jumps in the code.

---

### `nst_inst_new_val`

**Synopsis**:

```better-c
nst_inst_new_val(id, val, start, end, err)
```

**Description**:

Calls `_nst_inst_new_val` casting `val` to a `Nst_Obj *`.

---

## Enums

### `Nst_InstID`

**Synopsis**:

```better-c
typedef enum _Nst_InstID
{
    NST_IC_NO_OP,
    NST_IC_POP_VAL,
    NST_IC_FOR_START,
    NST_IC_FOR_IS_DONE,
    NST_IC_FOR_GET_VAL,
    NST_IC_RETURN_VAL,
    NST_IC_RETURN_VARS,
    NST_IC_SET_VAL_LOC,
    NST_IC_SET_CONT_LOC,
    NST_IC_JUMP,
    NST_IC_JUMPIF_T,
    NST_IC_JUMPIF_F,
    NST_IC_JUMPIF_ZERO,
    NST_IC_TYPE_CHECK,
    NST_IC_HASH_CHECK,
    NST_IC_THROW_ERR,
    NST_IC_PUSH_CATCH,
    NST_IC_POP_CATCH,

    // These instruction push a value on the stack
    NST_IC_SET_VAL,
    NST_IC_GET_VAL,
    NST_IC_PUSH_VAL,
    NST_IC_SET_CONT_VAL,
    NST_IC_OP_CALL,
    NST_IC_OP_CAST,
    NST_IC_OP_RANGE,
    NST_IC_STACK_OP,
    NST_IC_LOCAL_OP,
    NST_IC_OP_IMPORT,
    NST_IC_OP_EXTRACT,
    NST_IC_DEC_INT,
    NST_IC_NEW_OBJ,
    NST_IC_DUP,
    NST_IC_ROT,
    NST_IC_MAKE_ARR,
    NST_IC_MAKE_ARR_REP,
    NST_IC_MAKE_VEC,
    NST_IC_MAKE_VEC_REP,
    NST_IC_MAKE_MAP,
    NST_IC_SAVE_ERROR,
    NST_IC_UNPACK_SEQ
}
Nst_InstID
```

**Instructions**:

- `NST_IC_DEC_INT`: decreases the top value by 1 in-place
- `NST_IC_DUP`: duplicates the top value
- `NST_IC_FOR_GET_VAL`: calls the `_get_val_` function of an iterator
- `NST_IC_FOR_IS_DONE`: calls the `_is_done_` function of an iterator
- `NST_IC_FOR_START`: calls the `_start_` function of an iterator
- `NST_IC_GET_VAL`: gets the value of a variable
- `NST_IC_HASH_CHECK`: checks that an object is hashable
- `NST_IC_JUMP`: absolute jump to a certain index
- `NST_IC_JUMPIF_F`: jumps to index `i` if the top value is falsy
- `NST_IC_JUMPIF_T`: jumps to index `i`  if the top value is truthy
- `NST_IC_JUMPIF_ZERO`: jumps to index `i` if the top value is zero
- `NST_IC_LOCAL_OP`: local operator
- `NST_IC_MAKE_ARR`: creates an array
- `NST_IC_MAKE_ARR_REP`: creates an array repeating the value
- `NST_IC_MAKE_MAP`: creates a map
- `NST_IC_MAKE_VEC`: creates a vector
- `NST_IC_MAKE_VEC_REP`: creates a vector repeating the value
- `NST_IC_NEW_OBJ`: duplicates the top value creating a new object
- `NST_IC_NO_OP`: does nothing
- `NST_IC_OP_CALL`: calls the function at the top of the stack
- `NST_IC_OP_CAST`: casts a value to another type
- `NST_IC_OP_EXTRACT`: extraction
- `NST_IC_OP_IMPORT`: imports the library with the path at the top of the stack
- `NST_IC_OP_RANGE`: creates a range iterator
- `NST_IC_POP_CATCH`: pops the top catch frame from the catch stack
- `NST_IC_POP_VAL`: pops the top value from the value stack
- `NST_IC_PUSH_CATCH`: pushes a catch frame on the catch stack
- `NST_IC_PUSH_VAL`: pushes a value on the value stack
- `NST_IC_RETURN_VAL`: returns the top value
- `NST_IC_RETURN_VARS`: used in libraries to return `_vars_`
- `NST_IC_ROT`: rotates the top N elements of the stack
- `NST_IC_SAVE_ERROR`: saves the caught error as a map
- `NST_IC_SET_CONT_LOC`: sets the value of a container removing it from the stack
- `NST_IC_SET_CONT_VAL`: sets the value of a container
- `NST_IC_SET_VAL`: sets the value of a variable
- `NST_IC_SET_VAL_LOC`: sets the value of a variable removing it from the stack
- `NST_IC_STACK_OP`: stack operator
- `NST_IC_THROW_ERR`: throws an error
- `NST_IC_TYPE_CHECK`: checks the type of the top value
- `NST_IC_UNPACK_SEQ`: unpacks a sequence

---

## Structs

### `Nst_Inst`

**Synopsis**:

```better-c
typedef struct _Nst_Instruction
{
    Nst_InstID id;
    Nst_Int int_val;
    Nst_Obj *val;
    Nst_Pos start;
    Nst_Pos end;
}
Nst_Inst
```

**Description**:

The structure defining a single Nest instruction

**Fields**:

- `id`: the instruction code
- `int_val`: the integer value of the instruction used by e.g. `NST_IC_JUMP`
- `val`: the object value used by e.g. `NST_PUSH_VAL`
- `start`: the start position of the instruction
- `end`: the end position of the instruction

---

### `Nst_InstList`

**Synopsis**:

```better-c
typedef struct _Nst_InstList
{
    usize total_size;
    Nst_Inst *instructions;
    Nst_LList *functions;
}
Nst_InstList
```

**Description**:

A structure containing the list of instructions.

**Fields**:

- `total_size`: the total number of instructions contained in the list
- `instructions`: the instruction array
- `functions`: the functions defined inside the instructions

---

## Functions

### `nst_inst_new`

**Synopsis**:

```better-c
Nst_Inst *nst_inst_new(Nst_InstID id, Nst_Pos start, Nst_Pos end, Nst_OpErr *err)
```

**Description**:

Creates a new `Nst_Inst` on the heap.

**Arguments**:

- `[in] id`: the instruction code
- `[in] start`: the start position of the instruction
- `[in] end`: the end position of the instruction
- `[out] err`: the error

**Return value**:

The function returns a new instruction or `NULL` if it fails.

---

### `_nst_inst_new_val`

**Synopsis**:

```better-c
Nst_Inst *_nst_inst_new_val(Nst_InstID id,
                            Nst_Obj   *val,
                            Nst_Pos    start,
                            Nst_Pos    end,
                            Nst_OpErr *err)
```

**Description**:

Creates a new `Nst_Inst` on the heap setting the `val` field.

**Arguments**:

- `[in] id`: the instruction code
- `[in] val`: the value to be contained by the instruction
- `[in] start`: the start position of the instruction
- `[in] end`: the end position of the instruction
- `[out] err`: the error

**Return value**:

The function returns a new instruction or `NULL` if it fails.

---

### `nst_inst_new_int`

**Synopsis**:

```better-c
Nst_Inst *nst_inst_new_int(Nst_InstID id,
                           Nst_Int    int_val,
                           Nst_Pos    start,
                           Nst_Pos    end,
                           Nst_OpErr *err)
```

**Description**:

Creates a new `Nst_Inst` on the heap setting the `int_val` field.

**Arguments**:

- `[in] id`: the instruction code
- `[in] int_val`: the integer value to be contained by the instruction
- `[in] start`: the start position of the instruction
- `[in] end`: the end position of the instruction
- `[out] err`: the error

**Return value**:

The function returns a new instruction or `NULL` if it fails.

---

### `nst_inst_destroy`

**Synopsis**:

```better-c
void nst_inst_destroy(Nst_Inst *inst)
```

**Description**:

Frees a heap allocated `Nst_Inst`.

---

### `nst_inst_list_destroy`

**Synopsis**:

```better-c
void nst_inst_list_destroy(Nst_InstList *inst_list)
```

**Description**:

Frees a heap allocated `Nst_InstList`.
