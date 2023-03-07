#include <errno.h>
#include "mem.h"
#include "llist.h"

static inline Nst_LLNode *LLNode_new(void *value, bool allocated)
{
    Nst_LLNode *node = (Nst_LLNode *)nst_malloc(1, sizeof(Nst_LLNode));
    if ( node == NULL )
    {
        return NULL;
    }
    node->allocated = allocated;
    node->next = NULL;
    node->value = value;
    return node;
}

void nst_llist_push(Nst_LList *llist, void *value, bool allocated)
{
    Nst_LLNode *node = LLNode_new(value, allocated);
    if ( node == NULL )
    {
        return;
    }

    node->next = llist->head;
    llist->head = node;

    if ( llist->tail == NULL )
    {
        llist->tail = node;
    }
    llist->size++;
}

void nst_llist_append(Nst_LList *llist, void *value, bool allocated)
{
    Nst_LLNode *node = LLNode_new(value, allocated);
    if ( node == NULL )
    {
        return;
    }

    if ( llist->tail != NULL )
    {
        llist->tail->next = node;
    }

    llist->tail = node;

    if ( llist->head == NULL )
    {
        llist->head = node;
    }
    llist->size++;
}

void *nst_llist_pop(Nst_LList *llist)
{
    if ( llist->size == 0 )
    {
        return NULL;
    }

    Nst_LLNode *node = llist->head;
    llist->head = llist->head->next;

    if ( llist->head == NULL )
    {
        llist->tail = NULL;
    }

    llist->size--;
    void *value = node->value;
    nst_free(node);
    return value;
}

Nst_LList *nst_llist_new()
{
    Nst_LList *llist = (Nst_LList *)nst_malloc(1, sizeof(Nst_LList));
    if ( llist == NULL )
    {
        return NULL;
    }

    llist->size = 0;
    llist->head = NULL;
    llist->tail = NULL;

    return llist;
}

void *nst_llist_peek_front(Nst_LList *llist)
{
    if ( llist->head == NULL )
    {
        return NULL;
    }
    return llist->head->value;
}

void *nst_llist_peek_back(Nst_LList *llist)
{
    if ( llist->tail == NULL )
    {
        return NULL;
    }
    return llist->tail->value;
}

void nst_llist_destroy(Nst_LList *llist, void (*item_destroy_func)(void*))
{
    nst_llist_empty(llist, item_destroy_func);
    nst_free(llist);
}

void nst_llist_empty(Nst_LList *llist, void (*item_destroy_func)(void *))
{
    if ( llist == NULL )
    {
        return;
    }

    Nst_LLNode *prev = NULL;

    for ( Nst_LLNode *cursor = llist->head; cursor != NULL; )
    {
        if ( cursor->allocated && item_destroy_func != NULL )
            (*item_destroy_func)(cursor->value);
        prev = cursor;
        cursor = cursor->next;
        llist->size--;
        nst_free(prev);
    }
    llist->head = NULL;
    llist->tail = NULL;
}
