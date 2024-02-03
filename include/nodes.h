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
#define Nst_NODE_RETUNS_VALUE(node_type) (node_type >= Nst_NT_STACK_OP)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The types of nodes in the AST.
 *
 * @brief Note: wrapper types are used to handle the positions of expressions
 * inside parenthesis properly
 */
NstEXP typedef enum _Nst_NodeType {
    Nst_NT_LONG_S,      // Long statement
    Nst_NT_WHILE_L,     // While or do-while loop
    Nst_NT_FOR_L,       // For or for-as loop
    Nst_NT_FUNC_DECLR,  // Function declaration or lambda
    Nst_NT_RETURN_S,    // Return statement
    Nst_NT_CONTINUE_S,  // Continue or breakthrough statement
    Nst_NT_BREAK_S,     // Break statement
    Nst_NT_SWITCH_S,    // Switch statement
    Nst_NT_TRY_CATCH_S, // Try-catch statement
    Nst_NT_S_WRAPPER,   // Statement wrapper

    // These nodes represent expressions

    Nst_NT_STACK_OP,       // Stack operation
    Nst_NT_LOCAL_STACK_OP, // Local-stack operation
    Nst_NT_LOCAL_OP,       // Local operation
    Nst_NT_SEQ_LIT,        // Array or Vector literal
    Nst_NT_MAP_LIT,        // Map literal
    Nst_NT_VALUE,          // Value literal
    Nst_NT_ACCESS,         // Variable access
    Nst_NT_EXTRACT_E,      // Extraction expression
    Nst_NT_ASSIGN_E,       // Assignment expression
    Nst_NT_COMP_ASSIGN_E,  // Compound assignment expression
    Nst_NT_IF_E,           // If expression
    Nst_NT_E_WRAPPER       // Expression wrapper
} Nst_NodeType;

NstEXP typedef enum _Nst_SeqNodeType {
    Nst_SNT_ARRAY,
    Nst_SNT_ARRAY_REP,
    Nst_SNT_VECTOR,
    Nst_SNT_VECTOR_REP,
    Nst_SNT_ASSIGNMENT_NAMES,
    Nst_SNT_NOT_SET
} Nst_SeqNodeType;

struct _Nst_Node;

NstEXP typedef struct _Nst_NodeData_LongS {
    Nst_LList *statements;
} Nst_NodeData_LongS;

NstEXP typedef struct _Nst_NodeData_WhileL {
    struct _Nst_Node *condition;
    struct _Nst_Node *body;
    bool is_dowhile;
} Nst_NodeData_WhileL;

NstEXP typedef struct _Nst_NodeData_ForL {
    struct _Nst_Node *iterator;
    struct _Nst_Node *assignment;
    struct _Nst_Node *body;
} Nst_NodeData_ForL;

NstEXP typedef struct _Nst_NodeData_FuncDeclr {
    Nst_Tok *name;
    Nst_LList *argument_names;
    struct _Nst_Node *body;
} Nst_NodeData_FuncDeclr;

NstEXP typedef struct _Nst_NodeData_ReturnS {
    struct _Nst_Node *value;
} Nst_NodeData_ReturnS;

NstEXP typedef struct _Nst_NodeData_SwitchS {
    struct _Nst_Node *expr;
    Nst_LList *values;
    Nst_LList *bodies;
    struct _Nst_Node *default_body;
} Nst_NodeData_SwitchS;

NstEXP typedef struct _Nst_NodeData_TryCatchS {
    struct _Nst_Node *try_body;
    struct _Nst_Node *catch_body;
    Nst_Tok *error_name;
} Nst_NodeData_TryCatchS;

NstEXP typedef struct _Nst_NodeData_SWrapper {
    struct _Nst_Node *statement;
} Nst_NodeData_SWrapper;

NstEXP typedef struct _Nst_NodeData_StackOp {
    Nst_LList *values;
    Nst_TokType op;
} Nst_NodeData_StackOp;

NstEXP typedef struct _Nst_NodeData_LocalStackOp {
    Nst_LList *values;
    struct _Nst_Node *special_value;
    Nst_TokType op;
} Nst_NodeData_LocalStackOp;

NstEXP typedef struct _Nst_NodeData_LocalOp {
    struct _Nst_Node *value;
    Nst_TokType op;
} Nst_NodeData_LocalOp;

NstEXP typedef struct _Nst_NodeData_SeqLit {
    Nst_LList *values;
    Nst_SeqNodeType type;
} Nst_NodeData_SeqLit;

NstEXP typedef struct _Nst_NodeData_MapLit {
    Nst_LList *keys;
    Nst_LList *values;
} Nst_NodeData_MapLit;

NstEXP typedef struct _Nst_NodeData_Value {
    Nst_Tok *value;
} Nst_NodeData_Value;

NstEXP typedef struct _Nst_NodeData_Access {
    Nst_Tok *value;
} Nst_NodeData_Access;

NstEXP typedef struct _Nst_NodeData_ExtractE {
    struct _Nst_Node *container;
    struct _Nst_Node *key;
} Nst_NodeData_ExtractE;

NstEXP typedef struct _Nst_NodeData_AssignE {
    struct _Nst_Node *value;
    struct _Nst_Node *name;
} Nst_NodeData_AssignE;

NstEXP typedef struct _Nst_NodeData_CompAssignE {
    Nst_LList *values;
    struct _Nst_Node *name;
    Nst_TokType op;
} Nst_NodeData_CompAssignE;

NstEXP typedef struct _Nst_NodeData_IfE {
    struct _Nst_Node *condition;
    struct _Nst_Node *body_if_true;
    struct _Nst_Node *body_if_false;
} Nst_NodeData_IfE;

NstEXP typedef struct _Nst_NodeData_EWrapper {
    struct _Nst_Node *expr;
} Nst_NodeData_EWrapper;

NstEXP typedef struct _Nst_Node {
    Nst_Pos start;
    Nst_Pos end;
    Nst_NodeType type;
    union {
        Nst_NodeData_LongS long_s;
        Nst_NodeData_WhileL while_l;
        Nst_NodeData_ForL for_l;
        Nst_NodeData_FuncDeclr func_declr;
        Nst_NodeData_ReturnS return_s;
        Nst_NodeData_SwitchS switch_s;
        Nst_NodeData_TryCatchS try_catch_s;
        Nst_NodeData_SWrapper s_wrapper;
        Nst_NodeData_StackOp stack_op;
        Nst_NodeData_LocalStackOp local_stack_op;
        Nst_NodeData_LocalOp local_op;
        Nst_NodeData_SeqLit seq_lit;
        Nst_NodeData_MapLit map_lit;
        Nst_NodeData_Value value;
        Nst_NodeData_Access access;
        Nst_NodeData_ExtractE extract_e;
        Nst_NodeData_AssignE assign_e;
        Nst_NodeData_CompAssignE comp_assign_e;
        Nst_NodeData_IfE if_e;
        Nst_NodeData_EWrapper e_wrapper;
    };
} Nst_Node;

NstEXP bool NstC _Nst_node_long_s_init(Nst_Node *node);
NstEXP void NstC _Nst_node_long_s_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_while_l_init(Nst_Node *node);
NstEXP void NstC _Nst_node_while_l_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_for_l_init(Nst_Node *node);
NstEXP void NstC _Nst_node_for_l_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_func_declr_init(Nst_Node *node);
NstEXP void NstC _Nst_node_func_declr_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_return_s_init(Nst_Node *node);
NstEXP void NstC _Nst_node_return_s_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_switch_s_init(Nst_Node *node);
NstEXP void NstC _Nst_node_switch_s_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_try_catch_s_init(Nst_Node *node);
NstEXP void NstC _Nst_node_try_catch_s_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_s_wrapper_init(Nst_Node *node);
NstEXP void NstC _Nst_node_s_wrapper_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_stack_op_init(Nst_Node *node);
NstEXP void NstC _Nst_node_stack_op_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_local_stack_op_init(Nst_Node *node);
NstEXP void NstC _Nst_node_local_stack_op_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_local_op_init(Nst_Node *node);
NstEXP void NstC _Nst_node_local_op_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_seq_lit_init(Nst_Node *node);
NstEXP void NstC _Nst_node_seq_lit_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_map_lit_init(Nst_Node *node);
NstEXP void NstC _Nst_node_map_lit_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_value_init(Nst_Node *node);
NstEXP void NstC _Nst_node_value_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_access_init(Nst_Node *node);
NstEXP void NstC _Nst_node_access_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_extract_e_init(Nst_Node *node);
NstEXP void NstC _Nst_node_extract_e_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_assign_e_init(Nst_Node *node);
NstEXP void NstC _Nst_node_assign_e_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_comp_assign_e_init(Nst_Node *node);
NstEXP void NstC _Nst_node_comp_assign_e_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_if_e_init(Nst_Node *node);
NstEXP void NstC _Nst_node_if_e_destroy(Nst_Node *node);

NstEXP bool NstC _Nst_node_e_wrapper_init(Nst_Node *node);
NstEXP void NstC _Nst_node_e_wrapper_destroy(Nst_Node *node);

NstEXP Nst_Node *NstC Nst_node_new(Nst_NodeType type);
NstEXP Nst_Node *NstC Nst_node_set_pos(Nst_Node *node, Nst_Pos start,
                                       Nst_Pos end);

NstEXP Nst_Node *NstC Nst_node_destroy(Nst_Node *node);

NstEXP void NstC Nst_print_node(Nst_Node *node);

NstEXP const i8 *NstC Nst_node_type_to_str(Nst_NodeType nt);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_H
