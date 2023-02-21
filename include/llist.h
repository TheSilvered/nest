/* Linked list interface */

#ifndef LLIST_H
#define LLIST_H

#include "typedefs.h"

// Initialize a for loop on a list
#define NST_LLIST_ITER(node, llist) \
    Nst_LLNode *node = llist->head; \
    node != NULL; \
    node = node->next

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _Nst_LLNnode
{
    void *value;
    bool allocated;
    struct _Nst_LLNnode *next;
}
Nst_LLNode;

typedef struct
{
    Nst_LLNode *head;
    Nst_LLNode *tail;
    usize size;
}
Nst_LList;

typedef void (*nst_llist_destructor)(void *);

// Adds an element to the front
void nst_llist_push(Nst_LList *llist, void *value, bool allocated);
// Adds an element to the back
void nst_llist_append(Nst_LList *llist, void *value, bool allocated);
// Removes and returns an element from the front
void *nst_llist_pop(Nst_LList *llist);
// Returns the value from the head node
void *nst_llist_peek_front(Nst_LList *llist);
// Returns the value from the tail node
void *nst_llist_peek_back(Nst_LList *llist);

// Creates a new LList on the heap
Nst_LList *nst_llist_new();
// Frees the list and all the values inside the nodes.
// The value of the node is passed to 'item_destroy_func' when 'allocated' is true
// When 'allocated' is true but 'item_destroy_func' is NULL, the value is not freed
void nst_llist_destroy(Nst_LList *llist, void (*item_destroy_func)(void *));
// Frees all the values inside the list but mantains the list
// If 'item_destroy_func' is NULL and 'allocated' is true, the item is not freed
void nst_llist_empty(Nst_LList *llist, void (*item_destroy_func)(void *));

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LLIST_H
