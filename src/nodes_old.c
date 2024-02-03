#if 0
#include <errno.h>
#include "nodes_old.h"
#include "tokens.h"
#include "mem.h"

Nst_Node__old *Nst_node_new__old(i32 type)
{
    Nst_Node__old *node = Nst_malloc_c(1, Nst_Node__old);
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

Nst_Node__old *Nst_node_new_pos__old(i32 type, Nst_Pos start, Nst_Pos end)
{
    Nst_Node__old *node = Nst_node_new__old(type);
    if (node == NULL)
        return NULL;
    Nst_node_set_pos__old(node, start, end);
    return node;
}

void Nst_node_set_pos__old(Nst_Node__old *node, Nst_Pos start, Nst_Pos end)
{
    node->start = start;
    node->end = end;
}

void Nst_node_destroy__old(Nst_Node__old *node)
{
    if (node == NULL)
        return;

    Nst_llist_empty(node->tokens, (Nst_LListDestructor)Nst_tok_destroy);
    Nst_llist_empty(node->nodes, (Nst_LListDestructor)Nst_node_destroy__old);
    Nst_free(node->tokens);
    Nst_free(node);
}
#endif