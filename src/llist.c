#include <errno.h>
#include "nest.h"

Nst_LLNode *Nst_llnode_new(void *value, bool allocated)
{
    Nst_LLNode *node = Nst_malloc_c(1, Nst_LLNode);
    if (node == NULL)
        return NULL;
    node->allocated = allocated;
    node->next = NULL;
    node->value = value;
    return node;
}

bool Nst_llist_push(Nst_LList *llist, void *value, bool allocated)
{
    Nst_LLNode *node = Nst_llnode_new(value, allocated);
    if (node == NULL)
        return false;

    node->next = llist->head;
    llist->head = node;

    if (llist->tail == NULL)
        llist->tail = node;
    llist->len++;
    return true;
}

bool Nst_llist_append(Nst_LList *llist, void *value, bool allocated)
{
    Nst_LLNode *node = Nst_llnode_new(value, allocated);
    if (node == NULL)
        return false;

    if (llist->tail != NULL)
        llist->tail->next = node;

    llist->tail = node;

    if (llist->head == NULL)
        llist->head = node;
    llist->len++;
    return true;
}

bool Nst_llist_insert(Nst_LList *llist, void *value, bool allocated,
                      Nst_LLNode *node)
{
    Nst_LLNode *new_node = Nst_llnode_new(value, allocated);
    if (new_node == NULL)
        return false;

    if (!node) {
        Nst_llist_push_llnode(llist, new_node);
        return true;
    }

    new_node->next = node->next;
    node->next = new_node;
    if (new_node->next == NULL)
        llist->tail = new_node;
    llist->len++;
    return true;
}

void *Nst_llist_pop(Nst_LList *llist)
{
    if (llist->len == 0)
        return NULL;

    Nst_LLNode *node = llist->head;
    llist->head = llist->head->next;

    if (llist->head == NULL)
        llist->tail = NULL;

    llist->len--;
    void *value = node->value;
    Nst_free(node);
    return value;
}

void Nst_llist_init(Nst_LList *llist)
{
    llist->len = 0;
    llist->head = NULL;
    llist->tail = NULL;
}

void *Nst_llist_peek_front(Nst_LList *llist)
{
    if (llist->head == NULL)
        return NULL;
    return llist->head->value;
}

void *Nst_llist_peek_back(Nst_LList *llist)
{
    if (llist->tail == NULL)
        return NULL;
    return llist->tail->value;
}

void Nst_llist_empty(Nst_LList *llist, Nst_Destructor item_destructor)
{
    if (llist == NULL)
        return;
    Nst_LLNode *prev = NULL;

    for (Nst_LLNode *cursor = llist->head; cursor != NULL;) {
        if (cursor->allocated && item_destructor != NULL)
            item_destructor(cursor->value);
        prev = cursor;
        cursor = cursor->next;
        Nst_free(prev);
    }
    llist->head = NULL;
    llist->tail = NULL;
    llist->len = 0;
}

void Nst_llist_move_nodes(Nst_LList *from, Nst_LList *to)
{
    if (to->len == 0) {
        to->head = from->head;
        to->tail = from->tail;
        to->len = from->len;
    } else {
        to->tail->next = from->head;
        to->tail = from->tail;
        to->len += from->len;
    }

    from->head = NULL;
    from->tail = NULL;
    from->len = 0;
}

void Nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node)
{
    node->next = llist->head;
    llist->head = node;
    llist->len++;
    if (llist->len == 1)
        llist->tail = node;
}

void Nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node)
{
    node->next = NULL;
    if (llist->len > 0)
        llist->tail->next = node;
    else
        llist->head = node;
    llist->tail = node;
    llist->len++;
}

Nst_LLNode *Nst_llist_pop_llnode(Nst_LList *llist)
{
    if (llist->len == 0)
        return NULL;
    Nst_LLNode *node = llist->head;
    llist->head = node->next;
    llist->len--;
    if (llist->len == 0)
        llist->tail = NULL;
    node->next = NULL;
    return node;
}
