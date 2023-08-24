# `global_consts.h`

Constant objects used in the program.

## Authors

TheSilvered

## Structs

### `Nst_StrConsts`

**Synopsis:**

```better-c
typedef struct _Nst_StrConsts {
    // Type strings

    Nst_StrObj *t_Type;
    Nst_StrObj *t_Int;
    Nst_StrObj *t_Real;
    Nst_StrObj *t_Bool;
    Nst_StrObj *t_Null;
    Nst_StrObj *t_Str;
    Nst_StrObj *t_Array;
    Nst_StrObj *t_Vector;
    Nst_StrObj *t_Map;
    Nst_StrObj *t_Func;
    Nst_StrObj *t_Iter;
    Nst_StrObj *t_Byte;
    Nst_StrObj *t_IOFile;

    // Constant strings

    Nst_StrObj *c_true;
    Nst_StrObj *c_false;
    Nst_StrObj *c_null;

    // Standard error names

    Nst_StrObj *e_SyntaxError;
    Nst_StrObj *e_MemoryError;
    Nst_StrObj *e_ValueError;
    Nst_StrObj *e_TypeError;
    Nst_StrObj *e_CallError;
    Nst_StrObj *e_MathError;
    Nst_StrObj *e_ImportError;

    // Other

    Nst_StrObj *o__vars_;
    Nst_StrObj *o__globals_;
    Nst_StrObj *o__args_;
    Nst_StrObj *o__cwd_;
    Nst_StrObj *o_failed_alloc;
} Nst_StrConsts
```

**Description:**

String constants.

Fields starting with `t_` are the name of types, the ones starting with `c_` are
the name of constants, the one starting with `e_` are the name of errors and the
one starting with `o_` are others.

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
    Nst_Obj *Int_0;
    Nst_Obj *Int_1;
    Nst_Obj *Int_neg1;
    Nst_Obj *Real_0;
    Nst_Obj *Real_1;
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
    Nst_IOFileObj *in;
    Nst_IOFileObj *out;
    Nst_IOFileObj *err;
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
    Nst_FuncObj *range_start;
    Nst_FuncObj *range_is_done;
    Nst_FuncObj *range_get_val;

    Nst_FuncObj *seq_start;
    Nst_FuncObj *seq_is_done;
    Nst_FuncObj *seq_get_val;

    Nst_FuncObj *str_start;
    Nst_FuncObj *str_is_done;
    Nst_FuncObj *str_get_val;

    Nst_FuncObj *map_start;
    Nst_FuncObj *map_is_done;
    Nst_FuncObj *map_get_val;
} Nst_IterFunctions
```

**Description:**

The functions of the built-in iterators.

---

## Functions

### `_Nst_init_objects`

**Synopsis:**

```better-c
bool _Nst_init_objects(void)
```

**Description:**

Initializes all the global constants and IO streams.

**Returns:**

`true` on success and `false` on failure. No error is set.

---

### `_Nst_del_objects`

**Synopsis:**

```better-c
void _Nst_del_objects(void)
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

### `Nst_false`

**Synopsis:**

```better-c
Nst_Obj *Nst_false(void)
```

**Description:**

Returns the false object constant.

---

### `Nst_null`

**Synopsis:**

```better-c
Nst_Obj *Nst_null(void)
```

**Description:**

Returns the null object constant.

---

### `Nst_type`

**Synopsis:**

```better-c
const Nst_TypeObjs *Nst_type(void)
```

**Description:**

Returns a [`Nst_TypeObjs`](c_api-global_consts.md/#nst_typeobjs) struct
containing all the types.

---

### `Nst_str`

**Synopsis:**

```better-c
const Nst_StrConsts *Nst_str(void)
```

**Description:**

Returns a [`Nst_StrConsts`](c_api-global_consts.md/#nst_strconsts) struct
containing all the string constants.

---

### `Nst_const`

**Synopsis:**

```better-c
const Nst_Consts *Nst_const(void)
```

**Description:**

Returns a [`Nst_Consts`](c_api-global_consts.md/#nst_consts) struct containing
all the object constants.

---

### `Nst_iter_func`

**Synopsis:**

```better-c
const Nst_IterFunctions *Nst_iter_func(void)
```

**Description:**

Returns a [`Nst_IterFunctions`](c_api-global_consts.md/#nst_iterfunctions)
struct containing all the function constants.

---

### `Nst_stdio`

**Synopsis:**

```better-c
Nst_StdStreams *Nst_stdio(void)
```

**Description:**

Returns a [`Nst_StdStreams`](c_api-global_consts.md/#nst_stdstreams) struct
containing all the standard IO streams.

