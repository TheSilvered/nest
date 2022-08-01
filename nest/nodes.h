#ifndef Nst_NODES_H
#define Nst_NODES_H

#include "error.h"
#include "llist.h"

#define NODE(expr) ((Nst_Node *)(expr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Pos start;
    Nst_Pos end;
    int type;
    LList *nodes;
    LList *tokens;
}
Nst_Node;

enum Nst_NodeTypes
{
    NST_NT_LONG_S,
    NST_NT_WHILE_L,
    NST_NT_DOWHILE_L,
    NST_NT_FOR_L,
    NST_NT_FOR_AS_L,
    NST_NT_IF_E,
    NST_NT_FUNC_DECLR,
    NST_NT_RETURN_S,
    NST_NT_STACK_OP,
    NST_NT_LOCAL_STACK_OP,
    NST_NT_LOCAL_OP,
    NST_NT_ARR_LIT,
    NST_NT_VECT_LIT,
    NST_NT_MAP_LIT,
    NST_NT_VALUE,
    NST_NT_ACCESS,
    NST_NT_EXTRACT_E,
    NST_NT_ASSIGN_E,
    NST_NT_CONTINUE_S,
    NST_NT_BREAK_S,
    NST_NT_SWITCH_S
};

Nst_Node *new_node_tokens(Nst_Pos start, Nst_Pos end, int type, LList *tokens);
Nst_Node *new_node_nodes(Nst_Pos start, Nst_Pos end, int type, LList *nodes);
Nst_Node *new_node_full(Nst_Pos start, Nst_Pos end, int type, LList *nodes, LList *tokens);
Nst_Node *new_node_empty(Nst_Pos start, Nst_Pos end, int type);
void destroy_node(Nst_Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_NODES_H
