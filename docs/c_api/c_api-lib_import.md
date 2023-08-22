# `lib_import.h`

C/C++ library utilities.

## Authors

TheSilvered

## Macros

### `Nst_MAKE_FUNCDECLR`

**Synopsis:**

```better-c
Nst_MAKE_FUNCDECLR(func_ptr, argc)
```

**Description:**

Initializes a function declaration.

For the name of the function the name of the function pointer is used.

**Parameters:**

- `func_ptr`: the function pointer to use
- `argc`: the number of arguments the function accepts

---

### `Nst_MAKE_NAMED_FUNCDECLR`

**Synopsis:**

```better-c
Nst_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name)
```

**Description:**

Initializes a function declaration with a custom name.

**Parameters:**

- `func_ptr`: the function pointer to use
- `argc`: the number of arguments the function accepts
- `func_name`: the name to use as a C string

---

### `Nst_MAKE_OBJDECLR`

**Synopsis:**

```better-c
Nst_MAKE_OBJDECLR(obj_ptr)
```

**Description:**

Initialized an object declaration.

For the name of the object the name of the pointer is used.

**Parameters:**

- `obj_pointer`: the pointer to the Nest object to declare

---

### `Nst_MAKE_NAMED_OBJDECLR`

**Synopsis:**

```better-c
Nst_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name)
```

**Description:**

Initialized an object declaration with a custom name.

**Parameters:**

- `obj_pointer`: the pointer to the Nest object to declare
- `obj_name`: the name to use as a C string

---

### `Nst_RETURN_TRUE`

**Description:**

Returns Nst_true().

---

### `Nst_RETURN_FALSE`

**Description:**

Returns Nst_false().

---

### `Nst_RETURN_NULL`

**Description:**

Returns Nst_null().

---

### `Nst_RETURN_ZERO`

**Description:**

Returns Nst_const()->Int_0.

---

### `Nst_RETURN_ONE`

**Description:**

Returns Nst_const()->Int_1.

---

### `Nst_RETURN_COND`

**Synopsis:**

```better-c
Nst_RETURN_COND(cond)
```

**Description:**

Returns Nst_true() if cond is true and Nst_false otherwise. cond is a C
condition.

---

### `Nst_FUNC_SIGN`

**Synopsis:**

```better-c
Nst_FUNC_SIGN(name)
```

**Description:**

Function signature for a Nest-callable C function.

---

### `Nst_DEF_EXTRACT`

**Synopsis:**

```better-c
Nst_DEF_EXTRACT(ltrl, ...)
```

**Description:**

Default call to Nst_extract_arg_values that returns nullptr on error.

---

### `Nst_DEF_VAL`

**Synopsis:**

```better-c
Nst_DEF_VAL(obj, val, def_val)
```

**Description:**

Results in def_val if obj is Nst_null() and in val otherwise.

---

### `Nst_T`

**Synopsis:**

```better-c
Nst_T(obj, type_name)
```

**Description:**

Checks if the type of an object is type_name.

---

## Structs

### `Nst_ObjDeclr`

**Synopsis:**

```better-c
typedef struct _Nst_ObjDeclr {
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
} Nst_ObjDeclr
```

**Description:**

Structure defining an object declaration.

**Fields:**

- `ptr`: the pointer to the object or function
- `arg_num`: the number of arguments if the object is a function, -1 for other
  declarations
- `name`: the name of the declared object

---

### `Nst_DeclrList`

**Synopsis:**

```better-c
typedef struct _Nst_DeclrList {
    Nst_ObjDeclr *objs;
    usize obj_count;
} Nst_DeclrList
```

**Description:**

Structure defining a list of object declarations.

**Fields:**

- `objs`: the array of declared objects
- `obj_count`: the number of objects inside the array

---

## Functions

### `Nst_extract_arg_values`

**Synopsis:**

```better-c
bool Nst_extract_arg_values(const i8 *types, usize arg_num, Nst_Obj **args,
                            ...)
```

**Description:**

Checks the types of the arguments and extracts their values.

If you want to check but not get the value of an argument, the pointer in the
variable arguments can be NULL. Check the syntax for the types argument in
lib_import.h

**Parameters:**

- `types`: the string that defines the expected types of the arguments
- `arg_num`: the number of arguments passed
- `args`: the arguments to check
- `...`: the pointers to the variables where the values extracted are stored and
  to the custom types

