# `instructions.h`

Bytecode instruction interface.

## Authors

TheSilvered

## Macros

### `Nst_INST_IS_JUMP`

**Synopsis:**

```better-c
Nst_INST_IS_JUMP(inst_id)
```

**Description:**

Checks whether a given instruction ID represents a jump instruction.

---

### `Nst_inst_new_val`

**Synopsis:**

```better-c
Nst_inst_new_val(id, val, start, end)
```

**Description:**

Alias for [`_Nst_inst_new_val`](c_api-instructions.md/#_nst_inst_new_val) that
casts `val` to [`Nst_Obj *`](c_api-obj.md/#nst_obj).

---

## Structs

### `Nst_Inst`

**Synopsis:**

```better-c
typedef struct _Nst_Inst {
    struct _Nst_InstID id;
    i64 int_val;
    Nst_Obj *val;
    Nst_Pos start;
    Nst_Pos end;
} Nst_Inst
```

**Description:**

The structure representing an instruction in Nest.

**Fields:**

- `id`: the ID of the instruction
- `int_val`: an integer value used by the instruction
- `val`: an object used by the instruction
- `start`: the start position of the instruction
- `end`: the end position of the instruction

---

### `Nst_InstList`

**Synopsis:**

```better-c
typedef struct _Nst_InstList {
    usize total_size;
    Nst_Inst *instructions;
    Nst_LList *functions;
} Nst_InstList
```

**Description:**

The structure representing a list of instructions in Nest.

**Fields:**

- `total_size`: the total number of instructions in the list
- `instructions`: the array of instructions
- `functions`: the list of functions declared withing the bytecode

---

## Functions

### `Nst_inst_new`

**Synopsis:**

```better-c
Nst_Inst *Nst_inst_new(Nst_InstID id, Nst_Pos start, Nst_Pos end)
```

**Description:**

Creates a new instruction on the heap.

**Parameters:**

- `id`: the id of the instruction to create
- `start`: the start position of the instruction
- `end`: the end position of the instruction

**Returns:**

The new instruction or `NULL` on failure. The error is set.

---

### `_Nst_inst_new_val`

**Synopsis:**

```better-c
Nst_Inst *_Nst_inst_new_val(Nst_InstID id, Nst_Obj *val, Nst_Pos start,
                            Nst_Pos end)
```

**Description:**

Creates a new instruction on the heap with a Nest object value.

The reference count of `val` is increased.

**Parameters:**

- `id`: the id of the instruction to create
- `val`: the Nest object value
- `start`: the start position of the instruction
- `end`: the end position of the instruction

**Returns:**

The new instruction or `NULL` on failure. The error is set.

---

### `Nst_inst_new_int`

**Synopsis:**

```better-c
Nst_Inst *Nst_inst_new_int(Nst_InstID id, i64 int_val, Nst_Pos start,
                           Nst_Pos end)
```

**Description:**

Creates a new instruction on the heap with an integer value.

**Parameters:**

- `id`: the id of the instruction to create
- `int_val`: the integer value
- `start`: the start position of the instruction
- `end`: the end position of the instruction

**Returns:**

The new instruction or `NULL` on failure. The error is set.

---

### `Nst_inst_destroy`

**Synopsis:**

```better-c
void Nst_inst_destroy(Nst_Inst *inst)
```

**Description:**

Destroys a [`Nst_Inst`](c_api-instructions.md/#nst_inst) allocated on the heap.

---

### `Nst_inst_list_destroy`

**Synopsis:**

```better-c
void Nst_inst_list_destroy(Nst_InstList *inst_list)
```

**Description:**

Destroys a [`Nst_InstList`](c_api-instructions.md/#nst_instlist).

---

## Enums

### `Nst_InstID`

**Synopsis:**

```better-c
typedef enum _Nst_InstID {
    Nst_IC_NO_OP,
    Nst_IC_POP_VAL,
    Nst_IC_FOR_START,
    Nst_IC_FOR_IS_DONE,
    Nst_IC_FOR_GET_VAL,
    Nst_IC_RETURN_VAL,
    Nst_IC_RETURN_VARS,
    Nst_IC_SET_VAL_LOC,
    Nst_IC_SET_CONT_LOC,
    Nst_IC_JUMP,
    Nst_IC_JUMPIF_T,
    Nst_IC_JUMPIF_F,
    Nst_IC_JUMPIF_ZERO,
    Nst_IC_PUSH_CATCH,
    Nst_IC_TYPE_CHECK,
    Nst_IC_HASH_CHECK,
    Nst_IC_THROW_ERR,
    Nst_IC_POP_CATCH,
    Nst_IC_SET_VAL,
    Nst_IC_GET_VAL,
    Nst_IC_PUSH_VAL,
    Nst_IC_SET_CONT_VAL,
    Nst_IC_OP_CALL,
    Nst_IC_OP_CAST,
    Nst_IC_OP_RANGE,
    Nst_IC_STACK_OP,
    Nst_IC_LOCAL_OP,
    Nst_IC_OP_IMPORT,
    Nst_IC_OP_EXTRACT,
    Nst_IC_DEC_INT,
    Nst_IC_NEW_OBJ,
    Nst_IC_DUP,
    Nst_IC_ROT,
    Nst_IC_MAKE_ARR,
    Nst_IC_MAKE_ARR_REP,
    Nst_IC_MAKE_VEC,
    Nst_IC_MAKE_VEC_REP,
    Nst_IC_MAKE_MAP,
    Nst_IC_SAVE_ERROR,
    Nst_IC_UNPACK_SEQ
} Nst_InstID
```

**Description:**

Instruction IDs in the Nest virtual machine.

