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

/* Evaluates to `true` if the specified node type returns a value. */
#define Nst_NODE_RETUNS_VALUE(node_type) (node_type >= Nst_NT_E_STACK_OP)

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

/* [docs:link Nst_NT_S_LIST Nst_NodeType] */
/* [docs:link Nst_NT_S_WHILE_LP Nst_NodeType] */
/* [docs:link Nst_NT_S_FOR_LP Nst_NodeType] */
/* [docs:link Nst_NT_S_FN_DECL Nst_NodeType] */
/* [docs:link Nst_NT_S_RETURN Nst_NodeType] */
/* [docs:link Nst_NT_S_CONTINUE Nst_NodeType] */
/* [docs:link Nst_NT_S_BREAK Nst_NodeType] */
/* [docs:link Nst_NT_S_SWITCH Nst_NodeType] */
/* [docs:link Nst_NT_S_TRY_CATCH Nst_NodeType] */
/* [docs:link Nst_NT_S_WRAPPER Nst_NodeType] */
/* [docs:link Nst_NT_S_NOP Nst_NodeType] */
/* [docs:link Nst_NT_E_STACK_OP Nst_NodeType] */
/* [docs:link Nst_NT_E_LOC_STACK_OP Nst_NodeType] */
/* [docs:link Nst_NT_E_LOCAL_OP Nst_NodeType] */
/* [docs:link Nst_NT_E_SEQ_LITERAL Nst_NodeType] */
/* [docs:link Nst_NT_E_MAP_LITERAL Nst_NodeType] */
/* [docs:link Nst_NT_E_VALUE Nst_NodeType] */
/* [docs:link Nst_NT_E_ACCESS Nst_NodeType] */
/* [docs:link Nst_NT_E_EXTRACTION Nst_NodeType] */
/* [docs:link Nst_NT_E_ASSIGNMENT Nst_NodeType] */
/* [docs:link Nst_NT_E_COMP_ASSIGN Nst_NodeType] */
/* [docs:link Nst_NT_E_IF Nst_NodeType] */
/* [docs:link Nst_NT_E_WRAPPER Nst_NodeType] */

/**
 * The types of nodes in the AST.
 *
 * @brief Note: wrapper types are used to handle the positions of expressions
 * inside parenthesis properly
 */
NstEXP typedef enum _Nst_NodeType {
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
} Nst_NodeType;

/* The types of a `Nst_Node` of type `Nst_NT_E_SEQ_LITERAL` */
NstEXP typedef enum _Nst_SeqNodeType {
    Nst_SNT_ARRAY,
    Nst_SNT_ARRAY_REP,
    Nst_SNT_VECTOR,
    Nst_SNT_VECTOR_REP,
    Nst_SNT_ASSIGNMENT_NAMES,
    Nst_SNT_NOT_SET
} Nst_SeqNodeType;

struct _Nst_Node;

/* The data for a `Nst_NT_S_LIST` node. */
NstEXP typedef struct _Nst_NodeData_SList {
    Nst_PtrArray statements;
} Nst_NodeData_SList;

/* The data for a `Nst_NT_S_WHILE_LP` node. */
NstEXP typedef struct _Nst_NodeData_SWhileLp {
    struct _Nst_Node *condition;
    struct _Nst_Node *body;
    bool is_dowhile;
} Nst_NodeData_SWhileLp;

/* The data for a `Nst_NT_S_FOR_LP` node. */
NstEXP typedef struct _Nst_NodeData_SForLp {
    struct _Nst_Node *iterator;
    struct _Nst_Node *assignment;
    struct _Nst_Node *body;
} Nst_NodeData_SForLp;

/* The data for a `Nst_NT_S_FN_DECL` node. */
NstEXP typedef struct _Nst_NodeData_SFnDecl {
    Nst_ObjRef *name;
    Nst_PtrArray argument_names;
    struct _Nst_Node *body;
} Nst_NodeData_SFnDecl;

/* The data for a `Nst_NT_S_RETURN` node. */
NstEXP typedef struct _Nst_NodeData_SReturn {
    struct _Nst_Node *value;
} Nst_NodeData_SReturn;

/* The data for a `Nst_NT_S_SWITCH` node. */
NstEXP typedef struct _Nst_NodeData_SSwitch {
    struct _Nst_Node *expr;
    Nst_PtrArray values;
    Nst_PtrArray bodies;
    struct _Nst_Node *default_body;
} Nst_NodeData_SSwitch;

/* The data for a `Nst_NT_S_TRY_CATCH` node. */
NstEXP typedef struct _Nst_NodeData_STryCatch {
    struct _Nst_Node *try_body;
    struct _Nst_Node *catch_body;
    Nst_ObjRef *error_name;
} Nst_NodeData_STryCatch;

/* The data for a `Nst_NT_S_WRAPPER` node. */
NstEXP typedef struct _Nst_NodeData_SWrapper {
    struct _Nst_Node *statement;
} Nst_NodeData_SWrapper;

/* The data for a `Nst_NT_E_STACK_OP` node. */
NstEXP typedef struct _Nst_NodeData_EStackOp {
    Nst_PtrArray values;
    Nst_TokType op;
} Nst_NodeData_EStackOp;

/* The data for a `Nst_NT_E_LOC_STACK_OP` node. */
NstEXP typedef struct _Nst_NodeData_ELocStackOp {
    Nst_PtrArray values;
    struct _Nst_Node *special_value;
    Nst_TokType op;
} Nst_NodeData_ELocStackOp;

/* The data for a `Nst_NT_E_LOCAL_OP` node. */
NstEXP typedef struct _Nst_NodeData_ELocalOp {
    struct _Nst_Node *value;
    Nst_TokType op;
} Nst_NodeData_ELocalOp;

/* The data for a `Nst_NT_E_SEQ_LITERAL` node. */
NstEXP typedef struct _Nst_NodeData_ESeqLiteral {
    Nst_PtrArray values;
    Nst_SeqNodeType type;
} Nst_NodeData_ESeqLiteral;

/* The data for a `Nst_NT_E_MAP_LITERAL` node. */
NstEXP typedef struct _Nst_NodeData_EMapLiteral {
    Nst_PtrArray keys;
    Nst_PtrArray values;
} Nst_NodeData_EMapLiteral;

/* The data for a `Nst_NT_E_VALUE` node. */
NstEXP typedef struct _Nst_NodeData_EValue {
    Nst_ObjRef *value;
} Nst_NodeData_EValue;

/* The data for a `Nst_NT_E_ACCESS` node. */
NstEXP typedef struct _Nst_NodeData_EAccess {
    Nst_Obj *value;
} Nst_NodeData_EAccess;

/* The data for a `Nst_NT_E_EXTRACTION` node. */
NstEXP typedef struct _Nst_NodeData_EExtraction {
    struct _Nst_Node *container;
    struct _Nst_Node *key;
} Nst_NodeData_EExtraction;

/* The data for a `Nst_NT_E_ASSIGNMENT` node. */
NstEXP typedef struct _Nst_NodeData_EAssignment {
    struct _Nst_Node *value;
    struct _Nst_Node *name;
} Nst_NodeData_EAssignment;

/* The data for a `Nst_NT_E_COMP_ASSIGN` node. */
NstEXP typedef struct _Nst_NodeData_ECompAssign {
    Nst_PtrArray values;
    struct _Nst_Node *name;
    Nst_TokType op;
} Nst_NodeData_ECompAssign;

/* The data for a `Nst_NT_E_IF` node. */
NstEXP typedef struct _Nst_NodeData_EIf {
    struct _Nst_Node *condition;
    struct _Nst_Node *body_if_true;
    struct _Nst_Node *body_if_false;
} Nst_NodeData_EIf;

/* The data for a `Nst_NT_E_WRAPPER` node. */
NstEXP typedef struct _Nst_NodeData_EWrapper {
    struct _Nst_Node *expr;
} Nst_NodeData_EWrapper;

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
} Nst_Node;

/* Initialize a CS node. */
NstEXP bool NstC _Nst_node_s_list_init(Nst_Node *node);
/* Destroy a CS node. */
NstEXP void NstC _Nst_node_s_list_destroy(Nst_Node *node);

/* Initialize a WL node. */
NstEXP bool NstC _Nst_node_s_while_lp_init(Nst_Node *node);
/* Destroy a WL node. */
NstEXP void NstC _Nst_node_s_while_lp_destroy(Nst_Node *node);

/* Initialize a FL node. */
NstEXP bool NstC _Nst_node_s_for_lp_init(Nst_Node *node);
/* Destroy a FL node. */
NstEXP void NstC _Nst_node_s_for_lp_destroy(Nst_Node *node);

/* Initialize a FD node. */
NstEXP bool NstC _Nst_node_s_fn_decl_init(Nst_Node *node);
/* Destroy a FD node. */
NstEXP void NstC _Nst_node_s_fn_decl_destroy(Nst_Node *node);

/* Initialize a RT node. */
NstEXP bool NstC _Nst_node_s_return_init(Nst_Node *node);
/* Destroy a RT node. */
NstEXP void NstC _Nst_node_s_return_destroy(Nst_Node *node);

/* Initialize a SW node. */
NstEXP bool NstC _Nst_node_s_switch_init(Nst_Node *node);
/* Destroy a SW node. */
NstEXP void NstC _Nst_node_s_switch_destroy(Nst_Node *node);

/* Initialize a TC node. */
NstEXP bool NstC _Nst_node_s_try_catch_init(Nst_Node *node);
/* Destroy a TC node. */
NstEXP void NstC _Nst_node_s_try_catch_destroy(Nst_Node *node);

/* Initialize a WS node. */
NstEXP bool NstC _Nst_node_s_wrapper_init(Nst_Node *node);
/* Destroy a WS node. */
NstEXP void NstC _Nst_node_s_wrapper_destroy(Nst_Node *node);

/* Initialize a SO node. */
NstEXP bool NstC _Nst_node_e_stack_op_init(Nst_Node *node);
/* Destroy a SO node. */
NstEXP void NstC _Nst_node_e_stack_op_destroy(Nst_Node *node);

/* Initialize a LS node. */
NstEXP bool NstC _Nst_node_e_loc_stack_op_init(Nst_Node *node);
/* Destroy a LS node. */
NstEXP void NstC _Nst_node_e_loc_stack_op_destroy(Nst_Node *node);

/* Initialize a LO node. */
NstEXP bool NstC _Nst_node_e_local_op_init(Nst_Node *node);
/* Destroy a LO node. */
NstEXP void NstC _Nst_node_e_local_op_destroy(Nst_Node *node);

/* Initialize a SL node. */
NstEXP bool NstC _Nst_node_e_seq_literal_init(Nst_Node *node);
/* Destroy a SL node. */
NstEXP void NstC _Nst_node_e_seq_literal_destroy(Nst_Node *node);

/* Initialize a ML node. */
NstEXP bool NstC _Nst_node_e_map_literal_init(Nst_Node *node);
/* Destroy a ML node. */
NstEXP void NstC _Nst_node_e_map_literal_destroy(Nst_Node *node);

/* Initialize a VL node. */
NstEXP bool NstC _Nst_node_e_value_init(Nst_Node *node);
/* Destroy a VL node. */
NstEXP void NstC _Nst_node_e_value_destroy(Nst_Node *node);

/* Initialize a AC node. */
NstEXP bool NstC _Nst_node_e_access_init(Nst_Node *node);
/* Destroy a AC node. */
NstEXP void NstC _Nst_node_e_access_destroy(Nst_Node *node);

/* Initialize a EX node. */
NstEXP bool NstC _Nst_node_e_extraction_init(Nst_Node *node);
/* Destroy a EX node. */
NstEXP void NstC _Nst_node_e_extraction_destroy(Nst_Node *node);

/* Initialize a AS node. */
NstEXP bool NstC _Nst_node_e_assignment_init(Nst_Node *node);
/* Destroy a AS node. */
NstEXP void NstC _Nst_node_e_assignment_destroy(Nst_Node *node);

/* Initialize a CA node. */
NstEXP bool NstC _Nst_node_e_comp_assign_init(Nst_Node *node);
/* Destroy a CA node. */
NstEXP void NstC _Nst_node_e_comp_assign_destroy(Nst_Node *node);

/* Initialize a IE node. */
NstEXP bool NstC _Nst_node_e_if_init(Nst_Node *node);
/* Destroy a IE node. */
NstEXP void NstC _Nst_node_e_if_destroy(Nst_Node *node);

/* Initialize a WE node. */
NstEXP bool NstC _Nst_node_e_wrapper_init(Nst_Node *node);
/* Destroy a WE node. */
NstEXP void NstC _Nst_node_e_wrapper_destroy(Nst_Node *node);

NstEXP Nst_Node *NstC Nst_node_new(Nst_NodeType type);
NstEXP void NstC Nst_node_set_span(Nst_Node *node, Nst_Span span);

/* Destroy the contents of `node` and frees it. */
NstEXP void NstC Nst_node_destroy(Nst_Node *node);
/* Destroy only the contents of `node` without freeing it. */
NstEXP void NstC Nst_node_destroy_contents(Nst_Node *node);

/* Print an `Nst_Node` to the standard output. */
NstEXP void NstC Nst_print_node(Nst_Node *node);

/* Convert an `Nst_NodeType` to a string. */
NstEXP const char *NstC Nst_node_type_to_str(Nst_NodeType nt);

/**
 * Change the type of a node destroying the previous contents but keeping the
 * position.
 *
 * @brief Note: changing the node to a `Nst_NT_S_NOP` is guaranteed to succeed.
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
