/* Nodes of the AST */

#ifndef NODES_H
#define NODES_H

#include "error.h"

#define Nst_NODE(expr) ((Nst_Node *)(expr))
#define Nst_NODE_RETUNS_VALUE(node_type) (node_type >= Nst_NT_STACK_OP)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef enum _Nst_NodeType
{
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
NstEXP Nst_Node *NstC Nst_node_new(Nst_NodeType type);
// New node on the heap with a position
NstEXP Nst_Node *NstC Nst_node_new_pos(Nst_NodeType type, Nst_Pos start,
                                       Nst_Pos end);
// Sets the position of a node
NstEXP void NstC Nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end);
// Destroy a token and its children
NstEXP void NstC Nst_node_destroy(Nst_Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_H
