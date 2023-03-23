#include <errno.h>
#include "nodes.h"
#include "tokens.h"
#include "mem.h"

Nst_Node *nst_node_new(Nst_NodeType type, Nst_OpErr *err)
{
    Nst_Node *node = (Nst_Node *)nst_malloc(1, sizeof(Nst_Node), err);
    Nst_LList *lists = (Nst_LList *)nst_malloc(2, sizeof(Nst_LList), err);
    if ( node == NULL || lists == NULL )
    {
        if ( node ) nst_free(node);
        if ( lists ) nst_free(lists);
        return NULL;
    }
    lists[0].head = NULL;
    lists[0].tail = NULL;
    lists[0].size = 0;
    lists[1].head = NULL;
    lists[1].tail = NULL;
    lists[1].size = 0;

    node->start = nst_no_pos();
    node->end = nst_no_pos();
    node->type = type;
    node->tokens = lists;
    node->nodes = lists + 1;
    return node;
}

Nst_Node *nst_node_new_pos(Nst_NodeType type,
                           Nst_Pos start,
                           Nst_Pos end,
                           Nst_OpErr *err)
{
    Nst_Node *node = nst_node_new(type, err);
    if ( node == NULL )
    {
        return NULL;
    }
    nst_node_set_pos(node, start, end);
    return node;
}

void nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
{
    node->start = start;
    node->end = end;
}

void nst_node_destroy(Nst_Node *node)
{
    if ( node == NULL )
    {
        return;
    }

    nst_llist_empty(node->tokens, (Nst_LListDestructor)nst_token_destroy);
    nst_llist_empty(node->nodes, (Nst_LListDestructor)nst_node_destroy);
    nst_free(node->tokens);
    nst_free(node);
}
