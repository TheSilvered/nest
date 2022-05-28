#include <stdlib.h>
#include <errno.h>
#include "nodes.h"
#include "tokens.h"

Node *new_node_tokens(Pos start, Pos end, int type, LList *tokens)
{
    Node *node = malloc(sizeof(Node));

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

Node *new_node_nodes(Pos start, Pos end, int type, LList *nodes)
{
    Node *node = malloc(sizeof(Node));

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

Node *new_node_full(Pos start, Pos end, int type, LList *nodes, LList *tokens)
{
    Node *node = malloc(sizeof(Node));

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

Node *new_node_empty(Pos start, Pos end, int type)
{
    Node *node = malloc(sizeof(Node));

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
        destroy_node(node);
        return NULL;
    }

    node->tokens = tokens;
    return node;
}

void destroy_node(Node *node)
{
    if ( node == NULL )
        return;

    LList_destroy(node->tokens, destroy_token);
    LList_destroy(node->nodes, destroy_node);

    free(node);
}
