/* Nodes of the AST */

#ifndef NODES_H
#define NODES_H

#include "error.h"

#define NST_NODE(expr) ((Nst_Node *)(expr))
#define NST_NODE_RETUNS_VALUE(node_type) (node_type >= NST_NT_STACK_OP)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

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

typedef struct _Nst_Node
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_NodeType type;
    Nst_LList *nodes;
    Nst_LList *tokens;
}
Nst_Node;

// New node on the heap with only a list of tokens
Nst_Node *nst_node_new_tokens(Nst_Pos      start,
                              Nst_Pos      end,
                              Nst_NodeType type,
                              Nst_LList   *tokens);
// New node on the heap with only a list of nodes
Nst_Node *nst_node_new_nodes(Nst_Pos      start,
                             Nst_Pos      end,
                             Nst_NodeType type,
                             Nst_LList   *nodes);
// New node on the heap with both a list of nodes an tokens
Nst_Node *nst_node_new_full(Nst_Pos      start,
                            Nst_Pos      end,
                            Nst_NodeType type,
                            Nst_LList   *nodes,
                            Nst_LList   *tokens);
// New node with no lists
Nst_Node *nst_node_new_empty(Nst_Pos start, Nst_Pos end, Nst_NodeType type);
// Destroy a token and its children
void nst_node_destroy(Nst_Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !NODES_H
