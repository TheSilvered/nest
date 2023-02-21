#include <stdlib.h>
#include <errno.h>
#include "nodes.h"
#include "tokens.h"

Nst_Node *nst_node_new_tokens(Nst_Pos      start,
                              Nst_Pos      end,
                              Nst_NodeType type,
                              Nst_LList   *tokens)
{
    Nst_Node *node = (Nst_Node *)malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;
    node->tokens = tokens;

    errno = 0;
    Nst_LList *nodes = nst_llist_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->nodes = nodes;
    return node;
}

Nst_Node *nst_node_new_nodes(Nst_Pos      start,
                             Nst_Pos      end,
                             Nst_NodeType type,
                             Nst_LList   *nodes)
{
    Nst_Node *node = (Nst_Node *)malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;
    node->nodes = nodes;

    Nst_LList *tokens = nst_llist_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->tokens = tokens;
    return node;
}

Nst_Node *nst_node_new_full(Nst_Pos      start,
                            Nst_Pos      end,
                            Nst_NodeType type,
                            Nst_LList   *nodes,
                            Nst_LList   *tokens)
{
    Nst_Node *node = (Nst_Node *)malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;
    node->nodes = nodes;
    node->tokens = tokens;
    return node;
}

Nst_Node *nst_node_new_empty(Nst_Pos start, Nst_Pos end, Nst_NodeType type)
{
    Nst_Node *node = (Nst_Node *)malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;

    Nst_LList *nodes = nst_llist_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->nodes = nodes;

    Nst_LList *tokens = nst_llist_new();
    if ( errno == ENOMEM )
    {
        nst_node_destroy(node);
        return NULL;
    }

    node->tokens = tokens;
    return node;
}

void nst_node_destroy(Nst_Node *node)
{
    if ( node == NULL )
    {
        return;
    }

    nst_llist_destroy(node->tokens, (nst_llist_destructor)nst_token_destroy);
    nst_llist_destroy(node->nodes, (nst_llist_destructor)nst_node_destroy);

    free(node);
}
