/**
 * @file llist.h
 *
 * @brief Singly-linked list
 *
 * @author TheSilvered
 */

#ifndef LLIST_H
#define LLIST_H

#include "typedefs.h"

/**
 * Loops through the list given.
 *
 * @brief It should be put inside the brackets of a for loop with nothing
 * else.
 *
 * @param node: the name of the variable holding the current node in the
 * iteration
 * @param llist: the list to iterate
 */
#define Nst_LLIST_ITER(node, llist)                                           \
    Nst_LLNode *node = llist->head;                                           \
    node != NULL;                                                             \
    node = node->next

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * The structure representing a node of a `Nst_LList`.
 *
 * @param value: the value that the node contains
 * @param allocated: whether the value contained in the node was allocated
 * @param next: the next node in the list
 */
NstEXP typedef struct _Nst_LLNode {
    void *value;
    bool allocated;
    struct _Nst_LLNode *next;
} Nst_LLNode;

/**
 * The structure representing a linked list.
 *
 * @param head: the first node in the list
 * @param tail: the last node in the list
 * @param len: the total number of nodes in the list
 */
NstEXP typedef struct _Nst_LList {
    Nst_LLNode *head;
    Nst_LLNode *tail;
    usize len;
} Nst_LList;

/**
 * Add a value to the front of the list.
 *
 * @param llist: the list to add the value to
 * @param value: the value to add
 * @param allocated: whether to pass this value to the destructor when the list
 * is destroyed
 *
 * @return `true` if the value is added and `false` on failure. The error is
 * set.
 */
NstEXP bool NstC Nst_llist_push(Nst_LList *llist, void *value, bool allocated);
/**
 * Add a value to the back of the list.
 *
 * @param llist: the list to add the value to
 * @param value: the value to add
 * @param allocated: whether to pass this value to the destructor when the list
 * is destroyed
 *
 * @return `true` if the value is added and `false` on failure. The error is
 * set.
 */
NstEXP bool NstC Nst_llist_append(Nst_LList *llist, void *value,
                                  bool allocated);
/**
 * Add a value after a given node.
 *
 * @brief node can be `NULL`, in which case the function behaves like
 * `Nst_llist_push`.
 *
 * @param llist: the list to add the value to
 * @param value: the value to add
 * @param allocated: whether to pass this value to the destructor when the list
 * is destroyed
 * @param node: the node belonging to the list after which the value is
 * inserted
 *
 * @return `true` if the value is added and `false` on failure. The error is
 * set.
 */
NstEXP bool NstC Nst_llist_insert(Nst_LList *llist, void *value,
                                  bool allocated, Nst_LLNode *node);
/**
 * Remove and returns the front value of a list. If the list is empty `NULL` is
 * returned, no error is set.
 */
NstEXP void *NstC Nst_llist_pop(Nst_LList *llist);
/**
 * @return The front value of a list. If the list is empty `NULL` is* returned,
 * no error is set.
 */
NstEXP void *NstC Nst_llist_peek_front(Nst_LList *llist);
/**
 * @return The back value of a list. If the list is empty `NULL` is returned,
 * no error is set.
 */
NstEXP void *NstC Nst_llist_peek_back(Nst_LList *llist);
/* Add a node to the front of a list. */
NstEXP void NstC Nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node);
/* Add a node to the back of a list. */
NstEXP void NstC Nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node);
/**
 * Remove the front node of a list and returns it. If the list is empty `NULL`
 * is returned, no error is set.
 */
NstEXP Nst_LLNode *NstC Nst_llist_pop_llnode(Nst_LList *llist);

/* Initialize an `Nst_LList`. */
NstEXP void NstC Nst_llist_init(Nst_LList *llist);

/**
 * Create a new node on the heap.
 *
 * @brief To free the node use `Nst_free`. The value is *not* handled
 * automatically and must be freed manually.
 *
 * @return The new node or `NULL` on failure. The error is set.
 */
NstEXP Nst_LLNode *NstC Nst_llnode_new(void *value, bool allocated);

/**
 * Destroy all the nodes inside a list.
 *
 * @brief If the value of a node is marked as allocated it will be passed to
 * item_destructor otherwise it is left untouched.
 *
 * @param llist: the list to empty
 * @param item_destructor: the destructor to use on allocated values
 */
NstEXP void NstC Nst_llist_empty(Nst_LList *llist,
                                 Nst_Destructor item_destructor);
/**
 * Move all the nodes from a list to the end of another.
 *
 * @brief If to has already some values the new ones are added at the end.
 *
 * @param from: the list to move the nodes from
 * @param to: the list to move the nodes to
 */
NstEXP void NstC Nst_llist_move_nodes(Nst_LList *from, Nst_LList *to);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LLIST_H
