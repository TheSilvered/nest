#ifndef Nst_LLIST_H
#define Nst_LLIST_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct _llnode
{
    void *value;
    bool allocated;
    struct _llnode *next;
}
LLNode;

typedef struct
{
    LLNode *head;
    LLNode *tail;
    size_t size;
}
LList;

// Adds an element to the front
void LList_push(LList *llist, void *value, bool allocated);
// Adds an element to the back
void LList_append(LList *llist, void *value, bool allocated);
// Removes and returns an element from the front
void *LList_pop(LList *llist);
// Returns the value from the head node
void *LList_peek_front(LList *llist);
// Returns the value from the tail node
void *LList_peek_back(LList *llist);

// Creates a new LList on the heap
LList *LList_new();
/* Frees the list and all the values inside the nodes.
The value of the node is passed to 'item_destroy_func' when 'allocated' is true
When 'allocated' is true but 'item_destroy_func' is NULL, the value is not freed */
void LList_destroy(LList *llist, void (*item_destroy_func)(void *));
/* Frees all the values inside the list but mantains the list
If 'item_destroy_func' is NULL and 'allocated' is true, the item is not freed */
void LList_empty(LList *llist, void (*item_destroy_func)(void *));

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !Nst_LLIST_H
