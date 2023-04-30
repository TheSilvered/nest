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

EXPORT typedef struct _Nst_LLNnode
{
    void *value;
    bool allocated;
    struct _Nst_LLNnode *next;
}
Nst_LLNode;

EXPORT typedef struct _Nst_LList
{
    Nst_LLNode *head;
    Nst_LLNode *tail;
    usize size;
}
Nst_LList;

struct _Nst_OpErr;

EXPORT typedef void (*Nst_LListDestructor)(void *);

// Adds an element to the front
EXPORT bool nst_llist_push(Nst_LList         *llist,
                           void              *value,
                           bool               allocated,
                           struct _Nst_OpErr *err);
// Adds an element to the back
EXPORT bool nst_llist_append(Nst_LList         *llist,
                             void              *value,
                             bool               allocated,
                             struct _Nst_OpErr *err);
// Inserts an element after node
EXPORT bool nst_llist_insert(Nst_LList         *llist,
                             void              *value,
                             bool               allocated,
                             Nst_LLNode        *node,
                             struct _Nst_OpErr *err);
// Removes and returns an element from the front
EXPORT void *nst_llist_pop(Nst_LList *llist);
// Returns the value from the head node
EXPORT void *nst_llist_peek_front(Nst_LList *llist);
// Returns the value from the tail node
EXPORT void *nst_llist_peek_back(Nst_LList *llist);

EXPORT void nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node);
EXPORT void nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node);
EXPORT Nst_LLNode *nst_llist_pop_llnode(Nst_LList *llist);

// Creates a new LList on the heap
EXPORT Nst_LList *nst_llist_new(struct _Nst_OpErr *err);
// Creates a new LLNode on the heap
EXPORT Nst_LLNode *nst_llnode_new(void              *value,
                                  bool               allocated,
                                  struct _Nst_OpErr *err);
// Frees the list and all the values inside the nodes.
// The value of the node is passed to 'item_destroy_func' when 'allocated' is true
// When 'allocated' is true but 'item_destroy_func' is NULL, the value is not freed
EXPORT void nst_llist_destroy(Nst_LList *llist, void (*item_destroy_func)(void *));
// Frees all the values inside the list but maintains the list
// If 'item_destroy_func' is NULL and 'allocated' is true, the item is not freed
EXPORT void nst_llist_empty(Nst_LList *llist, void (*item_destroy_func)(void *));
// Moves the contents of one llist to another
EXPORT void nst_llist_move_nodes(Nst_LList *from, Nst_LList *to);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LLIST_H
