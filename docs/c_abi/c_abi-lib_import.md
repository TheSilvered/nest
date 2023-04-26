# `lib_import.h`

This header contains

## Macros

### `NST_MAKE_FUNCDECLR`

**Synopsis**:

```better-c
NST_MAKE_FUNCDECLR(func_ptr, argc)
```

**Description**:

Initializes a [`Nst_ObjDeclr`](#nst_objdeclr) using `func_ptr` as the name.

**Arguments**:

- `func_ptr`: the function to store
- `argc`: the number of arguments taken by the function

---

### `NST_MAKE_NAMED_FUNCDECLR`

**Synopsis**:

```better-c
NST_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name)
```

**Description**:

Initializes a [`Nst_ObjDeclr`](#nst_objdeclr) using a custom name.

**Arguments**:

- `func_ptr`: the function to store
- `argc`: the number of arguments taken by the function
- `func_name`: the name of the function as a NUL-terminated string

---

### `NST_MAKE_OBJDECLR`

**Synopsis**:

```better-c
NST_MAKE_OBJDECLR(obj_ptr)
```

**Description**:

Initializes a [`Nst_ObjDeclr`] storing `obj_ptr` and using its name.

**Arguments**:

- `obj_ptr`: the object to store

---

### `NST_MAKE_NAMED_OBJDECLR`

**Synopsis**:

```better-c
NST_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name)
```

**Description**:

Initializes a [`Nst_ObjDeclr`] storing `obj_ptr` and using a custom name.

**Arguments**:

- `obj_ptr`: the object to store
- `obj_name`: the name as a NUL-terminated string

---

### `NST_SET_ERROR`

**Synopsis**:

```better-c
NST_SET_ERROR(err_name, err_msg)
```

**Description**:

Sets the name and message of the error increasing the reference count of the
name but not of the message.

**Arguments**:

- `err_name`: the name of the error
- `err_msg`: the message of the error

---

### `NST_SET_[error_name]_ERROR` macros

**Synopsis**:

```better-c
NST_SET_[error_name]_ERROR(msg)
```

**Description**:

Availalable macros:

```better-c
NST_SET_SYNTAX_ERROR
NST_SET_MEMORY_ERROR
NST_SET_TYPE_ERROR
NST_SET_VALUE_ERROR
NST_SET_MATH_ERROR
NST_SET_CALL_ERROR
NST_SET_IMPORT_ERROR
```

Sets an error with the name as `[error_name]` and the message as `msg`

**Arguments**:

- `msg`: the message of the error

---

### `NST_SET_RAW_ERROR`

**Synopsis**:

```better-c
NST_SET_RAW_ERROR(err_name, err_msg)
```

**Description**:

Sets the name and message of the error increasing the reference count of the
name and creating a new Nest string from a NUL-terminated one.

**Arguments**:

- `err_name`: the name of the error
- `err_msg`: a NUL-terminated string for the message

---

### `NST_SET_RAW_[error_name]_ERROR` macros

**Synopsis**:

```better-c
NST_SET_RAW_[error_name]_ERROR(msg)
```

**Description**:

Availalable macros:

```better-c
NST_SET_RAW_SYNTAX_ERROR
NST_SET_RAW_MEMORY_ERROR
NST_SET_RAW_TYPE_ERROR
NST_SET_RAW_VALUE_ERROR
NST_SET_RAW_MATH_ERROR
NST_SET_RAW_CALL_ERROR
NST_SET_RAW_IMPORT_ERROR
```

Sets an error with the name as `[error_name]` and the message as `msg`

**Arguments**:

- `msg`: a NUL-terminated string for the message

---

### `NST_FAILED_ALLOCATION`

**Description**:

Sets the error name to `Memory Error` and the message with failed allocation.

---

### `NST_RETURN_[obj]`

**Synopsis**:

```better-c
NST_RETURN_TRUE
NST_RETURN_FALSE
NST_RETURN_NULL
NST_RETURN_ZERO
NST_RETURN_ONE
```

**Description**:

Returns `[obj]`, for `NST_RETURN_ZERO` and `NST_RETURN_ONE` the type is `Int`.

---

### `NST_RETURN_COND`

**Synopsis**:

```better-c
NST_RETURN_COND(cond)
```

**Description**:

Returns either `nst_true()` or `nst_false()` depending on `cond`.

---

### `NST_FUNC_SIGN`

**Synopsis**:

```better-c
NST_FUNC_SIGN(name)
```

**Description**:

Expands into the function signature of a function that can be called through
Nest.

**Arguments**:

- `name`: the name of the function

**Example**:

```better-c
NST_FUNC_SIGN(my_func)

// Expands into

Nst_Obj *name(usize arg_num, Nst_Obj **args, Nst_OpErr *err)
```

- `arg_num`: is the number of arguments of the function
- `args`: the arguments passed
- `err`: the error

---

### `NST_DEF_EXTRACT`

**Synopsis**:

```better-c
NST_DEF_EXTRACT(ltrl, ...)
```

**Description**:

Default call to [`nst_extract_arg_values`](#nst_extract_arg_values), checking
the return value and exiting in case of failure.

**Arguments**:

- `ltrl`: the `types` parameter for `nst_extract_arg_values`

---

### `NST_DEF_VAL`

**Synopsis**:

```better-c
NST_DEF_VAL(obj, val, def_val)
```

**Description**:

Evaluates to `def_val` if `obj` is `nst_null()` and to `def_val` otherwise.

**Arguments**:

- `obj`: the object that may be `null`
- `val`: the value used if the object is not `null`
- `def_val`: the value used if the object is `null`

---

### `NST_ERROR_OCCURRED`

**Description**:

Evaluates to `true` if an error has occurred and to `false` otherwise.

---

### `NST_RETURN_NEW_STR`

**Synopsis**:

```better-c
NST_RETURN_NEW_STR(val, len)
```

**Description**:

Creates a new `Nst_StrObj` with the length of `len` and the contents of `val`.
If an error occurs `val` is freed.

**Arguments**:

- `val`: the content of the string to create
- `len`: the length of the string

---

## Structs

### `Nst_ObjDeclr`

**Synopsis**:

```better-c
typedef struct _Nst_ObjDeclr
{
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
}
Nst_ObjDeclr;
```

**Description**:

A structure that defines an object exported by a C library.

**Fields**:

- `ptr`: a pointer to the exported object
- `arg_num`: the number of arguments taken by the object if it is a function, if
  it is not this field is set to `-1`
- `name`: the name of the object

---

### `Nst_DeclrList`

**Synopsis**:

```better-c
typedef struct _Nst_DeclrList
{
    Nst_ObjDeclr *objs;
    usize obj_count;
}
Nst_DeclrList;
```

**Description**:

A structure used to hold all the objects of a C library.

**Fields**:

- `objs`: the objects of the library
- `obj_count`: the number of objects exported

---

## Functions

### `nst_extract_arg_values`

**Synopsis**:

```better-c
bool nst_extract_arg_values(const i8  *types,
                            usize      arg_num,
                            Nst_Obj  **args,
                            Nst_OpErr *err,
                            ...)
```

**Description**:

Extracts the values from the arguments performing type checking.

**Arguments**:

- `types`: a string of letters for the types, check the
  [full format](#types-format)
- `arg_num`: the number of arguments passed to the function
- `args`: the arguments
- `err`: the error

**Return value**:

The function returns `true` on success and `false` on failure.

---

## Types format

### Object letters

**Builtin types**

- `t`: `Type`
- `i`: `Int`
- `r`: `Real`
- `b`: `Bool`
- `n`: `Null`
- `s`: `Str`
- `v`: `Vector`
- `a`: `Array`
- `m`: `Map`
- `f`: `Func`
- `I`: `Iter`
- `B`: `Byte`
- `F`: `IOFile`

**Shorthands**

- `l`: `i|B_i`
- `N`: `i|r|B_r`
- `A`: `a|v`
- `S`: `a|v|s:a`
- `R`: `I|a|v|s:I`

**Other**

- `o`: any object
- `#`: a custom object

### Features

**Custom types**

You can have up to 3 custom types in a single argument and you should label
them with `#`. These types should be passed to the function before the argument
itself.

```better-c
// example of a custom type from libs/nest_co/nest_co.cpp
NST_DEF_EXTRACT("#?A", t_Coroutine, &co, &co_args)

// if we were to swap ?A and # t_Coroutine would follow &co_args
NST_DEF_EXTRACT("?A#", &co_args, t_Coroutine, &co)
```

**Optional types**

To have an optional type you can use `?` before the type itself. Using `|n` is
the same thing.

**Multiple types per argument**

To have multiple possible types an argument can be you can use a pipe (`|`)
between them.

**Automatic type casting**

After the type specified you can add `:` or `_` followed by exactly one letter.
`:` is a cast between Nest objects, `_` is a cast to a C type. When using the
latter there cannot be any optional or custom types and it is restricted to
only these types after the underscore: `i`, `r`, `b`, `B` or `s`.

**Implicit casting**

If a type is specified as only one of `i`, `r`, `b` or `B` it automatically
becomes `i_i`, `r_r`, `b_b` or `B_B` if it is not used to check the contents of
a sequence. To get the object itself use `i:i`, `r:r`, `b:b`, `B:B` and then
immediately decrease the reference (it is safe in this case since no new objects
are created).

**Sequence type checking**

You can additionally check the types present inside the matched sequence
By following the type with a dot

### Examples

`i|r|B_B?A.#|i`: An `Int`, `Real` or `Byte` casted to a `Nst_Byte` followed by
an optional `Array` or `Vector` that, if it exists, should contain only objects
of a custom type or integers.
