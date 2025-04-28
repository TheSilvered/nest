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

    Nst_ObjRef *t_Type;
    Nst_ObjRef *t_Int;
    Nst_ObjRef *t_Real;
    Nst_ObjRef *t_Bool;
    Nst_ObjRef *t_Null;
    Nst_ObjRef *t_Str;
    Nst_ObjRef *t_Array;
    Nst_ObjRef *t_Vector;
    Nst_ObjRef *t_Map;
    Nst_ObjRef *t_Func;
    Nst_ObjRef *t_Iter;
    Nst_ObjRef *t_Byte;
    Nst_ObjRef *t_IOFile;
    Nst_ObjRef *t_IEnd;

    // Constant strings

    Nst_ObjRef *c_true;
    Nst_ObjRef *c_false;
    Nst_ObjRef *c_null;
    Nst_ObjRef *c_inf;
    Nst_ObjRef *c_nan;
    Nst_ObjRef *c_neginf;
    Nst_ObjRef *c_negnan;

    // Standard error names

    Nst_ObjRef *e_SyntaxError;
    Nst_ObjRef *e_MemoryError;
    Nst_ObjRef *e_ValueError;
    Nst_ObjRef *e_TypeError;
    Nst_ObjRef *e_CallError;
    Nst_ObjRef *e_MathError;
    Nst_ObjRef *e_ImportError;
    Nst_ObjRef *e_Interrupt;

    // Other

    Nst_ObjRef *o__vars_;
    Nst_ObjRef *o__globals_;
    Nst_ObjRef *o__args_;
    Nst_ObjRef *o_failed_alloc;
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
    Nst_ObjRef *Type;
    Nst_ObjRef *Int;
    Nst_ObjRef *Real;
    Nst_ObjRef *Bool;
    Nst_ObjRef *Null;
    Nst_ObjRef *Str;
    Nst_ObjRef *Array;
    Nst_ObjRef *Vector;
    Nst_ObjRef *Map;
    Nst_ObjRef *Func;
    Nst_ObjRef *Iter;
    Nst_ObjRef *Byte;
    Nst_ObjRef *IOFile;
    Nst_ObjRef *IEnd;
} Nst_TypeObjs
```

**Description:**

Type constants.

---

### `Nst_Consts`

**Synopsis:**

```better-c
typedef struct _Nst_Consts {
    Nst_ObjRef *Bool_true;
    Nst_ObjRef *Bool_false;
    Nst_ObjRef *Null_null;
    Nst_ObjRef *IEnd_iend;
    Nst_ObjRef *Int_0;
    Nst_ObjRef *Int_1;
    Nst_ObjRef *Int_neg1;
    Nst_ObjRef *Real_0;
    Nst_ObjRef *Real_1;
    Nst_ObjRef *Real_nan;
    Nst_ObjRef *Real_negnan;
    Nst_ObjRef *Real_inf;
    Nst_ObjRef *Real_neginf;
    Nst_ObjRef *Byte_0;
    Nst_ObjRef *Byte_1;
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
    Nst_ObjRef *in;
    Nst_ObjRef *out;
    Nst_ObjRef *err;
} Nst_StdStreams
```

**Description:**

Standard IO streams.

!!!note
    These are not constant and can change at run-time.

---

## Functions

### `_Nst_globals_init`

**Synopsis:**

```better-c
bool _Nst_globals_init(void)
```

**Description:**

Initialize all the global constants and IO streams.

**Returns:**

`true` on success and `false` on failure. No error is set.

---

### `_Nst_globals_quit`

**Synopsis:**

```better-c
void _Nst_globals_quit(void)
```

**Description:**

Delete all the global constants and IO streams.

---

### `Nst_true`

**Synopsis:**

```better-c
Nst_Obj *Nst_true(void)
```

**Returns:**

The `true` object constant.

---

### `Nst_true_ref`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_true_ref(void)
```

**Returns:**

A new reference to the `true` object constant.

---

### `Nst_false`

**Synopsis:**

```better-c
Nst_Obj *Nst_false(void)
```

**Returns:**

The `false` object constant.

---

### `Nst_false_ref`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_false_ref(void)
```

**Returns:**

A new reference to the `false` object constant.

---

### `Nst_null`

**Synopsis:**

```better-c
Nst_Obj *Nst_null(void)
```

**Returns:**

The `null` object constant.

---

### `Nst_null_ref`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_null_ref(void)
```

**Returns:**

A new reference to the `null` object constant.

---

### `Nst_iend`

**Synopsis:**

```better-c
Nst_Obj *Nst_iend(void)
```

**Returns:**

The `iend` object constant.

---

### `Nst_iend_ref`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_iend_ref(void)
```

**Returns:**

A new reference to the `iend` object constant.

---

### `Nst_type`

**Synopsis:**

```better-c
const Nst_TypeObjs *Nst_type(void)
```

**Returns:**

A [`Nst_TypeObjs`](c_api-global_consts.md#nst_typeobjs) struct containing all
the types.

---

### `Nst_str`

**Synopsis:**

```better-c
const Nst_StrConsts *Nst_str(void)
```

**Returns:**

A [`Nst_StrConsts`](c_api-global_consts.md#nst_strconsts) struct containing all
the string constants.

---

### `Nst_const`

**Synopsis:**

```better-c
const Nst_Consts *Nst_const(void)
```

**Returns:**

A [`Nst_Consts`](c_api-global_consts.md#nst_consts) struct containing all the
object constants.

---

### `Nst_stdio`

**Synopsis:**

```better-c
Nst_StdStreams *Nst_stdio(void)
```

**Returns:**

A [`Nst_StdStreams`](c_api-global_consts.md#nst_stdstreams) struct containing
all the standard IO streams.
