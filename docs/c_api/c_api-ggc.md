# `ggc.h`

Generational Garbage Collector (GGC).

## Authors

TheSilvered

---

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

### `Nst_GGC_HEAD`

**Description:**

The macro to add support to the GGC to an object structure.

It must be placed after [`Nst_OBJ_HEAD`](c_api-obj.md#nst_obj_head) and before
any other fields.

---

### `Nst_GGC_OBJ_INIT`

**Synopsis:**

```better-c
#define Nst_GGC_OBJ_INIT(obj)
```

**Description:**

Initializes the fields of a [`Nst_GGCObj`](c_api-ggc.md#nst_ggcobj). Should be
called after having initialized all the other fields of the object.

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

The structure representing a generation of the garbage collector.

**Fields:**

- `head`: the first object in the generation
- `tail`: the last object in the generation
- `len`: the total number of objects in the generation

---

## Functions

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

### `Nst_ggc_obj_reachable`

**Synopsis:**

```better-c
void Nst_ggc_obj_reachable(Nst_Obj *obj)
```

**Description:**

Sets an [`Nst_Obj`](c_api-typedefs.md#nst_obj) as reachable for the garbage
collector.

---

## Enums

### `Nst_GGCFlags`

**Synopsis:**

```better-c
typedef enum _Nst_GGCFlags {
    Nst_FLAG_GGC_REACHABLE    = Nst_FLAG(32),
    Nst_FLAG_GGC_PRESERVE_MEM = Nst_FLAG(31),
    Nst_FLAG_GGC_IS_SUPPORTED = Nst_FLAG(30)
} Nst_GGCFlags
```

**Description:**

The flags of a garbage collector object.
