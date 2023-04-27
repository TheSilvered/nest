#include <errno.h>
#include "mem.h"
#include "llist.h"

static inline Nst_LLNode *LLNode_new(void              *value,
                                     bool               allocated,
                                     struct _Nst_OpErr *err)
{
    Nst_LLNode *node = (Nst_LLNode *)nst_malloc(1, sizeof(Nst_LLNode), err);
    if ( node == NULL )
    {
        return NULL;
    }
    node->allocated = allocated;
    node->next = NULL;
    node->value = value;
    return node;
}

bool nst_llist_push(Nst_LList         *llist,
                    void              *value,
                    bool               allocated,
                    struct _Nst_OpErr *err)
{
    Nst_LLNode *node = LLNode_new(value, allocated, err);
    if ( node == NULL )
    {
        return false;
    }

    node->next = llist->head;
    llist->head = node;

    if ( llist->tail == NULL )
    {
        llist->tail = node;
    }
    llist->size++;
    return true;
}

bool nst_llist_append(Nst_LList *llist,
                      void *value,
                      bool allocated,
                      struct _Nst_OpErr *err)
{
    Nst_LLNode *node = LLNode_new(value, allocated, err);
    if ( node == NULL )
    {
        return false;
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
    return true;
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

Nst_LList *nst_llist_new(struct _Nst_OpErr *err)
{
    Nst_LList *llist = (Nst_LList *)nst_malloc(1, sizeof(Nst_LList), err);
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
        {
            (*item_destroy_func)(cursor->value);
        }
        prev = cursor;
        cursor = cursor->next;
        llist->size--;
        nst_free(prev);
    }
    llist->head = NULL;
    llist->tail = NULL;
}

void nst_llist_move_nodes(Nst_LList *from, Nst_LList *to)
{
    if ( to->size == 0 )
    {
        to->head = from->head;
        to->tail = from->tail;
        to->size = from->size;
    }
    else
    {
        to->tail->next = from->head;
        to->tail = from->tail;
        to->size += from->size;
    }

    from->head = NULL;
    from->tail = NULL;
    from->size = 0;
}

void nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node)
{
    node->next = llist->head;
    llist->head = node;
    llist->size++;
    if ( llist->size == 1 )
    {
        llist->tail = node;
    }
}

void nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node)
{
    node->next = NULL;
    if ( llist->size > 0 )
    {
        llist->tail->next = node;
    }
    else
    {
        llist->head = node;
    }
    llist->tail = node;
    llist->size++;
}

Nst_LLNode *nst_llist_pop_llnode(Nst_LList *llist)
{
    if ( llist->size == 0 )
    {
        return NULL;
    }
    Nst_LLNode *node = llist->head;
    llist->head = node->next;
    llist->size--;
    if ( llist->size == 0 )
    {
        llist->tail = NULL;
    }
    node->next = NULL;
    return node;
}
