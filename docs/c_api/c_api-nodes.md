# `nodes.h`

Nodes of the AST.

## Authors

TheSilvered

## Macros

### `Nst_NODE`

**Synopsis:**

```better-c
Nst_NODE(expr)
```

**Description:**

Casts `expr` to [`Nst_Node *`](c_api-nodes.md/#nst_node).

---

### `Nst_NODE_RETUNS_VALUE`

**Synopsis:**

```better-c
Nst_NODE_RETUNS_VALUE(node_type)
```

**Description:**

Evaluates to `true` if the specified node type returns a value.

---

## Structs

### `Nst_Node`

**Synopsis:**

```better-c
typedef struct _Nst_Node {
    Nst_Pos start;
    Nst_Pos end;
    struct _Nst_NodeType type;
    Nst_LList *nodes;
    Nst_LList *tokens;
} struct _Nst_Node
```

**Description:**

A structure representing a node.

**Fields:**

- `start`: the starting position of the node
- `end`: the ending position of the node
- `type`: the type of the node
- `nodes`: the child nodes of the node
- `tokens`: the tokens that the node contains

---

## Functions

### `Nst_node_new`

**Synopsis:**

```better-c
Nst_Node *Nst_node_new(Nst_NodeType type)
```

**Description:**

Creates and initializes a new AST node on the heap.

Though the position is not in the arguments, its position should be set later
with [`Nst_node_set_pos`](c_api-nodes.md/#nst_node_set_pos).

**Parameters:**

- `type`: the type of the node to initialize

**Returns:**

The new node or `NULL` on failure. The error is set.

---

### `Nst_node_new_pos`

**Synopsis:**

```better-c
Nst_Node *Nst_node_new_pos(Nst_NodeType type, Nst_Pos start, Nst_Pos end)
```

**Description:**

Creates and initializes a new AST node on the heap.

**Parameters:**

- `type`: the type of the node to initialize
- `start`: the start position of the node
- `end`: the end position of the node

**Returns:**

The new node or `NULL` on failure. The error is set.

---

### `Nst_node_set_pos`

**Synopsis:**

```better-c
void Nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
```

**Description:**

Sets the position of a node.

---

### `Nst_node_destroy`

**Synopsis:**

```better-c
void Nst_node_destroy(Nst_Node *node)
```

**Description:**

Destroys a node and its children.

---

## Enums

### `Nst_NodeType`

**Synopsis:**

```better-c
typedef enum _Nst_NodeType {
    Nst_NT_LONG_S,
    Nst_NT_WHILE_L,
    Nst_NT_DOWHILE_L,
    Nst_NT_FOR_L,
    Nst_NT_FOR_AS_L,
    Nst_NT_FUNC_DECLR,
    Nst_NT_RETURN_S,
    Nst_NT_CONTINUE_S,
    Nst_NT_BREAK_S,
    Nst_NT_SWITCH_S,
    Nst_NT_TRY_CATCH_S,

    // These nodes return a value

    Nst_NT_STACK_OP,
    Nst_NT_LOCAL_STACK_OP,
    Nst_NT_LOCAL_OP,
    Nst_NT_ARR_LIT,
    Nst_NT_VEC_LIT,
    Nst_NT_MAP_LIT,
    Nst_NT_VALUE,
    Nst_NT_ACCESS,
    Nst_NT_EXTRACT_E,
    Nst_NT_ASSIGN_E,
    Nst_NT_IF_E,
    Nst_NT_LAMBDA
} struct _Nst_NodeType
```

**Description:**

The types of nodes in the AST.

