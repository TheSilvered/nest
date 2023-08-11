#include <errno.h>
#include "nodes.h"
#include "tokens.h"
#include "mem.h"

Nst_Node *Nst_node_new(Nst_NodeType type)
{
    Nst_Node *node = Nst_malloc_c(1, Nst_Node);
    Nst_LList *lists = Nst_malloc_c(2, Nst_LList);
    if (node == NULL || lists == NULL) {
        if (node)
            Nst_free(node);
        if (lists)
            Nst_free(lists);
        return NULL;
    }
    Nst_llist_init(&lists[0]);
    Nst_llist_init(&lists[1]);

    node->start = Nst_no_pos();
    node->end = Nst_no_pos();
    node->type = type;
    node->tokens = lists;
    node->nodes = lists + 1;
    return node;
}

Nst_Node *Nst_node_new_pos(Nst_NodeType type, Nst_Pos start, Nst_Pos end)
{
    Nst_Node *node = Nst_node_new(type);
    if (node == NULL)
        return NULL;
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
    if (node == NULL)
        return;

    Nst_llist_empty(node->tokens, (Nst_LListDestructor)Nst_tok_destroy);
    Nst_llist_empty(node->nodes, (Nst_LListDestructor)Nst_node_destroy);
    Nst_free(node->tokens);
    Nst_free(node);
}
