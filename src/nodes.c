#include <errno.h>
#include "nodes.h"
#include "tokens.h"
#include "mem.h"

Nst_Node *Nst_node_new(Nst_NodeType type)
{
    Nst_Node *node = Nst_malloc_c(1, Nst_Node);
    Nst_LList *lists = Nst_malloc_c(2, Nst_LList);
    if ( node == NULL || lists == NULL )
    {
        if ( node ) Nst_free(node);
        if ( lists ) Nst_free(lists);
        return NULL;
    }
    lists[0].head = NULL;
    lists[0].tail = NULL;
    lists[0].size = 0;
    lists[1].head = NULL;
    lists[1].tail = NULL;
    lists[1].size = 0;

    node->start = Nst_no_pos();
    node->end = Nst_no_pos();
    node->type = type;
    node->tokens = lists;
    node->nodes = lists + 1;
    return node;
}

Nst_Node *Nst_node_new_pos(Nst_NodeType type,
                           Nst_Pos start,
                           Nst_Pos end)
{
    Nst_Node *node = Nst_node_new(type);
    if ( node == NULL )
    {
        return NULL;
    }
    Nst_node_set_pos(node, start, end);
    return node;
}

void Nst_node_set_pos(Nst_Node *node, Nst_Pos start, Nst_Pos end)
{
    node->start = start;
    node->end = end;
}

void Nst_node_destroy(Nst_Node *node)
{
    if ( node == NULL )
    {
        return;
    }

    Nst_llist_empty(node->tokens, (Nst_LListDestructor)Nst_token_destroy);
    Nst_llist_empty(node->nodes, (Nst_LListDestructor)Nst_node_destroy);
    Nst_free(node->tokens);
    Nst_free(node);
}
