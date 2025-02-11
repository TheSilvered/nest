# `global_consts.h`

Constant objects used in the program.

## Authors

TheSilvered

---

## Structs

### `Nst_StrConsts`

**Synopsis:**

```better-c
typedef struct _Nst_StrConsts {
    // Built-in type names

    Nst_Obj *t_Type;
    Nst_Obj *t_Int;
    Nst_Obj *t_Real;
    Nst_Obj *t_Bool;
    Nst_Obj *t_Null;
    Nst_Obj *t_Str;
    Nst_Obj *t_Array;
    Nst_Obj *t_Vector;
    Nst_Obj *t_Map;
    Nst_Obj *t_Func;
    Nst_Obj *t_Iter;
    Nst_Obj *t_Byte;
    Nst_Obj *t_IOFile;
    Nst_Obj *t_IEnd;

    // Constant strings

    Nst_Obj *c_true;
    Nst_Obj *c_false;
    Nst_Obj *c_null;
    Nst_Obj *c_inf;
    Nst_Obj *c_nan;
    Nst_Obj *c_neginf;
    Nst_Obj *c_negnan;

    // Standard error names

    Nst_Obj *e_SyntaxError;
    Nst_Obj *e_MemoryError;
    Nst_Obj *e_ValueError;
    Nst_Obj *e_TypeError;
    Nst_Obj *e_CallError;
    Nst_Obj *e_MathError;
    Nst_Obj *e_ImportError;
    Nst_Obj *e_Interrupt;

    // Other

    Nst_Obj *o__vars_;
    Nst_Obj *o__globals_;
    Nst_Obj *o__args_;
    Nst_Obj *o_failed_alloc;
} Nst_StrConsts
```

**Description:**

String constants.

Fields starting with `c_` are the name of language constants (e.g. true), the
one starting with `e_` are the name of errors and the ones starting with `o_`
are others.

---

### `Nst_TypeObjs`

**Synopsis:**

```better-c
typedef struct _Nst_TypeObjs {
    Nst_TypeObj *Type;
    Nst_TypeObj *Int;
    Nst_TypeObj *Real;
    Nst_TypeObj *Bool;
    Nst_TypeObj *Null;
    Nst_TypeObj *Str;
    Nst_TypeObj *Array;
    Nst_TypeObj *Vector;
    Nst_TypeObj *Map;
    Nst_TypeObj *Func;
    Nst_TypeObj *Iter;
    Nst_TypeObj *Byte;
    Nst_TypeObj *IOFile;
    Nst_TypeObj *IEnd;
} Nst_TypeObjs
```

**Description:**

Type constants.

---

### `Nst_Consts`

**Synopsis:**

```better-c
typedef struct _Nst_Consts {
    Nst_Obj *Bool_true;
    Nst_Obj *Bool_false;
    Nst_Obj *Null_null;
    Nst_Obj *IEnd_iend;
    Nst_Obj *Int_0;
    Nst_Obj *Int_1;
    Nst_Obj *Int_neg1;
    Nst_Obj *Real_0;
    Nst_Obj *Real_1;
    Nst_Obj *Real_nan;
    Nst_Obj *Real_negnan;
    Nst_Obj *Real_inf;
    Nst_Obj *Real_neginf;
    Nst_Obj *Byte_0;
    Nst_Obj *Byte_1;
} Nst_Consts
```

**Description:**

Other constants.

Each constants is preceded by the name of its type.

---

### `Nst_StdStreams`

**Synopsis:**

```better-c
typedef struct _Nst_StdStreams {
    Nst_Obj *in;
    Nst_Obj *out;
    Nst_Obj *err;
} Nst_StdStreams
```

**Description:**

Standard IO streams.

!!!note
    These are not constant and can change at run-time.

---

### `Nst_IterFunctions`

**Synopsis:**

```better-c
typedef struct _Nst_IterFunctions {
    Nst_Obj *range_start;
    Nst_Obj *range_next;

    Nst_Obj *seq_start;
    Nst_Obj *seq_next;

    Nst_Obj *str_start;
    Nst_Obj *str_next;

    Nst_Obj *map_start;
    Nst_Obj *map_next;
} Nst_IterFunctions
```

**Description:**

The functions of the built-in iterators.

---

## Functions

### `_Nst_globals_init`

**Synopsis:**

```better-c
bool _Nst_globals_init(void)
```

**Description:**

Initializes all the global constants and IO streams.

**Returns:**

`true` on success and `false` on failure. No error is set.

---

### `_Nst_globals_quit`

**Synopsis:**

```better-c
void _Nst_globals_quit(void)
```

**Description:**

Deletes all the global constants and IO streams.

---

### `Nst_true`

**Synopsis:**

```better-c
Nst_Obj *Nst_true(void)
```

**Description:**

Returns the true object constant.

---

### `Nst_true_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_true_ref(void)
```

**Description:**

Returns a new reference to the true object constant.

---

### `Nst_false`

**Synopsis:**

```better-c
Nst_Obj *Nst_false(void)
```

**Description:**

Returns the false object constant.

---

### `Nst_false_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_false_ref(void)
```

**Description:**

Returns a new reference to the false object constant.

---

### `Nst_null`

**Synopsis:**

```better-c
Nst_Obj *Nst_null(void)
```

**Description:**

Returns the null object constant.

---

### `Nst_null_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_null_ref(void)
```

**Description:**

Returns a new reference to the null object constant.

---

### `Nst_iend`

**Synopsis:**

```better-c
Nst_Obj *Nst_iend(void)
```

**Description:**

Returns the iend object constant.

---

### `Nst_iend_ref`

**Synopsis:**

```better-c
Nst_Obj *Nst_iend_ref(void)
```

**Description:**

Returns a new reference to the iend object constant.

---

### `Nst_type`

**Synopsis:**

```better-c
const Nst_TypeObjs *Nst_type(void)
```

**Description:**

Returns a [`Nst_TypeObjs`](c_api-global_consts.md#nst_typeobjs) struct
containing all the types.

---

### `Nst_str`

**Synopsis:**

```better-c
const Nst_StrConsts *Nst_str(void)
```

**Description:**

Returns a [`Nst_StrConsts`](c_api-global_consts.md#nst_strconsts) struct
containing all the string constants.

---

### `Nst_const`

**Synopsis:**

```better-c
const Nst_Consts *Nst_const(void)
```

**Description:**

Returns a [`Nst_Consts`](c_api-global_consts.md#nst_consts) struct containing
all the object constants.

---

### `Nst_iter_func`

**Synopsis:**

```better-c
const Nst_IterFunctions *Nst_iter_func(void)
```

**Description:**

Returns a [`Nst_IterFunctions`](c_api-global_consts.md#nst_iterfunctions) struct
containing all the function constants.

---

### `Nst_stdio`

**Synopsis:**

```better-c
Nst_StdStreams *Nst_stdio(void)
```

**Description:**

Returns a [`Nst_StdStreams`](c_api-global_consts.md#nst_stdstreams) struct
containing all the standard IO streams.
