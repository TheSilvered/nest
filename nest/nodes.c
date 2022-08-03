#include <stdlib.h>
#include <errno.h>
#include "nodes.h"
#include "tokens.h"

Nst_Node *nst_new_node_tokens(Nst_Pos start, Nst_Pos end, int type, LList *tokens)
{
    Nst_Node *node = malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;
    node->tokens = tokens;

    LList *nodes = LList_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->nodes = nodes;
    return node;
}

Nst_Node *nst_new_node_nodes(Nst_Pos start, Nst_Pos end, int type, LList *nodes)
{
    Nst_Node *node = malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;
    node->nodes = nodes;

    LList *tokens = LList_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->tokens = tokens;
    return node;
}

Nst_Node *nst_new_node_full(Nst_Pos start, Nst_Pos end, int type, LList *nodes, LList *tokens)
{
    Nst_Node *node = malloc(sizeof(Nst_Node));

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

Nst_Node *nst_new_node_empty(Nst_Pos start, Nst_Pos end, int type)
{
    Nst_Node *node = malloc(sizeof(Nst_Node));

    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    node->start = start;
    node->end = end;
    node->type = type;

    LList *nodes = LList_new();
    if ( errno == ENOMEM )
    {
        free(node);
        return NULL;
    }

    node->nodes = nodes;

    LList *tokens = LList_new();
    if ( errno == ENOMEM )
    {
        nst_destroy_node(node);
        return NULL;
    }

    node->tokens = tokens;
    return node;
}

void nst_destroy_node(Nst_Node *node)
{
    if ( node == NULL )
        return;

    LList_destroy(node->tokens, nst_destroy_token);
    LList_destroy(node->nodes, nst_destroy_node);

    free(node);
}
