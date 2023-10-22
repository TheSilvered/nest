# `llist.h`

Singly-linked list.

## Authors

TheSilvered

---

## Macros

### `Nst_LLIST_ITER`

**Synopsis:**

```better-c
#define Nst_LLIST_ITER(node, llist)
```

**Description:**

Loops through the list given.

It should be put inside the brackets of a for loop with nothing else.

**Parameters:**

- `node`: the name of the variable holding the current node in the iteration
- `llist`: the list to iterate

---

## Structs

### `Nst_LLNode`

**Synopsis:**

```better-c
typedef struct _Nst_LLNode {
    void *value;
    bool allocated;
    struct _Nst_LLNode *next;
} Nst_LLNode
```

**Description:**

The structure representing a node of a [`Nst_LList`](c_api-llist.md#nst_llist).

**Fields:**

- `value`: the value that the node contains
- `allocated`: whether the value contained in the node was allocated
- `next`: the next node in the list

---

### `Nst_LList`

**Synopsis:**

```better-c
typedef struct _Nst_LList {
    Nst_LLNode *head;
    Nst_LLNode *tail;
    usize len;
} Nst_LList
```

**Description:**

The structure representing a linked list.

**Fields:**

- `head`: the first node in the list
- `tail`: the last node in the list
- `len`: the total number of nodes in the list

---

## Type aliases

### `Nst_LListDestructor`

**Synopsis:**

```better-c
typedef void (*Nst_LListDestructor)(void *)
```

**Description:**

The type of a list destructor.

---

## Functions

### `Nst_llist_push`

**Synopsis:**

```better-c
bool Nst_llist_push(Nst_LList *llist, void *value, bool allocated)
```

**Description:**

Adds a value to the front of the list.

**Parameters:**

- `llist`: the list to add the value to
- `value`: the value to add
- `allocated`: whether to pass this value to the destructor when the list is
  destroyed

**Returns:**

`true` if the value is added and `false` on failure. The error is set.

---

### `Nst_llist_append`

**Synopsis:**

```better-c
bool Nst_llist_append(Nst_LList *llist, void *value, bool allocated)
```

**Description:**

Adds a value to the back of the list.

**Parameters:**

- `llist`: the list to add the value to
- `value`: the value to add
- `allocated`: whether to pass this value to the destructor when the list is
  destroyed

**Returns:**

`true` if the value is added and `false` on failure. The error is set.

---

### `Nst_llist_insert`

**Synopsis:**

```better-c
bool Nst_llist_insert(Nst_LList *llist, void *value, bool allocated,
                      Nst_LLNode *node)
```

**Description:**

Adds a value after a given node.

node can be `NULL`, in which case the function behaves like
[`Nst_llist_push`](c_api-llist.md#nst_llist_push).

**Parameters:**

- `llist`: the list to add the value to
- `value`: the value to add
- `allocated`: whether to pass this value to the destructor when the list is
  destroyed
- `node`: the node belonging to the list after which the value is inserted

**Returns:**

`true` if the value is added and `false` on failure. The error is set.

---

### `Nst_llist_pop`

**Synopsis:**

```better-c
void *Nst_llist_pop(Nst_LList *llist)
```

**Description:**

Removes and returns the front value from a list.

---

### `Nst_llist_peek_front`

**Synopsis:**

```better-c
void *Nst_llist_peek_front(Nst_LList *llist)
```

**Description:**

Returns the front value of a list. If the list is empty `NULL` is returned, no
error is set.

---

### `Nst_llist_peek_back`

**Synopsis:**

```better-c
void *Nst_llist_peek_back(Nst_LList *llist)
```

**Description:**

Returns the back value of a list. If the list is empty `NULL` is returned, no
error is set.

---

### `Nst_llist_push_llnode`

**Synopsis:**

```better-c
void Nst_llist_push_llnode(Nst_LList *llist, Nst_LLNode *node)
```

**Description:**

Adds a node to the front of a list.

---

### `Nst_llist_append_llnode`

**Synopsis:**

```better-c
void Nst_llist_append_llnode(Nst_LList *llist, Nst_LLNode *node)
```

**Description:**

Adds a node to the back of a list.

---

### `Nst_llist_pop_llnode`

**Synopsis:**

```better-c
Nst_LLNode *Nst_llist_pop_llnode(Nst_LList *llist)
```

**Description:**

Removes the front node of a list and returns it.

---

### `Nst_llist_new`

**Synopsis:**

```better-c
Nst_LList *Nst_llist_new(void)
```

**Description:**

Creates and initializes a new list on the heap

**Returns:**

The new list or `NULL` on failure. The error is set.

---

### `Nst_llist_init`

**Synopsis:**

```better-c
void Nst_llist_init(Nst_LList *llist)
```

**Description:**

Initializes a llist.

---

### `Nst_llnode_new`

**Synopsis:**

```better-c
Nst_LLNode *Nst_llnode_new(void *value, bool allocated)
```

**Description:**

Creates a new node on the heap.

**Returns:**

The new node or `NULL` on failure. The error is set.

---

### `Nst_llist_destroy`

**Synopsis:**

```better-c
void Nst_llist_destroy(Nst_LList *llist, void (*item_destructor)(void *))
```

**Description:**

Destroys a heap-allocated list.

If the value of a node is marked as allocated it will be passed to
item_destructor otherwise it is left untouched.

**Parameters:**

- `llist`: the list to destroy
- `item_destructor`: the destructor to use on allocated values

---

### `Nst_llist_empty`

**Synopsis:**

```better-c
void Nst_llist_empty(Nst_LList *llist, void (*item_destructor)(void *))
```

**Description:**

Destroys all the nodes inside list.

If the value of a node is marked as allocated it will be passed to
item_destructor otherwise it is left untouched.

**Parameters:**

- `llist`: the list to empty
- `item_destructor`: the destructor to use on allocated values

---

### `Nst_llist_move_nodes`

**Synopsis:**

```better-c
void Nst_llist_move_nodes(Nst_LList *from, Nst_LList *to)
```

**Description:**

Moves all the nodes from a list to the end of another.

If to has already some values the new ones are added at the end.

**Parameters:**

- `from`: the list to move the nodes from
- `to`: the list to move the nodes to
