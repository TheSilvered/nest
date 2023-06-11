# `nodes.h`

This header contains the structures and functions to handle AST nodes.

## Macros

### `NST_NODE`

**Synopsis**:

```better-c
NST_NODE(expr)
```

**Description**:

Casts `expr` to `Nst_Node *`.

---

### `NST_NODE_RETUNS_VALUE`

**Synopsis**:

```better-c
NST_NODE_RETUNS_VALUE(node_type)
```

**Description**:

Checks if a node is an expression or a statement.

**Arguments**:

- `node_type`: a `Nst_NodeType`, _not_ a `Nst_Node`

---

## Structs

### `Nst_Node`

**Synopsis**:

```better-c
typedef struct _Nst_Node
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_NodeType type;
    Nst_LList *nodes;
    Nst_LList *tokens;
}
Nst_Node
```

**Description**:

The structure for a node of the abstract syntax tree.

**Fields**:

- `start`: the start position of the node
- `end`: the end position of the node
- `type`: the type of the node
- `nodes`: the child nodes
- `tokens`: the tokens it refernecs

---

## Functions

### `nst_node_new`

**Synopsis**:

```better-c
Nst_Node *nst_node_new(Nst_NodeType type, Nst_OpErr *err)
```

**Description**:

Creates a new node on the heap.

**Arguments**:

- `[in] type`: the type of the new node
- `[out] err`: the error

**Return value**:

The function returns the new node or `NULL` if an error occurs.

---

### `nst_node_new_pos`

**Synopsis**:

```better-c
Nst_Node *nst_node_new_pos(Nst_NodeType type,
                           Nst_Pos      start,
                           Nst_Pos      end,
                           Nst_OpErr   *err)
```

**Description**:

Creates a new node on the heap but setting also its `start` and `end` fields.

**Arguments**:

- `[in] type`: the type of the new node
- `[in] start`: the start position of the new node
- `[in] end`: the start position of the new node
- `[out] err`: the error

**Return value**:

The function returns the new node or `NULL` if an error occurs.

---

### `nst_node_set_pos`

**Synopsis**:

```better-c
void nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
```

**Description**:

Sets the position of a node.

**Arguments**:

- `[inout] node`: the node to set the position to
- `[in] start`: the start position
- `[in] end`: the end position

---

### `nst_node_destroy`

**Synopsis**:

```better-c
void nst_node_destroy(Nst_Node *node)
```

**Description**:

Destroys a node and all its children nodes and tokens.

**Arguments**:

- `node`: the node to destroy

---

## Enums

### `Nst_NodeType`

**Synopsis**:

```better-c
typedef enum _Nst_NodeType
{
    NST_NT_LONG_S,
    NST_NT_WHILE_L,
    NST_NT_DOWHILE_L,
    NST_NT_FOR_L,
    NST_NT_FOR_AS_L,
    NST_NT_FUNC_DECLR,
    NST_NT_RETURN_S,
    NST_NT_CONTINUE_S,
    NST_NT_BREAK_S,
    NST_NT_SWITCH_S,
    NST_NT_TRY_CATCH_S,
    NST_NT_STACK_OP,
    NST_NT_LOCAL_STACK_OP,
    NST_NT_LOCAL_OP,
    NST_NT_ARR_LIT,
    NST_NT_VEC_LIT,
    NST_NT_MAP_LIT,
    NST_NT_VALUE,
    NST_NT_ACCESS,
    NST_NT_EXTRACT_E,
    NST_NT_ASSIGN_E,
    NST_NT_IF_E,
    NST_NT_LAMBDA
}
Nst_NodeType
```

**Description**:

The enum containing all the possible node types.
