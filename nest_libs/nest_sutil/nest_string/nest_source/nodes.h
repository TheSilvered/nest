#ifndef Nst_NODES_H
#define Nst_NODES_H

#include "error.h"
#include "llist.h"

#define NODE(expr) ((Node *)(expr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct Node
{
    Pos start;
    Pos end;
    int type;
    LList *nodes;
    LList *tokens;
}
Node;

enum node_types
{
    LONG_S,
    WHILE_L,
    DOWHILE_L,
    FOR_L,
    FOR_AS_L,
    IF_E,
    FUNC_DECLR,
    RETURN_S,
    STACK_OP,
    LOCAL_STACK_OP,
    LOCAL_OP,
    ARR_LIT,
    VECT_LIT,
    MAP_LIT,
    VALUE,
    ACCESS,
    EXTRACT_E,
    ASSIGN_E,
    CONTINUE_S,
    BREAK_S
};

Node *new_node_tokens(Pos start, Pos end, int type, LList *tokens);
Node *new_node_nodes(Pos start, Pos end, int type, LList *nodes);
Node *new_node_full(Pos start, Pos end, int type, LList *nodes, LList *tokens);
Node *new_node_empty(Pos start, Pos end, int type);
void destroy_node(Node *node);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_NODES_H
