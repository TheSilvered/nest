/**
 * @file lib_import.h
 *
 * @brief C/C++ library utilities
 *
 * @author TheSilvered
 */

/* [docs:link lib_import.h <c_api-lib_import.md#usage-of-the-types-argument>] */

/* [docs:raw]
## Usage of the `types` argument

### Builtin types

- `t`: `Type`
- `i`: `Integer`
- `r`: `Real`
- `b`: `Bool`
- `n`: `Null`
- `s`: `String`
- `v`: `Vector`
- `a`: `Array`
- `m`: `Map`
- `f`: `Func`
- `I`: `Iter`
- `B`: `Bite`
- `F`: `IOFile`

### Other

  `o`: any object

### Whitespace

Whitespace inside the type string is ignored.

```better-c
// All of these are the same
"isv"
"i s v"
" i s v "
" is v"
```

### Union types

To specify more than one type that an argument can be, use a pipe (`|`) between
the various types.

```better-c
Nst_Obj *var;
Nst_DEF_EXTRACT("i|s", &var); // 'var' accepts only Int or Str objects
```

### Custom types

You can have any number of custom types in a single argument and you should
label them with `#`. The custom type(s) are to be passed to the function
before the pointer to the variable.

```better-c
Nst_Obj *var;
Nst_DEF_EXTRACT("#|#|#", custom_type1, custom_type2, custom_type3, &var);
```

### Optional types

To have an optional type you can use `?` _before_ the type itself. Optional
types allow for either the specified types or `Null` to be accepted. Because of
this writing `?i` and `i|n` is the same thing.

```better-c
Nst_IOFileObj *file;
Nst_DEF_EXTRACT("?F", &file); // 'file' can be either a File or NULL object
```

### Casting to other `Nst_Obj`

By following the selected type(s) with a colon (`:`) you can specify with one
letter the type that the object should be casted to after it has been checked.
When casting to an object a new reference is always put inside the given
variable that needs to be removed with `Nst_dec_ref` when no longer in use.

The type to cast to be specified by any of the builtin types except `n` in
addition to `o`. Using `o` will only increase the reference of the argument and
casting between an Array and a Vector object will not modify the object (apart
from increasing the reference) since they use the same structure in memory.

```better-c
// 'num' accepts a Int and Byte objects but it will always contain an Int
Nst_IntObj *num;
Nst_DEF_EXTRACT("i|B:i", &num);
```

### Casting to C types

If you follow the selected type(s) with an underscore (`_`), you can extract
the value of the argument into a C value. This method only accepts `i`, `r`,
`b` and `B` and cannot be used along with `:`.

```better-c
// 'opt' only accepts Int objects but will always contain a boolean
bool opt;
Nst_DEF_EXTRACT("i_b", &opt);
```

### Implicit casting to C types

If a type is specified as only one of `i`, `r`, `b` or `B` (and not a union
of types) it is automatically translated to `i_i`, `r_r`, `b_b` and `B_B`
respectively.

The values are extracted to the following C types:

- `i` -> `i64`
- `r` -> `f64`
- `b` -> `bool`
- `B` -> `u8`

```better-c
// even though only 'i' and 'r' are specified, the extracted values are C types
i64 int_num;
f64 real_num;
Nst_DEF_EXTRACT("i r", &int_num, &real_num);
```

If you instead want the object itself you can write `i|i`, `r|r`, `b|b` or
`B|B` since the type is in a union.

### Sequence type checking

You can additionally check the types present inside a matched sequence by
following the type with a dot (`.`). If the type to check is not an `Array` or
`Vector` object no checking is done. Note that this kind of checking cannot
occur if the argument is casted to a C type.

```better-c
// 'array_of_ints' can be either an Array or Null object. If it is the
// former its elements can only be Int and Byte objects.
Nst_SeqObj *array_of_ints;
Nst_DEF_EXTRACT("?a.i|B" &array_of_ints);
```

### Shorthands

There are some shorthands that reduce the complexity of the type string by
representing commonly used types into a single character.

- `l` expands into `i|B_i`
- `N` expands into `i|r|B_r`
- `A` expands into `a|v`
- `S` expands into `a|v|s:a`
- `R` expands into `I|a|v|s:I`
- `y` expands into `o_b`

The shorthands that contain a cast (either `:` or `_`) will not cast the object
the shorthand is used for checking the contents of a sequence or are part of a
union. In case a cast is added after the type manually, it is overwritten.

```better-c
"S"   // matches Array, Vector and Str and casts the object to Array
"I|S" // matches Iter, Array, Vector and Str and casts the object to Array
"l:r" // matches Int and Byte and casts the object to Real
"a.S" // matches Array that contains Array, Vector or Str, no casting occurs
```

### Examples

```better-c
// An Int, Real or Byte all casted to a u8 followed by an optional
// Array or Vector that, if it exists, should contain only objects of a custom
// type or integers.
"N_B ?A.#|i"

// A Str, Array or Vector that casted to an Array should contain only strings.
// This can be written in an expanded form as "a|v|s:a.s"
"S.s"
```
*/

#ifndef LIB_IMPORT_H
#define LIB_IMPORT_H

#include "interpreter.h"
#include "global_consts.h"

/**
 * Initializes a function declaration.
 *
 * @brief For the name of the function the name of the function pointer is
 * used.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 */
#define Nst_MAKE_FUNCDECLR(func_ptr, argc)                                    \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(#func_ptr, false))                           \
    }

/**
 * Initializes a function declaration with a custom name.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 * @param func_name: the name to use as a C string
 */
#define Nst_MAKE_NAMED_FUNCDECLR(func_ptr, argc, func_name)                   \
    {                                                                         \
        (void *)(func_ptr),                                                   \
        argc,                                                                 \
        STR(Nst_string_new_c_raw(func_name, false))                           \
    }

/**
 * Initialized an object declaration.
 *
 * @brief For the name of the object the name of the pointer is used.
 *
 * @param obj_pointer: the pointer to the Nest object to declare
 */
#define Nst_MAKE_OBJDECLR(obj_ptr)                                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(#obj_ptr, false))                            \
    }

/**
 * Initialized an object declaration with a custom name.
 *
 * @param obj_pointer: the pointer to the Nest object to declare
 * @param obj_name: the name to use as a C string
 */
#define Nst_MAKE_NAMED_OBJDECLR(obj_ptr, obj_name)                            \
    {                                                                         \
        (void *)(obj_ptr),                                                    \
        -1,                                                                   \
        STR(Nst_string_new_c_raw(obj_name, false))                            \
    }

/* [docs:link Nst_const()->Int_0 Nst_const] */
/* [docs:link Nst_const()->Int_1 Nst_const] */

/* Returns `Nst_true()`. */
#define Nst_RETURN_TRUE return Nst_true_ref()
/* Returns `Nst_false()`. */
#define Nst_RETURN_FALSE return Nst_false_ref()
/* Returns `Nst_null()`. */
#define Nst_RETURN_NULL return Nst_null_ref()
/* Returns `Nst_const()->Int_0`. */
#define Nst_RETURN_ZERO return Nst_inc_ref(Nst_const()->Int_0)
/* Returns `Nst_const()->Int_1`. */
#define Nst_RETURN_ONE return Nst_inc_ref(Nst_const()->Int_1)
/**
 * @brief Returns `Nst_true()` if `cond` is `true` and `Nst_false()` otherwise.
 * `cond` is a C condition.
 */
#define Nst_RETURN_COND(cond)                                                 \
    return (cond) ? Nst_true_ref() : Nst_false_ref()

/* Function signature for a Nest-callable C function. */
#define Nst_FUNC_SIGN(name)                                                   \
    Nst_Obj *NstC name(usize arg_num, Nst_Obj **args)

/* Default call to `Nst_extract_arg_values` that returns `NULL` on error. */
#define Nst_DEF_EXTRACT(ltrl, ...) do {                                       \
    if (!Nst_extract_arg_values(ltrl, arg_num, args, __VA_ARGS__))            \
        return NULL;                                                          \
    } while (0)

/* Results in `def_val` if obj is `Nst_null()` and in `val` otherwise. */
#define Nst_DEF_VAL(obj, val, def_val)                                        \
    (OBJ(obj) == Nst_null() ? (def_val) : (val))

/* Checks if the type of an object is `type_name`. */
#define Nst_T(obj, type_name) ((obj)->type == Nst_type()->type_name)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/**
 * Structure defining an object declaration.
 *
 * @param ptr: the pointer to the object or function
 * @param arg_num: the number of arguments if the object is a function, `-1`
 * for other declarations
 * @param name: the name of the declared object
 */
NstEXP typedef struct _Nst_ObjDeclr {
    void *ptr;
    isize arg_num;
    Nst_StrObj *name;
} Nst_ObjDeclr;

/**
 * Structure defining a list of object declarations.
 *
 * @param objs: the array of declared objects
 * @param obj_count: the number of objects inside the array
 */
NstEXP typedef struct _Nst_DeclrList {
    Nst_ObjDeclr *objs;
    usize obj_count;
} Nst_DeclrList;

/**
 * Checks the types of the arguments and extracts their values.
 *
 * @brief Check the syntax for the types argument in `lib_import.h`.
 *
 * @param types: the string that defines the expected types of the arguments
 * @param arg_num: the number of arguments passed
 * @param args: the arguments to check
 * @param ...: the pointers to the variables where the values extracted are
 * stored and to the custom types, if the pointer to a variable is `NULL` the
 * argument is checked but no value is extracted
 *
 * @return `true` on success and `false` on failure. The error is set.
 */
NstEXP bool NstC Nst_extract_arg_values(const i8 *types, usize arg_num,
                                        Nst_Obj **args, ...);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
