# `llist.h`

This header contains various functions to operate with singly-linked lists.

## Macros

### `NST_LLIST_ITER`

**Synopsis**:

```better-c
NST_LLIST_ITER(node, llist)
```

**Description**:

Sets up a for-loop to iterate though the nodes in `llist`.

**Arguments**:

- `node`: the name of the variable of the loop
- `llist`: the list to iterate on

---

## Structs

### `Nst_LLNode`

**Synopsis**:

```better-c
typedef struct _Nst_LLNnode
{
    void *value;
    bool allocated;
    struct _Nst_LLNnode *next;
}
Nst_LLNode
```

**Description**:

The node of a linked list.

**Fields**:

- `value`: the value of the node
- `allocated`: whether the value of the node should be freed when the list is
  destroyed
- `next`: the next node in the list

---

### `Nst_LList`

**Synopsis**:

```better-c
typedef struct _Nst_LList
{
    Nst_LLNode *head;
    Nst_LLNode *tail;
    usize size;
}
Nst_LList
```

**Description**:

The struct for a linked list.

**Fields**:

- `head`: the first node in the list
- `tail`: the last node in the list
- `size`: the total number of nodes in the list

---

## Type aliases

### `Nst_LListDestructor`

**Synopsis**:

```better-c
typedef void (*Nst_LListDestructor)(void *)
```

---

## Functions

### `nst_llist_new`

**Synopsis**:

```better-c
Nst_LList *nst_llist_new(Nst_OpErr *err)
```

**Description**:

Creates a new `Nst_LList` on the heap.

**Arguments**:

- `[out] err`: the error

**Return value**:

The function returns the created list on success and `NULL` on failure.

---

### `nst_llnode_new`

**Synopsis**:

```better-c
Nst_LLNode *nst_llnode_new(void      *value,
                           bool       allocated,
                           Nst_OpErr *err)
```

**Description**:

Creates a new `Nst_LLNode` on the heap.

**Arguments**:

- `[in] value`: the value of the node
- `[in] allocated`: whether the value is allocated
- `[out] err`: the error

**Return value**:

The function returns the created node on success and `NULL` on failure.

---

### `nst_llist_push`

**Synopsis**:

```better-c
bool nst_llist_push(Nst_LList *llist,
                    void      *value,
                    bool       allocated,
                    Nst_OpErr *err)
```

**Description**:

Inserts an element to the front.

**Arguments**:

- `[inout] llist`: the list to add the element to
- `[in] value`: the value to insert
- `[in] allocated`: whether the value needs to be freed when the list is
  destroyed
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_llist_append`

**Synopsis**:

```better-c
bool nst_llist_append(Nst_LList *llist,
                      void      *value,
                      bool       allocated,
                      Nst_OpErr *err)
```

**Description**:

Appends an element to the back.

**Arguments**:

- `[inout] llist`: the list to add the element to
- `[in] value`: the value to append
- `[in] allocated`: whether the value needs to be freed when the list is
  destroyed
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_llist_insert`

**Synopsis**:

```better-c
bool nst_llist_insert(Nst_LList  *llist,
                      void       *value,
                      bool        allocated,
                      Nst_LLNode *node,
                      Nst_OpErr  *err)
```

**Description**:

Inserts an element in `llist` after `node`.

**Arguments**:

- `[inout] llist`: the list to add the element to
- `[in] value`: the value to insert
- `[in] allocated`: whether the value needs to be freed when the list is
  destroyed
- `[inout] node`: the node that should precede the inserted value
- `[out] err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `nst_llist_pop`

**Synopsis**:

```better-c
void *nst_llist_pop(Nst_LList *llist)
```

**Description**:

Removes a node from the front.

**Arguments**:

- `llist`: the list to remove the element from

**Return value**:

The function returns the value inside the node.

---

### `nst_llist_peek_front`

**Synopsis**:

```better-c
void *nst_llist_peek_front(Nst_LList *llist)
```

**Arguments**:

- `[in] llist`: the list to get the value from

**Return value**:

The function returns the value from the head node.

---

### `nst_llist_peek_back`

**Synopsis**:

```better-c
void *nst_llist_peek_back(Nst_LList *llist)
```

**Arguments**:

- `[in] llist`: the list to get the value from

**Return value**:

The function returns the value from the tail node.

---

### `nst_llist_push_llnode`

**Synopsis**:

```better-c
void nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node)
```

**Description**:

Inserts `node` to the front of `llist`.

**Arguments**:

- `[inout] llist`: the list to add the node to
- `[in] node`: the node to add

---

### `nst_llist_append_llnode`

**Synopsis**:

```better-c
void nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node)
```

**Description**:

Appends `node` to the back of `llist`.

**Arguments**:

- `[inout] llist`: the list to add the node to
- `[in] node`: the node to add

---

### `nst_llist_pop_llnode`

**Synopsis**:

```better-c
Nst_LLNode *nst_llist_pop_llnode(Nst_LList *llist)
```

**Description**:

Removes the front node of `llist`.

**Arguments**:

- `llist`: the list to pop the node from

**Return value**:

The function returns the popped node.

---

### `nst_llist_destroy`

**Synopsis**:

```better-c
void nst_llist_destroy(Nst_LList *llist, void (*item_destroy_func)(void *))
```

**Description**:

Frees the list and all the values inside the nodes. The values of the nodes are
passed to `item_destroy_func` when `allocated` is `true`. If `item_destroy_func`
is `NULL` the values are not freed.

**Arguments**:

- `[in] llist`: the list to destroy
- `[in] item_destroy_func`: the destructor for the values of the nodes

---

### `nst_llist_empty`

**Synopsis**:

```better-c
void nst_llist_empty(Nst_LList *llist, void (*item_destroy_func)(void *))
```

**Description**:

Frees all the values inside the list but maintains the list. If
`item_destroy_func` is `NULL` the values are not freed.

**Arguments**:

- `[inout] llist`: the list to destroy
- `[in] item_destroy_func`: the destructor for the values of the nodes

---

### `nst_llist_move_nodes`

**Synopsis**:

```better-c
void nst_llist_move_nodes(Nst_LList *from, Nst_LList *to)
```

**Description**:

Moves the contents of one llist to another. If `to` is not empty the nodes are
appended at the end.

**Arguments**:

- `[in] from`: the list from which to move the nodes
- `[inout] to`: the list to add the nodes to
