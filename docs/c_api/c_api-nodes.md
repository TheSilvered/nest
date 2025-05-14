# `nodes.h`

Nodes of the AST.

## Authors

TheSilvered

---

## Macros

### `Nst_NODE_RETUNS_VALUE`

**Synopsis:**

```better-c
#define Nst_NODE_RETUNS_VALUE(node_type)
```

**Description:**

Evaluates to `true` if the specified node type returns a value.

---

## Structs

### `Nst_NodeData_SList`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SList {
    Nst_PtrArray statements;
} Nst_NodeData_SList
```

**Description:**

The data for a [`Nst_NT_S_LIST`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SWhileLp`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SWhileLp {
    Nst_Node *condition;
    Nst_Node *body;
    bool is_dowhile;
} Nst_NodeData_SWhileLp
```

**Description:**

The data for a [`Nst_NT_S_WHILE_LP`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SForLp`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SForLp {
    Nst_Node *iterator;
    Nst_Node *assignment;
    Nst_Node *body;
} Nst_NodeData_SForLp
```

**Description:**

The data for a [`Nst_NT_S_FOR_LP`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SFnDecl`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SFnDecl {
    Nst_ObjRef *name;
    Nst_PtrArray argument_names;
    Nst_Node *body;
} Nst_NodeData_SFnDecl
```

**Description:**

The data for a [`Nst_NT_S_FN_DECL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SReturn`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SReturn {
    Nst_Node *value;
} Nst_NodeData_SReturn
```

**Description:**

The data for a [`Nst_NT_S_RETURN`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SSwitch`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SSwitch {
    Nst_Node *expr;
    Nst_PtrArray values;
    Nst_PtrArray bodies;
    Nst_Node *default_body;
} Nst_NodeData_SSwitch
```

**Description:**

The data for a [`Nst_NT_S_SWITCH`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_STryCatch`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_STryCatch {
    Nst_Node *try_body;
    Nst_Node *catch_body;
    Nst_ObjRef *error_name;
} Nst_NodeData_STryCatch
```

**Description:**

The data for a [`Nst_NT_S_TRY_CATCH`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_SWrapper`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_SWrapper {
    Nst_Node *statement;
} Nst_NodeData_SWrapper
```

**Description:**

The data for a [`Nst_NT_S_WRAPPER`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EStackOp`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EStackOp {
    Nst_PtrArray values;
    Nst_TokType op;
} Nst_NodeData_EStackOp
```

**Description:**

The data for a [`Nst_NT_E_STACK_OP`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_ELocStackOp`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_ELocStackOp {
    Nst_PtrArray values;
    Nst_Node *special_value;
    Nst_TokType op;
} Nst_NodeData_ELocStackOp
```

**Description:**

The data for a [`Nst_NT_E_LOC_STACK_OP`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_ELocalOp`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_ELocalOp {
    Nst_Node *value;
    Nst_TokType op;
} Nst_NodeData_ELocalOp
```

**Description:**

The data for a [`Nst_NT_E_LOCAL_OP`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_ESeqLiteral`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_ESeqLiteral {
    Nst_PtrArray values;
    Nst_SeqNodeType type;
} Nst_NodeData_ESeqLiteral
```

**Description:**

The data for a [`Nst_NT_E_SEQ_LITERAL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EMapLiteral`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EMapLiteral {
    Nst_PtrArray keys;
    Nst_PtrArray values;
} Nst_NodeData_EMapLiteral
```

**Description:**

The data for a [`Nst_NT_E_MAP_LITERAL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EValue`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EValue {
    Nst_ObjRef *value;
} Nst_NodeData_EValue
```

**Description:**

The data for a [`Nst_NT_E_VALUE`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EAccess`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EAccess {
    Nst_Obj *value;
} Nst_NodeData_EAccess
```

**Description:**

The data for a [`Nst_NT_E_ACCESS`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EExtraction`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EExtraction {
    Nst_Node *container;
    Nst_Node *key;
} Nst_NodeData_EExtraction
```

**Description:**

The data for a [`Nst_NT_E_EXTRACTION`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EAssignment`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EAssignment {
    Nst_Node *value;
    Nst_Node *name;
} Nst_NodeData_EAssignment
```

**Description:**

The data for a [`Nst_NT_E_ASSIGNMENT`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_ECompAssign`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_ECompAssign {
    Nst_PtrArray values;
    Nst_Node *name;
    Nst_TokType op;
} Nst_NodeData_ECompAssign
```

**Description:**

The data for a [`Nst_NT_E_COMP_ASSIGN`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EIf`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EIf {
    Nst_Node *condition;
    Nst_Node *body_if_true;
    Nst_Node *body_if_false;
} Nst_NodeData_EIf
```

**Description:**

The data for a [`Nst_NT_E_IF`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_EWrapper`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_EWrapper {
    Nst_Node *expr;
} Nst_NodeData_EWrapper
```

**Description:**

The data for a [`Nst_NT_E_WRAPPER`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_Node`

**Synopsis:**

```better-c
typedef struct _Nst_Node {
    Nst_Span span;
    Nst_NodeType type;
    union {
        Nst_NodeData_SList s_list;
        Nst_NodeData_SWhileLp s_while_lp;
        Nst_NodeData_SForLp s_for_lp;
        Nst_NodeData_SFnDecl s_fn_decl;
        Nst_NodeData_SReturn s_return;
        Nst_NodeData_SSwitch s_switch;
        Nst_NodeData_STryCatch s_try_catch;
        Nst_NodeData_SWrapper s_wrapper;
        Nst_NodeData_EStackOp e_stack_op;
        Nst_NodeData_ELocStackOp e_loc_stack_op;
        Nst_NodeData_ELocalOp e_local_op;
        Nst_NodeData_ESeqLiteral e_seq_literal;
        Nst_NodeData_EMapLiteral e_map_literal;
        Nst_NodeData_EValue e_value;
        Nst_NodeData_EAccess e_access;
        Nst_NodeData_EExtraction e_extraction;
        Nst_NodeData_EAssignment e_assignment;
        Nst_NodeData_ECompAssign e_comp_assignment;
        Nst_NodeData_EIf e_if;
        Nst_NodeData_EWrapper e_wrapper;
    } v;
} Nst_Node
```

**Description:**

The structure representing a parser node.

**Fields:**

- `start`: the starting position of the node
- `end`: the ending position of the node
- `type`: the [`Nst_NodeType`](c_api-nodes.md#nst_nodetype) of the node
- `v`: a union that contains the node's data

---

## Functions

### `_Nst_node_s_list_init`

**Synopsis:**

```better-c
bool _Nst_node_s_list_init(Nst_Node *node)
```

**Description:**

Initialize a CS node.

---

### `_Nst_node_s_list_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_list_destroy(Nst_Node *node)
```

**Description:**

Destroy a CS node.

---

### `_Nst_node_s_while_lp_init`

**Synopsis:**

```better-c
bool _Nst_node_s_while_lp_init(Nst_Node *node)
```

**Description:**

Initialize a WL node.

---

### `_Nst_node_s_while_lp_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_while_lp_destroy(Nst_Node *node)
```

**Description:**

Destroy a WL node.

---

### `_Nst_node_s_for_lp_init`

**Synopsis:**

```better-c
bool _Nst_node_s_for_lp_init(Nst_Node *node)
```

**Description:**

Initialize a FL node.

---

### `_Nst_node_s_for_lp_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_for_lp_destroy(Nst_Node *node)
```

**Description:**

Destroy a FL node.

---

### `_Nst_node_s_fn_decl_init`

**Synopsis:**

```better-c
bool _Nst_node_s_fn_decl_init(Nst_Node *node)
```

**Description:**

Initialize a FD node.

---

### `_Nst_node_s_fn_decl_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_fn_decl_destroy(Nst_Node *node)
```

**Description:**

Destroy a FD node.

---

### `_Nst_node_s_return_init`

**Synopsis:**

```better-c
bool _Nst_node_s_return_init(Nst_Node *node)
```

**Description:**

Initialize a RT node.

---

### `_Nst_node_s_return_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_return_destroy(Nst_Node *node)
```

**Description:**

Destroy a RT node.

---

### `_Nst_node_s_switch_init`

**Synopsis:**

```better-c
bool _Nst_node_s_switch_init(Nst_Node *node)
```

**Description:**

Initialize a SW node.

---

### `_Nst_node_s_switch_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_switch_destroy(Nst_Node *node)
```

**Description:**

Destroy a SW node.

---

### `_Nst_node_s_try_catch_init`

**Synopsis:**

```better-c
bool _Nst_node_s_try_catch_init(Nst_Node *node)
```

**Description:**

Initialize a TC node.

---

### `_Nst_node_s_try_catch_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_try_catch_destroy(Nst_Node *node)
```

**Description:**

Destroy a TC node.

---

### `_Nst_node_s_wrapper_init`

**Synopsis:**

```better-c
bool _Nst_node_s_wrapper_init(Nst_Node *node)
```

**Description:**

Initialize a WS node.

---

### `_Nst_node_s_wrapper_destroy`

**Synopsis:**

```better-c
void _Nst_node_s_wrapper_destroy(Nst_Node *node)
```

**Description:**

Destroy a WS node.

---

### `_Nst_node_e_stack_op_init`

**Synopsis:**

```better-c
bool _Nst_node_e_stack_op_init(Nst_Node *node)
```

**Description:**

Initialize a SO node.

---

### `_Nst_node_e_stack_op_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_stack_op_destroy(Nst_Node *node)
```

**Description:**

Destroy a SO node.

---

### `_Nst_node_e_loc_stack_op_init`

**Synopsis:**

```better-c
bool _Nst_node_e_loc_stack_op_init(Nst_Node *node)
```

**Description:**

Initialize a LS node.

---

### `_Nst_node_e_loc_stack_op_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_loc_stack_op_destroy(Nst_Node *node)
```

**Description:**

Destroy a LS node.

---

### `_Nst_node_e_local_op_init`

**Synopsis:**

```better-c
bool _Nst_node_e_local_op_init(Nst_Node *node)
```

**Description:**

Initialize a LO node.

---

### `_Nst_node_e_local_op_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_local_op_destroy(Nst_Node *node)
```

**Description:**

Destroy a LO node.

---

### `_Nst_node_e_seq_literal_init`

**Synopsis:**

```better-c
bool _Nst_node_e_seq_literal_init(Nst_Node *node)
```

**Description:**

Initialize a SL node.

---

### `_Nst_node_e_seq_literal_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_seq_literal_destroy(Nst_Node *node)
```

**Description:**

Destroy a SL node.

---

### `_Nst_node_e_map_literal_init`

**Synopsis:**

```better-c
bool _Nst_node_e_map_literal_init(Nst_Node *node)
```

**Description:**

Initialize a ML node.

---

### `_Nst_node_e_map_literal_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_map_literal_destroy(Nst_Node *node)
```

**Description:**

Destroy a ML node.

---

### `_Nst_node_e_value_init`

**Synopsis:**

```better-c
bool _Nst_node_e_value_init(Nst_Node *node)
```

**Description:**

Initialize a VL node.

---

### `_Nst_node_e_value_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_value_destroy(Nst_Node *node)
```

**Description:**

Destroy a VL node.

---

### `_Nst_node_e_access_init`

**Synopsis:**

```better-c
bool _Nst_node_e_access_init(Nst_Node *node)
```

**Description:**

Initialize a AC node.

---

### `_Nst_node_e_access_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_access_destroy(Nst_Node *node)
```

**Description:**

Destroy a AC node.

---

### `_Nst_node_e_extraction_init`

**Synopsis:**

```better-c
bool _Nst_node_e_extraction_init(Nst_Node *node)
```

**Description:**

Initialize a EX node.

---

### `_Nst_node_e_extraction_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_extraction_destroy(Nst_Node *node)
```

**Description:**

Destroy a EX node.

---

### `_Nst_node_e_assignment_init`

**Synopsis:**

```better-c
bool _Nst_node_e_assignment_init(Nst_Node *node)
```

**Description:**

Initialize a AS node.

---

### `_Nst_node_e_assignment_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_assignment_destroy(Nst_Node *node)
```

**Description:**

Destroy a AS node.

---

### `_Nst_node_e_comp_assign_init`

**Synopsis:**

```better-c
bool _Nst_node_e_comp_assign_init(Nst_Node *node)
```

**Description:**

Initialize a CA node.

---

### `_Nst_node_e_comp_assign_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_comp_assign_destroy(Nst_Node *node)
```

**Description:**

Destroy a CA node.

---

### `_Nst_node_e_if_init`

**Synopsis:**

```better-c
bool _Nst_node_e_if_init(Nst_Node *node)
```

**Description:**

Initialize a IE node.

---

### `_Nst_node_e_if_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_if_destroy(Nst_Node *node)
```

**Description:**

Destroy a IE node.

---

### `_Nst_node_e_wrapper_init`

**Synopsis:**

```better-c
bool _Nst_node_e_wrapper_init(Nst_Node *node)
```

**Description:**

Initialize a WE node.

---

### `_Nst_node_e_wrapper_destroy`

**Synopsis:**

```better-c
void _Nst_node_e_wrapper_destroy(Nst_Node *node)
```

**Description:**

Destroy a WE node.

---

### `Nst_node_destroy`

**Synopsis:**

```better-c
void Nst_node_destroy(Nst_Node *node)
```

**Description:**

Destroy the contents of `node` and frees it.

---

### `Nst_node_destroy_contents`

**Synopsis:**

```better-c
void Nst_node_destroy_contents(Nst_Node *node)
```

**Description:**

Destroy only the contents of `node` without freeing it.

---

### `Nst_print_node`

**Synopsis:**

```better-c
void Nst_print_node(Nst_Node *node)
```

**Description:**

Print an [`Nst_Node`](c_api-nodes.md#nst_node) to the standard output.

---

### `Nst_node_type_to_str`

**Synopsis:**

```better-c
const char *Nst_node_type_to_str(Nst_NodeType nt)
```

**Description:**

Convert an [`Nst_NodeType`](c_api-nodes.md#nst_nodetype) to a string.

---

### `Nst_node_change_type`

**Synopsis:**

```better-c
bool Nst_node_change_type(Nst_Node *node, Nst_NodeType new_type)
```

**Description:**

Change the type of a node destroying the previous contents but keeping the
position.

!!!note
    Changing the node to a [`Nst_NT_S_NOP`](c_api-nodes.md#nst_nodetype) is
    guaranteed to succeed.

**Parameters:**

- `node`: the node to change the type of
- `new_type`: the new type for the node

**Returns:**

`true` on success and `false` on failure, the error is set.

---

## Enums

### `Nst_NodeType`

**Synopsis:**

```better-c
typedef enum _Nst_NodeType {
    // Statement nodes

    Nst_NT_S_LIST,
    Nst_NT_S_WHILE_LP,
    Nst_NT_S_FOR_LP,
    Nst_NT_S_FN_DECL,
    Nst_NT_S_RETURN,
    Nst_NT_S_CONTINUE,
    Nst_NT_S_BREAK,
    Nst_NT_S_SWITCH,
    Nst_NT_S_TRY_CATCH,
    Nst_NT_S_WRAPPER,
    Nst_NT_S_NOP,

    // Expression nodes

    Nst_NT_E_STACK_OP,
    Nst_NT_E_LOC_STACK_OP,
    Nst_NT_E_LOCAL_OP,
    Nst_NT_E_SEQ_LITERAL,
    Nst_NT_E_MAP_LITERAL,
    Nst_NT_E_VALUE,
    Nst_NT_E_ACCESS,
    Nst_NT_E_EXTRACTION,
    Nst_NT_E_ASSIGNMENT,
    Nst_NT_E_COMP_ASSIGN,
    Nst_NT_E_IF,
    Nst_NT_E_WRAPPER
} Nst_NodeType
```

**Description:**

The types of nodes in the AST.

!!!note
    Wrapper types are used to handle the positions of expressions inside
    parenthesis properly

---

### `Nst_SeqNodeType`

**Synopsis:**

```better-c
typedef enum _Nst_SeqNodeType {
    Nst_SNT_ARRAY,
    Nst_SNT_ARRAY_REP,
    Nst_SNT_VECTOR,
    Nst_SNT_VECTOR_REP,
    Nst_SNT_ASSIGNMENT_NAMES,
    Nst_SNT_NOT_SET
} Nst_SeqNodeType
```

**Description:**

The types of a [`Nst_Node`](c_api-nodes.md#nst_node) of type
[`Nst_NT_E_SEQ_LITERAL`](c_api-nodes.md#nst_nodetype)
