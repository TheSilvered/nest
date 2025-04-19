/**
 * @file nodes.h
 *
 * @brief Nodes of the AST
 *
 * @author TheSilvered
 */

#ifndef NODES_H
#define NODES_H

#include "error.h"
#include "tokens.h"

/* Casts `expr` to `Nst_Node *`. */
#define Nst_NODE(expr) ((Nst_Node *)(expr))
/* Evaluates to `true` if the specified node type returns a value. */
#define Nst_NODE_RETUNS_VALUE(node_type) (node_type >= Nst_NT_SO)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

// IMPORTANT
// When adding a new node type:
//  1) Add it to the Nst_NodeType enum as Nst_NT_[UPPERCASE_NAME]
//  2) Add a struct called Nst_NodeData_[PascalCaseName] that defines its data
//     if it contains any
//  3) Add the node data to the Nst_Node union with the identifier in
//     snake_case
//  4) Implement an init, destroy and print functions and add them to
//     initializers, destructors and prints respectively, or set them to NULL
//     if the node does not contain any data
//  5) Add the node name to nt_strings

/* [docs:link Nst_NT_CS Nst_NodeType] */
/* [docs:link Nst_NT_WL Nst_NodeType] */
/* [docs:link Nst_NT_FL Nst_NodeType] */
/* [docs:link Nst_NT_FD Nst_NodeType] */
/* [docs:link Nst_NT_RT Nst_NodeType] */
/* [docs:link Nst_NT_CN Nst_NodeType] */
/* [docs:link Nst_NT_BR Nst_NodeType] */
/* [docs:link Nst_NT_SW Nst_NodeType] */
/* [docs:link Nst_NT_TC Nst_NodeType] */
/* [docs:link Nst_NT_WS Nst_NodeType] */
/* [docs:link Nst_NT_NP Nst_NodeType] */
/* [docs:link Nst_NT_SO Nst_NodeType] */
/* [docs:link Nst_NT_LS Nst_NodeType] */
/* [docs:link Nst_NT_LO Nst_NodeType] */
/* [docs:link Nst_NT_SL Nst_NodeType] */
/* [docs:link Nst_NT_ML Nst_NodeType] */
/* [docs:link Nst_NT_VL Nst_NodeType] */
/* [docs:link Nst_NT_AC Nst_NodeType] */
/* [docs:link Nst_NT_EX Nst_NodeType] */
/* [docs:link Nst_NT_AS Nst_NodeType] */
/* [docs:link Nst_NT_CA Nst_NodeType] */
/* [docs:link Nst_NT_IE Nst_NodeType] */
/* [docs:link Nst_NT_WE Nst_NodeType] */

/**
 * The types of nodes in the AST.
 *
 * @brief Note: wrapper types are used to handle the positions of expressions
 * inside parenthesis properly
 */
NstEXP typedef enum _Nst_NodeType {
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
} Nst_NodeType;

/* The types of a `Nst_Node` of type `Nst_NT_SL` */
NstEXP typedef enum _Nst_SeqNodeType {
    Nst_SNT_ARRAY,
    Nst_SNT_ARRAY_REP,
    Nst_SNT_VECTOR,
    Nst_SNT_VECTOR_REP,
    Nst_SNT_ASSIGNMENT_NAMES,
    Nst_SNT_NOT_SET
} Nst_SeqNodeType;

struct _Nst_Node;

/* The data for a `Nst_NT_CS` node. */
NstEXP typedef struct _Nst_NodeData_Cs {
    Nst_DynArray statements;
} Nst_NodeData_Cs;

/* The data for a `Nst_NT_WL` node. */
NstEXP typedef struct _Nst_NodeData_Wl {
    struct _Nst_Node *condition;
    struct _Nst_Node *body;
    bool is_dowhile;
} Nst_NodeData_Wl;

/* The data for a `Nst_NT_FL` node. */
NstEXP typedef struct _Nst_NodeData_Fl {
    struct _Nst_Node *iterator;
    struct _Nst_Node *assignment;
    struct _Nst_Node *body;
} Nst_NodeData_Fl;

/* The data for a `Nst_NT_FD` node. */
NstEXP typedef struct _Nst_NodeData_Fd {
    Nst_ObjRef *name;
    Nst_DynArray argument_names;
    struct _Nst_Node *body;
} Nst_NodeData_Fd;

/* The data for a `Nst_NT_RT` node. */
NstEXP typedef struct _Nst_NodeData_Rt {
    struct _Nst_Node *value;
} Nst_NodeData_Rt;

/* The data for a `Nst_NT_SW` node. */
NstEXP typedef struct _Nst_NodeData_Sw {
    struct _Nst_Node *expr;
    Nst_DynArray values;
    Nst_DynArray bodies;
    struct _Nst_Node *default_body;
} Nst_NodeData_Sw;

/* The data for a `Nst_NT_TC` node. */
NstEXP typedef struct _Nst_NodeData_Tc {
    struct _Nst_Node *try_body;
    struct _Nst_Node *catch_body;
    Nst_ObjRef *error_name;
} Nst_NodeData_Tc;

/* The data for a `Nst_NT_WS` node. */
NstEXP typedef struct _Nst_NodeData_Ws {
    struct _Nst_Node *statement;
} Nst_NodeData_Ws;

/* The data for a `Nst_NT_SO` node. */
NstEXP typedef struct _Nst_NodeData_So {
    Nst_DynArray values;
    Nst_TokType op;
} Nst_NodeData_So;

/* The data for a `Nst_NT_LS` node. */
NstEXP typedef struct _Nst_NodeData_Ls {
    Nst_DynArray values;
    struct _Nst_Node *special_value;
    Nst_TokType op;
} Nst_NodeData_Ls;

/* The data for a `Nst_NT_LO` node. */
NstEXP typedef struct _Nst_NodeData_Lo {
    struct _Nst_Node *value;
    Nst_TokType op;
} Nst_NodeData_Lo;

/* The data for a `Nst_NT_SL` node. */
NstEXP typedef struct _Nst_NodeData_Sl {
    Nst_DynArray values;
    Nst_SeqNodeType type;
} Nst_NodeData_Sl;

/* The data for a `Nst_NT_ML` node. */
NstEXP typedef struct _Nst_NodeData_Ml {
    Nst_DynArray keys;
    Nst_DynArray values;
} Nst_NodeData_Ml;

/* The data for a `Nst_NT_VL` node. */
NstEXP typedef struct _Nst_NodeData_Vl {
    Nst_ObjRef *value;
} Nst_NodeData_Vl;
/* The data for a `Nst_NT_AC` node. */
NstEXP typedef struct _Nst_NodeData_Ac {
    Nst_Obj *value;
} Nst_NodeData_Ac;
/* The data for a `Nst_NT_EX` node. */
NstEXP typedef struct _Nst_NodeData_Ex {
    struct _Nst_Node *container;
    struct _Nst_Node *key;
} Nst_NodeData_Ex;
/* The data for a `Nst_NT_AS` node. */
NstEXP typedef struct _Nst_NodeData_As {
    struct _Nst_Node *value;
    struct _Nst_Node *name;
} Nst_NodeData_As;
/* The data for a `Nst_NT_CA` node. */
NstEXP typedef struct _Nst_NodeData_Ca {
    Nst_DynArray values;
    struct _Nst_Node *name;
    Nst_TokType op;
} Nst_NodeData_Ca;
/* The data for a `Nst_NT_IE` node. */
NstEXP typedef struct _Nst_NodeData_Ie {
    struct _Nst_Node *condition;
    struct _Nst_Node *body_if_true;
    struct _Nst_Node *body_if_false;
} Nst_NodeData_Ie;
/* The data for a `Nst_NT_WE` node. */
NstEXP typedef struct _Nst_NodeData_We {
    struct _Nst_Node *expr;
} Nst_NodeData_We;

/**
 * The structure representing a parser node.
 *
 * @param start: the starting position of the node
 * @param end: the ending position of the node
 * @param type: the `Nst_NodeType` of the node
 * @param v: a union that contains the node's data
 */
NstEXP typedef struct _Nst_Node {
    Nst_Span span;
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
} Nst_Node;

/* Initializes a CS node. */
NstEXP bool NstC _Nst_node_cs_init(Nst_Node *node);
/* Destroyes a CS node. */
NstEXP void NstC _Nst_node_cs_destroy(Nst_Node *node);

/* Initializes a WL node. */
NstEXP bool NstC _Nst_node_wl_init(Nst_Node *node);
/* Destroyes a WL node. */
NstEXP void NstC _Nst_node_wl_destroy(Nst_Node *node);

/* Initializes a FL node. */
NstEXP bool NstC _Nst_node_fl_init(Nst_Node *node);
/* Destroyes a FL node. */
NstEXP void NstC _Nst_node_fl_destroy(Nst_Node *node);

/* Initializes a FD node. */
NstEXP bool NstC _Nst_node_fd_init(Nst_Node *node);
/* Destroyes a FD node. */
NstEXP void NstC _Nst_node_fd_destroy(Nst_Node *node);

/* Initializes a RT node. */
NstEXP bool NstC _Nst_node_rt_init(Nst_Node *node);
/* Destroyes a RT node. */
NstEXP void NstC _Nst_node_rt_destroy(Nst_Node *node);

/* Initializes a SW node. */
NstEXP bool NstC _Nst_node_sw_init(Nst_Node *node);
/* Destroyes a SW node. */
NstEXP void NstC _Nst_node_sw_destroy(Nst_Node *node);

/* Initializes a TC node. */
NstEXP bool NstC _Nst_node_tc_init(Nst_Node *node);
/* Destroyes a TC node. */
NstEXP void NstC _Nst_node_tc_destroy(Nst_Node *node);

/* Initializes a WS node. */
NstEXP bool NstC _Nst_node_ws_init(Nst_Node *node);
/* Destroyes a WS node. */
NstEXP void NstC _Nst_node_ws_destroy(Nst_Node *node);

/* Initializes a SO node. */
NstEXP bool NstC _Nst_node_so_init(Nst_Node *node);
/* Destroyes a SO node. */
NstEXP void NstC _Nst_node_so_destroy(Nst_Node *node);

/* Initializes a LS node. */
NstEXP bool NstC _Nst_node_ls_init(Nst_Node *node);
/* Destroyes a LS node. */
NstEXP void NstC _Nst_node_ls_destroy(Nst_Node *node);

/* Initializes a LO node. */
NstEXP bool NstC _Nst_node_lo_init(Nst_Node *node);
/* Destroyes a LO node. */
NstEXP void NstC _Nst_node_lo_destroy(Nst_Node *node);

/* Initializes a SL node. */
NstEXP bool NstC _Nst_node_sl_init(Nst_Node *node);
/* Destroyes a SL node. */
NstEXP void NstC _Nst_node_sl_destroy(Nst_Node *node);

/* Initializes a ML node. */
NstEXP bool NstC _Nst_node_ml_init(Nst_Node *node);
/* Destroyes a ML node. */
NstEXP void NstC _Nst_node_ml_destroy(Nst_Node *node);

/* Initializes a VL node. */
NstEXP bool NstC _Nst_node_vl_init(Nst_Node *node);
/* Destroyes a VL node. */
NstEXP void NstC _Nst_node_vl_destroy(Nst_Node *node);

/* Initializes a AC node. */
NstEXP bool NstC _Nst_node_ac_init(Nst_Node *node);
/* Destroyes a AC node. */
NstEXP void NstC _Nst_node_ac_destroy(Nst_Node *node);

/* Initializes a EX node. */
NstEXP bool NstC _Nst_node_ex_init(Nst_Node *node);
/* Destroyes a EX node. */
NstEXP void NstC _Nst_node_ex_destroy(Nst_Node *node);

/* Initializes a AS node. */
NstEXP bool NstC _Nst_node_as_init(Nst_Node *node);
/* Destroyes a AS node. */
NstEXP void NstC _Nst_node_as_destroy(Nst_Node *node);

/* Initializes a CA node. */
NstEXP bool NstC _Nst_node_ca_init(Nst_Node *node);
/* Destroyes a CA node. */
NstEXP void NstC _Nst_node_ca_destroy(Nst_Node *node);

/* Initializes a IE node. */
NstEXP bool NstC _Nst_node_ie_init(Nst_Node *node);
/* Destroyes a IE node. */
NstEXP void NstC _Nst_node_ie_destroy(Nst_Node *node);

/* Initializes a WE node. */
NstEXP bool NstC _Nst_node_we_init(Nst_Node *node);
/* Destroyes a WE node. */
NstEXP void NstC _Nst_node_we_destroy(Nst_Node *node);

NstEXP Nst_Node *NstC Nst_node_new(Nst_NodeType type);
NstEXP void NstC Nst_node_set_span(Nst_Node *node, Nst_Span span);

/* Destroys the contents of `node` and frees it. */
NstEXP void NstC Nst_node_destroy(Nst_Node *node);
/* Destroys only the contents of `node` without freeing it. */
NstEXP void NstC Nst_node_destroy_contents(Nst_Node *node);

NstEXP void NstC Nst_print_node(Nst_Node *node);

NstEXP const char *NstC Nst_node_type_to_str(Nst_NodeType nt);

/**
 * Changes the type of a node destroying the previous contents but keeping the
 * position.
 *
 * @brief Note: changing the node to a `Nst_NT_NP` is guaranteed to succeed.
 *
 * @param node: the node to change the type of
 * @param new_type: the new type for the node
 *
 * @return `true` on success and `false` on failure, the error is set.
 */
NstEXP bool NstC Nst_node_change_type(Nst_Node *node, Nst_NodeType new_type);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_H
