# `global_consts.h`

This header contains the functions used to obtain global variables and constants.

## Structs

### `Nst_StrConsts`

**Synopsis**:

```better-c
typedef struct _Nst_StrConsts
{
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
}
Nst_StrConsts
```

**Description**:

This is a struct containing the strings that remain constant throughout the
lifetime of the program.

---

### `Nst_TypeObjs`

**Synopsis**:

```better-c
typedef struct _Nst_TypeObjs
{
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
}
Nst_TypeObjs
```

**Description**:

This structure contains the built-in types.

---

### `Nst_Consts`

**Synopsis**:

```better-c
typedef struct _Nst_Consts
{
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
}
Nst_Consts
```

**Description**:

This structure contains other constants of the language or that are often used.

---

### `Nst_StdStreams`

**Synopsis**:

```better-c
typedef struct _Nst_StdStreams
{
    Nst_IOFileObj *in;
    Nst_IOFileObj *out;
    Nst_IOFileObj *err;
}
Nst_StdStreams
```

**Description**:

This structure contains the files used as the standard streams by Nest. These
are not constant.

---

## Functions

### `_nst_init_objects`

**Synopsis**:

```better-c
bool _nst_init_objects(void)
```

**Description**:

This function initializes the objects inside `Nst_StrConsts`, `Nst_TypeObjs`,
`Nst_Consts` and `Nst_StdStreams`. It should never be called.

**Return value**:

The function returns `true` if all the fields were initialized correctly or
`false` if something failed.

---

### `_nst_del_objects`

**Synopsis**:

```better-c
void _nst_del_objects(void)
```

**Description**:

This function deletes the objects inside `Nst_StrConsts`, `Nst_TypeObjs`,
`Nst_Consts` and `Nst_StdStreams`. It should never be called.

---

### `nst_true`

**Synopsis**:

```better-c
Nst_Obj *nst_true(void)
```

**Return value**:

This function returns `Nst_Consts.Bool_true`.

---

### `nst_false`

**Synopsis**:

```better-c
Nst_Obj *nst_false(void)
```

**Return value**:

This function returns `Nst_Consts.Bool_false`.

---

### `nst_null`

**Synopsis**:

```better-c
Nst_Obj *nst_null(void)
```

**Return value**:

This function returns `Nst_Consts.Bool_null`.

---

### `nst_type`

**Synopsis**:

```better-c
const Nst_TypeObjs *nst_type(void)
```

**Return value**:

This function returns the global `Nst_TypeObjs` structure.

---

### `nst_str`

**Synopsis**:

```better-c
const Nst_StrConsts *nst_str(void)
```

**Return value**:

This function returns the global `Nst_StrConsts` structure.

---

### `nst_const`

**Synopsis**:

```better-c
const Nst_Consts *nst_const(void)
```

**Return value**:

This function returns the global `Nst_Consts` structure.

---

### `nst_stdio`

**Synopsis**:

```better-c
Nst_StdStreams *nst_stdio(void)
```

**Return value**:

This function returns the global `Nst_StdStreams` structure.
