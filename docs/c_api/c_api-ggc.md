# `ggc.h`

This header contains structs and functions related to the generational garbage
collector.

## Macros

### `GGC_OBJ`

**Synopsis**:

```better-c
GGC_OBJ(obj)
```

**Description**:

Casts an object to `Nst_GGCObj *`.

---

### `NST_OBJ_IS_TRACKED`

**Synopsis**:

```better-c
NST_OBJ_IS_TRACKED(obj)
```

**Description**:

Checks whether the object is tracked by the garbage collector.

---

### `NST_GGC_HEAD`

**Description**:

Adds the necessary fields to an object struct to make it traceable by the
garbage collector.  
It must always be inserted after `NST_OBJ_HEAD` and before any additional fields.

**Added fields**:

- `ggc_next`
- `ggc_prev`
- `ggc_list`
- `traverse_func`
- `track_func`

Check [`Nst_GGCObj`](#nst_ggcobj) for their function.

---

### `NST_GGC_OBJ_INIT`

**Synopsis**:

```better-c
NST_GGC_OBJ_INIT(obj, trav_func, track_function)
```

**Description**:

Initializes all the fields added by `NST_GGC_HEAD`.

**Arguments**:

- `obj`: the object of which the fields must be initialized
- `trav_func`: the traverse function of the object
- `track_function`: the tracking function of the object

---

## Structs

### `Nst_GGCObj`

**Synopsis**:

```better-c
typedef struct _Nst_GGCObj
{
    NST_OBJ_HEAD;
    NST_GGC_HEAD;
}
Nst_GGCObj;
```

**Description**:

The structure containing all the fields in common with any object that is
traceable by the garbage collector.

**Fields**:

- `ggc_next`: the next object contained in the `Nst_GGCList`
- `ggc_prev`: the previous object contained in the `Nst_GGCList`
- `ggc_list`: the `Nst_GGCList` the object is inside of, this is the list that
  `ggc_next` and `ggc_pref` refer to
- `traverse_func`: the function that adds the `NST_FLAG_GGC_REACHABLE` flag to
  any objects that the object references
- `track_func`: the function that tracks any objects referenced by the object
  with the [`nst_ggc_track_obj`](#nst_ggc_track_obj) function

**Example**:

The following two functions are used by the builtin `Map` type:

```better-c
void _nst_map_traverse(Nst_MapObj *map)
{
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        NST_FLAG_SET(map->nodes[i].key,   NST_FLAG_GGC_REACHABLE);
        NST_FLAG_SET(map->nodes[i].value, NST_FLAG_GGC_REACHABLE);
    }
}

void _nst_map_track(Nst_MapObj *map)
{
    for ( i32 i = nst_map_get_next_idx(-1, map);
          i != -1;
          i = nst_map_get_next_idx(i, map) )
    {
        if ( NST_FLAG_HAS(map->nodes[i].value, NST_FLAG_GGC_IS_SUPPORTED) )
        {
            nst_ggc_track_obj((Nst_GGCObj*)(map->nodes[i].value));
        }
    }
}
```

The `NST_FLAG_GGC_REACHABLE` flag is reserved for any object and can be set
even if it is not traceable by the garbage collector, as it happens in
`_nst_map_traverse`.

---

### `Nst_GGCList`

**Synopsis**:

```better-c
typedef struct _Nst_GGCList
{
    Nst_GGCObj *head;
    Nst_GGCObj *tail;
    usize size;
}
Nst_GGCList;
```

**Description**:

A structure defining a doubly linked list of objects belonging to the same
generation.

**Fields**:

- `head`: the first object in the list
- `tail`: the last object in the list
- `size`: the number of objects in the list

---

### `Nst_GarbageCollector`

**Synopsis**:

```better-c
typedef struct _Nst_GarbageCollector
{
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    Nst_Int old_gen_pending;
}
Nst_GarbageCollector;
```

**Description**:

The structure of the garbage collector holding the generations of the tracked
objects.

---

## Functions

### `nst_ggc_collect_gen`

**Synopsis**:

```better-c
void nst_ggc_collect_gen(Nst_GGCList *gen,
                         Nst_GGCList *other_gen1,
                         Nst_GGCList *other_gen2,
                         Nst_GGCList *other_gen3)
```

**Description**:

Deletes the objects that have no external reference inside of a certain
generation.

---

### `nst_ggc_collect`

**Synopsis**:

```better-c
void nst_ggc_collect(void)
```

**Description**:

Collects the generations if needed and moves the remaining objects to the next
one.

---

### `nst_ggc_track_obj`

**Synopsis**:

```better-c
void nst_ggc_track_obj(Nst_GGCObj *obj)
```

**Description**:

Sets `obj` to be tracked. If it is already tracked, the function does not fail.

**Arguments**:

- `[in] obj`: the object to track

---

### `nst_ggc_delete_objs`

**Synopsis**:

```better-c
void nst_ggc_delete_objs(Nst_GarbageCollector *ggc)
```

**Description**:

Deletes all the objects still tracked by the garbage collector.

**Arguments**:

- `ggc`: the garbage collector of which to delete the objects

---

## Enums

### `Nst_GGCFlag`

**Synopsis**:

```better-c
typedef enum _Nst_GGCFlag
{
    NST_FLAG_GGC_REACHABLE    = 0b10000000,
    NST_FLAG_GGC_UNREACHABLE  = 0b01000000,
    NST_FLAG_GGC_DELETED      = 0b00100000,
    NST_FLAG_GGC_IS_SUPPORTED = 0b00010000
}
Nst_GGCFlag;
```

**Description**:

The flags used to track the state of objects during a collection. They are
reserved for any object, even if it is not traceable.
