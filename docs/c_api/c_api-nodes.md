# `nodes.h`

Nodes of the AST.

## Authors

TheSilvered

---

## Macros

### `Nst_NODE`

**Synopsis:**

```better-c
#define Nst_NODE(expr)
```

**Description:**

Casts `expr` to [`Nst_Node *`](c_api-nodes.md#nst_node).

---

### `Nst_NODE_RETUNS_VALUE`

**Synopsis:**

```better-c
#define Nst_NODE_RETUNS_VALUE(node_type)
```

**Description:**

Evaluates to `true` if the specified node type returns a value.

---

## Structs

### `Nst_NodeData_Cs`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Cs {
    Nst_LList *statements;
} Nst_NodeData_Cs
```

**Description:**

The data for a [`Nst_NT_CS`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Wl`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Wl {
    Nst_Node *condition;
    Nst_Node *body;
    bool is_dowhile;
} Nst_NodeData_Wl
```

**Description:**

The data for a [`Nst_NT_WL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Fl`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Fl {
    Nst_Node *iterator;
    Nst_Node *assignment;
    Nst_Node *body;
} Nst_NodeData_Fl
```

**Description:**

The data for a [`Nst_NT_FL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Fd`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Fd {
    Nst_Tok *name;
    Nst_LList *argument_names;
    Nst_Node *body;
} Nst_NodeData_Fd
```

**Description:**

The data for a [`Nst_NT_FD`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Rt`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Rt {
    Nst_Node *value;
} Nst_NodeData_Rt
```

**Description:**

The data for a [`Nst_NT_RT`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Sw`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Sw {
    Nst_Node *expr;
    Nst_LList *values;
    Nst_LList *bodies;
    Nst_Node *default_body;
} Nst_NodeData_Sw
```

**Description:**

The data for a [`Nst_NT_SW`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Tc`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Tc {
    Nst_Node *try_body;
    Nst_Node *catch_body;
    Nst_Tok *error_name;
} Nst_NodeData_Tc
```

**Description:**

The data for a [`Nst_NT_TC`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ws`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ws {
    Nst_Node *statement;
} Nst_NodeData_Ws
```

**Description:**

The data for a [`Nst_NT_WS`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_So`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_So {
    Nst_LList *values;
    Nst_TokType op;
} Nst_NodeData_So
```

**Description:**

The data for a [`Nst_NT_SO`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ls`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ls {
    Nst_LList *values;
    Nst_Node *special_value;
    Nst_TokType op;
} Nst_NodeData_Ls
```

**Description:**

The data for a [`Nst_NT_LS`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Lo`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Lo {
    Nst_Node *value;
    Nst_TokType op;
} Nst_NodeData_Lo
```

**Description:**

The data for a [`Nst_NT_LO`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Sl`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Sl {
    Nst_LList *values;
    Nst_SeqNodeType type;
} Nst_NodeData_Sl
```

**Description:**

The data for a [`Nst_NT_SL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ml`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ml {
    Nst_LList *keys;
    Nst_LList *values;
} Nst_NodeData_Ml
```

**Description:**

The data for a [`Nst_NT_ML`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Vl`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Vl {
    Nst_Tok *value;
} Nst_NodeData_Vl
```

**Description:**

The data for a [`Nst_NT_VL`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ac`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ac {
    Nst_Tok *value;
} Nst_NodeData_Ac
```

**Description:**

The data for a [`Nst_NT_AC`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ex`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ex {
    Nst_Node *container;
    Nst_Node *key;
} Nst_NodeData_Ex
```

**Description:**

The data for a [`Nst_NT_EX`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_As`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_As {
    Nst_Node *value;
    Nst_Node *name;
} Nst_NodeData_As
```

**Description:**

The data for a [`Nst_NT_AS`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ca`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ca {
    Nst_LList *values;
    Nst_Node *name;
    Nst_TokType op;
} Nst_NodeData_Ca
```

**Description:**

The data for a [`Nst_NT_CA`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_Ie`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_Ie {
    Nst_Node *condition;
    Nst_Node *body_if_true;
    Nst_Node *body_if_false;
} Nst_NodeData_Ie
```

**Description:**

The data for a [`Nst_NT_IE`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_NodeData_We`

**Synopsis:**

```better-c
typedef struct _Nst_NodeData_We {
    Nst_Node *expr;
} Nst_NodeData_We
```

**Description:**

The data for a [`Nst_NT_WE`](c_api-nodes.md#nst_nodetype) node.

---

### `Nst_Node`

**Synopsis:**

```better-c
typedef struct _Nst_Node {
    Nst_Pos start;
    Nst_Pos end;
    Nst_NodeType type;
    union {
        Nst_NodeData_Cs cs;
        Nst_NodeData_Wl wl;
        Nst_NodeData_Fl fl;
        Nst_NodeData_Fd fd;
        Nst_NodeData_Rt rt;
        Nst_NodeData_Sw sw;
        Nst_NodeData_Tc tc;
        Nst_NodeData_Ws ws;
        Nst_NodeData_So so;
        Nst_NodeData_Ls ls;
        Nst_NodeData_Lo lo;
        Nst_NodeData_Sl sl;
        Nst_NodeData_Ml ml;
        Nst_NodeData_Vl vl;
        Nst_NodeData_Ac ac;
        Nst_NodeData_Ex ex;
        Nst_NodeData_As as;
        Nst_NodeData_Ca ca;
        Nst_NodeData_Ie ie;
        Nst_NodeData_We we;
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

### `_Nst_node_cs_init`

**Synopsis:**

```better-c
bool _Nst_node_cs_init(Nst_Node *node)
```

**Description:**

Initializes a CS node.

---

### `_Nst_node_cs_destroy`

**Synopsis:**

```better-c
void _Nst_node_cs_destroy(Nst_Node *node)
```

**Description:**

Destroyes a CS node.

---

### `_Nst_node_wl_init`

**Synopsis:**

```better-c
bool _Nst_node_wl_init(Nst_Node *node)
```

**Description:**

Initializes a WL node.

---

### `_Nst_node_wl_destroy`

**Synopsis:**

```better-c
void _Nst_node_wl_destroy(Nst_Node *node)
```

**Description:**

Destroyes a WL node.

---

### `_Nst_node_fl_init`

**Synopsis:**

```better-c
bool _Nst_node_fl_init(Nst_Node *node)
```

**Description:**

Initializes a FL node.

---

### `_Nst_node_fl_destroy`

**Synopsis:**

```better-c
void _Nst_node_fl_destroy(Nst_Node *node)
```

**Description:**

Destroyes a FL node.

---

### `_Nst_node_fd_init`

**Synopsis:**

```better-c
bool _Nst_node_fd_init(Nst_Node *node)
```

**Description:**

Initializes a FD node.

---

### `_Nst_node_fd_destroy`

**Synopsis:**

```better-c
void _Nst_node_fd_destroy(Nst_Node *node)
```

**Description:**

Destroyes a FD node.

---

### `_Nst_node_rt_init`

**Synopsis:**

```better-c
bool _Nst_node_rt_init(Nst_Node *node)
```

**Description:**

Initializes a RT node.

---

### `_Nst_node_rt_destroy`

**Synopsis:**

```better-c
void _Nst_node_rt_destroy(Nst_Node *node)
```

**Description:**

Destroyes a RT node.

---

### `_Nst_node_sw_init`

**Synopsis:**

```better-c
bool _Nst_node_sw_init(Nst_Node *node)
```

**Description:**

Initializes a SW node.

---

### `_Nst_node_sw_destroy`

**Synopsis:**

```better-c
void _Nst_node_sw_destroy(Nst_Node *node)
```

**Description:**

Destroyes a SW node.

---

### `_Nst_node_tc_init`

**Synopsis:**

```better-c
bool _Nst_node_tc_init(Nst_Node *node)
```

**Description:**

Initializes a TC node.

---

### `_Nst_node_tc_destroy`

**Synopsis:**

```better-c
void _Nst_node_tc_destroy(Nst_Node *node)
```

**Description:**

Destroyes a TC node.

---

### `_Nst_node_ws_init`

**Synopsis:**

```better-c
bool _Nst_node_ws_init(Nst_Node *node)
```

**Description:**

Initializes a WS node.

---

### `_Nst_node_ws_destroy`

**Synopsis:**

```better-c
void _Nst_node_ws_destroy(Nst_Node *node)
```

**Description:**

Destroyes a WS node.

---

### `_Nst_node_so_init`

**Synopsis:**

```better-c
bool _Nst_node_so_init(Nst_Node *node)
```

**Description:**

Initializes a SO node.

---

### `_Nst_node_so_destroy`

**Synopsis:**

```better-c
void _Nst_node_so_destroy(Nst_Node *node)
```

**Description:**

Destroyes a SO node.

---

### `_Nst_node_ls_init`

**Synopsis:**

```better-c
bool _Nst_node_ls_init(Nst_Node *node)
```

**Description:**

Initializes a LS node.

---

### `_Nst_node_ls_destroy`

**Synopsis:**

```better-c
void _Nst_node_ls_destroy(Nst_Node *node)
```

**Description:**

Destroyes a LS node.

---

### `_Nst_node_lo_init`

**Synopsis:**

```better-c
bool _Nst_node_lo_init(Nst_Node *node)
```

**Description:**

Initializes a LO node.

---

### `_Nst_node_lo_destroy`

**Synopsis:**

```better-c
void _Nst_node_lo_destroy(Nst_Node *node)
```

**Description:**

Destroyes a LO node.

---

### `_Nst_node_sl_init`

**Synopsis:**

```better-c
bool _Nst_node_sl_init(Nst_Node *node)
```

**Description:**

Initializes a SL node.

---

### `_Nst_node_sl_destroy`

**Synopsis:**

```better-c
void _Nst_node_sl_destroy(Nst_Node *node)
```

**Description:**

Destroyes a SL node.

---

### `_Nst_node_ml_init`

**Synopsis:**

```better-c
bool _Nst_node_ml_init(Nst_Node *node)
```

**Description:**

Initializes a ML node.

---

### `_Nst_node_ml_destroy`

**Synopsis:**

```better-c
void _Nst_node_ml_destroy(Nst_Node *node)
```

**Description:**

Destroyes a ML node.

---

### `_Nst_node_vl_init`

**Synopsis:**

```better-c
bool _Nst_node_vl_init(Nst_Node *node)
```

**Description:**

Initializes a VL node.

---

### `_Nst_node_vl_destroy`

**Synopsis:**

```better-c
void _Nst_node_vl_destroy(Nst_Node *node)
```

**Description:**

Destroyes a VL node.

---

### `_Nst_node_ac_init`

**Synopsis:**

```better-c
bool _Nst_node_ac_init(Nst_Node *node)
```

**Description:**

Initializes a AC node.

---

### `_Nst_node_ac_destroy`

**Synopsis:**

```better-c
void _Nst_node_ac_destroy(Nst_Node *node)
```

**Description:**

Destroyes a AC node.

---

### `_Nst_node_ex_init`

**Synopsis:**

```better-c
bool _Nst_node_ex_init(Nst_Node *node)
```

**Description:**

Initializes a EX node.

---

### `_Nst_node_ex_destroy`

**Synopsis:**

```better-c
void _Nst_node_ex_destroy(Nst_Node *node)
```

**Description:**

Destroyes a EX node.

---

### `_Nst_node_as_init`

**Synopsis:**

```better-c
bool _Nst_node_as_init(Nst_Node *node)
```

**Description:**

Initializes a AS node.

---

### `_Nst_node_as_destroy`

**Synopsis:**

```better-c
void _Nst_node_as_destroy(Nst_Node *node)
```

**Description:**

Destroyes a AS node.

---

### `_Nst_node_ca_init`

**Synopsis:**

```better-c
bool _Nst_node_ca_init(Nst_Node *node)
```

**Description:**

Initializes a CA node.

---

### `_Nst_node_ca_destroy`

**Synopsis:**

```better-c
void _Nst_node_ca_destroy(Nst_Node *node)
```

**Description:**

Destroyes a CA node.

---

### `_Nst_node_ie_init`

**Synopsis:**

```better-c
bool _Nst_node_ie_init(Nst_Node *node)
```

**Description:**

Initializes a IE node.

---

### `_Nst_node_ie_destroy`

**Synopsis:**

```better-c
void _Nst_node_ie_destroy(Nst_Node *node)
```

**Description:**

Destroyes a IE node.

---

### `_Nst_node_we_init`

**Synopsis:**

```better-c
bool _Nst_node_we_init(Nst_Node *node)
```

**Description:**

Initializes a WE node.

---

### `_Nst_node_we_destroy`

**Synopsis:**

```better-c
void _Nst_node_we_destroy(Nst_Node *node)
```

**Description:**

Destroyes a WE node.

---

### `Nst_node_destroy`

**Synopsis:**

```better-c
void Nst_node_destroy(Nst_Node *node)
```

**Description:**

Destroys the contents of `node` and frees it.

---

### `Nst_node_destroy_contents`

**Synopsis:**

```better-c
void Nst_node_destroy_contents(Nst_Node *node)
```

**Description:**

Destroys only the contents of `node` without freeing it.

---

### `Nst_node_change_type`

**Synopsis:**

```better-c
bool Nst_node_change_type(Nst_Node *node, Nst_NodeType new_type)
```

**Description:**

Changes the type of a node destroying the previous contents but keeping the
position.

!!!note
    Changing the node to a [`Nst_NT_NP`](c_api-nodes.md#nst_nodetype) is
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

    Nst_NT_CS, // Compound statement
    Nst_NT_WL, // While or do-while loop
    Nst_NT_FL, // For or for-as loop
    Nst_NT_FD, // Function declaration or lambda
    Nst_NT_RT, // Return statement
    Nst_NT_CN, // Continue or breakthrough statement
    Nst_NT_BR, // Break statement
    Nst_NT_SW, // Switch statement
    Nst_NT_TC, // Try-catch statement
    Nst_NT_WS, // Statement wrapper
    Nst_NT_NP, // No-op statement

    // Expression nodes

    Nst_NT_SO, // Stack operation
    Nst_NT_LS, // Local-stack operation
    Nst_NT_LO, // Local operation
    Nst_NT_SL, // Array or Vector (sequence) literal
    Nst_NT_ML, // Map literal
    Nst_NT_VL, // Value literal
    Nst_NT_AC, // Variable access
    Nst_NT_EX, // Extraction expression
    Nst_NT_AS, // Assignment expression
    Nst_NT_CA, // Compound assignment expression
    Nst_NT_IE, // If expression
    Nst_NT_WE  // Expression wrapper
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
[`Nst_NT_SL`](c_api-nodes.md#nst_nodetype)
