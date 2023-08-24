# `ggc.h`

Generational Garbage Collector (GGC).

## Authors

TheSilvered

## Macros

### `_Nst_GEN1_MAX`

**Description:**

The maximum number of objects inside the first generation.

---

### `_Nst_GEN2_MAX`

**Description:**

The maximum number of objects inside the second generation.

---

### `_Nst_GEN3_MAX`

**Description:**

The maximum number of objects inside the third generation.

---

### `_Nst_OLD_GEN_MIN`

**Description:**

The minimum size of the old generation needed to collect it.

---

### `GGC_OBJ`

**Synopsis:**

```better-c
GGC_OBJ(obj)
```

**Description:**

Casts obj to [`Nst_GGCObj *`](c_api-ggc.md/#nst_ggcobj).

---

### `Nst_OBJ_IS_TRACKED`

**Synopsis:**

```better-c
Nst_OBJ_IS_TRACKED(obj)
```

**Description:**

Checks whether a [`Nst_GGCObj`](c_api-ggc.md/#nst_ggcobj) is tracked by the
garbage collector.

---

### `Nst_GGC_HEAD`

**Description:**

The macro to add support to the GGC to an object structure.

It must be placed after [`Nst_OBJ_HEAD`](c_api-obj.md/#nst_obj_head) and before
any other fields.

---

### `Nst_GGC_OBJ_INIT`

**Synopsis:**

```better-c
Nst_GGC_OBJ_INIT(obj, trav_func, track_function)
```

**Description:**

Initializes the fields of a [`Nst_GGCObj`](c_api-ggc.md/#nst_ggcobj).

---

## Structs

### `Nst_GGCObj`

**Synopsis:**

```better-c
typedef struct _Nst_GGCObj {
    Nst_OBJ_HEAD;
    Nst_GGC_HEAD;
} Nst_GGCObj
```

**Description:**

The struct representing a garbage collector object.

**Fields:**

- `ggc_next`: the next object in the generation it belongs to
- `ggc_prev`: the previous object in the generation it belongs to
- `ggc_list`: the generation it belongs to
- `traverse_func`: the function that sets as reachable all the objects contained
  in the object
- `track_func`: the function that tracks all the traceable objects that the
  object contains when it itself is tracked

---

### `Nst_GGCList`

**Synopsis:**

```better-c
typedef struct _Nst_GGCList {
    Nst_GGCObj *head;
    Nst_GGCObj *tail;
    usize len;
} Nst_GGCList
```

**Description:**

The structure representing a garbage collector generation.

**Fields:**

- `head`: the first object in the generation
- `tail`: the last object in the generation
- `len`: the total number of objects in the generation

---

### `Nst_GarbageCollector`

**Synopsis:**

```better-c
typedef struct _Nst_GarbageCollector {
    Nst_GGCList gen1;
    Nst_GGCList gen2;
    Nst_GGCList gen3;
    Nst_GGCList old_gen;
    i64 old_gen_pending;
} Nst_GarbageCollector
```

**Description:**

The structure representing the garbage collector.

**Fields:**

- `gen1`: the first generation
- `gen2`: the second generation
- `gen3`: the third generation
- `old_gen`: the old generation
- `old_gen_pending`: the number of objects in the old generation that have been
  added since its last collection

---

## Functions

### `Nst_ggc_collect_gen`

**Synopsis:**

```better-c
void Nst_ggc_collect_gen(Nst_GGCList *gen, Nst_GGCList *other_gen1,
                         Nst_GGCList *other_gen2, Nst_GGCList *other_gen3)
```

**Description:**

Collects the object of a generation

---

### `Nst_ggc_collect`

**Synopsis:**

```better-c
void Nst_ggc_collect(void)
```

**Description:**

Runs a general collection, that collects generations as needed.

---

### `Nst_ggc_track_obj`

**Synopsis:**

```better-c
void Nst_ggc_track_obj(Nst_GGCObj *obj)
```

**Description:**

Adds an object to the tracked objects by the garbage collector.

---

### `Nst_ggc_delete_objs`

**Synopsis:**

```better-c
void Nst_ggc_delete_objs(void)
```

**Description:**

Deletes all objects still present in the garbage collector.

---

### `Nst_ggc_init`

**Synopsis:**

```better-c
void Nst_ggc_init(void)
```

**Description:**

Initializes the garbage collector of
[`Nst_state`](c_api-interpreter.md/#nst_get_state).

---

## Enums

### `Nst_GGCFlags`

**Synopsis:**

```better-c
typedef enum _Nst_GGCFlags {
    Nst_FLAG_GGC_REACHABLE    = 0x80000000,
    Nst_FLAG_GGC_UNREACHABLE  = 0x40000000,
    Nst_FLAG_GGC_DELETED      = 0x20000000,
    Nst_FLAG_GGC_IS_SUPPORTED = 0x10000000
} Nst_GGCFlags
```

**Description:**

The flags of a garbage collector object.

