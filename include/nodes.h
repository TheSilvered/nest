/* Nodes of the AST */

#ifndef NODES_H
#define NODES_H

#include "error.h"

#define NST_NODE(expr) ((Nst_Node *)(expr))
#define NST_NODE_RETUNS_VALUE(node_type) (node_type >= NST_NT_STACK_OP)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef enum _Nst_NodeType
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

    // These nodes return a value
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
Nst_NodeType;

NstEXP typedef struct _Nst_Node
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_NodeType type;
    Nst_LList *nodes;
    Nst_LList *tokens;
}
Nst_Node;

// New node on the heap
NstEXP Nst_Node *nst_node_new(Nst_NodeType type);
// New node on the heap with a position
NstEXP Nst_Node *nst_node_new_pos(Nst_NodeType type,
                                  Nst_Pos      start,
                                  Nst_Pos      end);
// Sets the position of a node
NstEXP void nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end);
// Destroy a token and its children
NstEXP void nst_node_destroy(Nst_Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_H
