# `assembler.h`

No description.

## Authors

TheSilvered

---

## Structs

### `Nst_Bytecode`

**Synopsis:**

```better-c
typedef struct _Nst_Bytecode {
    usize copy_count;
    usize len;
    Nst_Op *bytecode;
    Nst_Span *positions;
    usize obj_len;
    Nst_ObjRef **objects;
} Nst_Bytecode
```

**Description:**

The structure representing Nest bytecode.

**Fields:**

- `copy_count`: the number of times it has been copied
- `len`: the total number of instructions
- `bytecode`: bytecode instructions
- `positions`: the positions of the instructions
- `obj_count`: the number of objects in `objects`
- `objects`: the array of objects used by the bytecode

---

## Type aliases

### `Nst_Op`

**Synopsis:**

```better-c
typedef u16 Nst_Op
```

**Description:**

A bytecode instruction.

---

## Functions

### `Nst_assemble`

**Synopsis:**

```better-c
Nst_Bytecode *Nst_assemble(Nst_InstList *ilist)
```

**Description:**

Assemble an [`Nst_InstList`](c_api-instructions.md#nst_instlist) into bytecode.
The bytecode is heap allocated and must be freed with
[`Nst_bc_destroy`](c_api-assembler.md#nst_bc_destroy).

**Returns:**

The new bytecode or `NULL` on failure. The error is set.

---

### `Nst_bc_copy`

**Synopsis:**

```better-c
Nst_Bytecode *Nst_bc_copy(Nst_Bytecode *bc)
```

**Description:**

Make a copy of an [`Nst_Bytecode`](c_api-assembler.md#nst_bytecode). Success is
guaranteed.

---

### `Nst_bc_destroy`

**Synopsis:**

```better-c
void Nst_bc_destroy(Nst_Bytecode *bc)
```

**Description:**

Destroy and free an [`Nst_Bytecode`](c_api-assembler.md#nst_bytecode). This
function must be called for every copy made.

---

### `Nst_bc_print`

**Synopsis:**

```better-c
void Nst_bc_print(Nst_Bytecode *bc)
```

**Description:**

Print the bytecode to the standard output.

---

## Enums

### `Nst_OpCode`

**Synopsis:**

```better-c
typedef enum _Nst_OpCode {
    Nst_OP_POP_VAL,
    Nst_OP_FOR_START,
    Nst_OP_FOR_NEXT,
    Nst_OP_RETURN_VAL,
    Nst_OP_RETURN_VARS,
    Nst_OP_SET_VAL_LOC,
    Nst_OP_SET_CONT_LOC,
    Nst_OP_THROW_ERR,
    Nst_OP_POP_CATCH,
    Nst_OP_SET_VAL,
    Nst_OP_GET_VAL,
    Nst_OP_PUSH_VAL,
    Nst_OP_SET_CONT_VAL,
    Nst_OP_CALL,
    Nst_OP_SEQ_CALL,
    Nst_OP_CAST,
    Nst_OP_RANGE,
    Nst_OP_STACK,
    Nst_OP_LOCAL,
    Nst_OP_IMPORT,
    Nst_OP_EXTRACT,
    Nst_OP_DEC_INT,
    Nst_OP_NEW_INT,
    Nst_OP_DUP,
    Nst_OP_ROT_2,
    Nst_OP_ROT_3,
    Nst_OP_MAKE_ARR,
    Nst_OP_MAKE_ARR_REP,
    Nst_OP_MAKE_VEC,
    Nst_OP_MAKE_VEC_REP,
    Nst_OP_MAKE_MAP,
    Nst_OP_MAKE_FUNC,
    Nst_OP_SAVE_ERROR,
    Nst_OP_UNPACK_SEQ,
    Nst_OP_EXTEND_ARG,
    Nst_OP_JUMP,
    Nst_OP_JUMPIF_T,
    Nst_OP_JUMPIF_F,
    Nst_OP_JUMPIF_ZERO,
    Nst_OP_JUMPIF_IEND,
    Nst_OP_PUSH_CATCH
} Nst_OpCode
```

**Description:**

Instruction IDs in the Nest virtual machine.
