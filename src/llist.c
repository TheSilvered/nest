#include <errno.h>
#include <stdlib.h>
#include "llist.h"

static inline LLNode *LLNode_new(void *value, bool allocated)
{
    LLNode *node = (LLNode *)malloc(sizeof(LLNode));
    if ( node == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }
    node->allocated = allocated;
    node->next = NULL;
    node->value = value;
    return node;
}

void LList_push(LList *llist, void *value, bool allocated)
{
    LLNode *node = LLNode_new(value, allocated);
    if ( node == NULL )
    {
        errno = ENOMEM;
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

void LList_append(LList *llist, void *value, bool allocated)
{
    LLNode *node = LLNode_new(value, allocated);
    if ( node == NULL )
    {
        errno = ENOMEM;
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

void *LList_pop(LList *llist)
{
    if ( llist->size == 0 )
    {
        errno = ENOMEM;
        return NULL;
    }

    LLNode *node = llist->head;
    llist->head = llist->head->next;

    if ( llist->head == NULL )
    {
        llist->tail = NULL;
    }

    llist->size--;
    void *value = node->value;
    free(node);
    return value;
}

LList *LList_new()
{
    LList *llist = (LList *)malloc(sizeof(LList));
    if ( llist == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    llist->size = 0;
    llist->head = NULL;
    llist->tail = NULL;

    return llist;
}

void *LList_peek_front(LList *llist)
{
    if ( llist->head == NULL )
    {
        return NULL;
    }
    return llist->head->value;
}

void *LList_peek_back(LList *llist)
{
    if ( llist->tail == NULL )
    {
        return NULL;
    }
    return llist->tail->value;
}

void LList_destroy(LList *llist, void (*item_destroy_func)(void*))
{
    LList_empty(llist, item_destroy_func);
    free(llist);
}

void LList_empty(LList *llist, void (*item_destroy_func)(void *))
{
    if ( llist == NULL )
    {
        return;
    }

    LLNode *prev = NULL;

    for ( LLNode *cursor = llist->head; cursor != NULL; )
    {
        if ( cursor->allocated && item_destroy_func != NULL )
            (*item_destroy_func)(cursor->value);
        prev = cursor;
        cursor = cursor->next;
        llist->size--;
        free(prev);
    }
    llist->head = NULL;
    llist->tail = NULL;
}
