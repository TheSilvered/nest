/**
 * @file lib_import.h
 *
 * @brief Library import manager & C/C++ library utilities
 *
 * @author TheSilvered
 */

/* [docs:link lib_import.h <c_api-lib_import.md#usage-of-the-types-argument>] */

/* [docs:raw]
## Usage of the `types` argument

### Builtin types

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

### Casting to other `Nst_Obj`s

By following the selected type(s) with a colon (`:`) you can specify with one
letter the type that the object should be casted to after it has been checked.
When casting to an object a new reference is always put inside the given
variable that needs to be removed with `Nst_dec_ref` when no longer in use.

The type to cast to can be specified by any of the builtin types except `n`.
In addition you can use `o` just to increase the reference of the argument.
Casting between an Array and a Vector objects will only increase their
reference count and will not actually create a copy even if the types are
different.

```better-c
// 'num' accepts a Int and Byte objects but it will always contain an Int
Nst_IntObj *num;
Nst_DEF_EXTRACT("i|B:i", &num);
```

### Casting to C types

If you follow the selected type(s) with an underscore (`_`), you can extract
the value of the argument into a C value. This method only accepts `i`, `r`,
`b` and `B` and cannot be used along with `:`.

The values are extracted to the following C types:

- `i` -> `i64`
- `r` -> `f64`
- `b` -> `bool`
- `B` -> `u8`

```better-c
// 'opt' only accepts Int objects but will always contain a boolean
bool opt;
Nst_DEF_EXTRACT("i_b", &opt);
```

### Implicit casting to C types

If a type is specified as only one of `i`, `r`, `b` or `B` (and not a union
of types) it is automatically translated to `i_i`, `r_r`, `b_b` and `B_B`
respectively.

```better-c
// even though only 'i' and 'r' are specified, the extracted values are C types
i64 int_num;
f64 real_num;
Nst_DEF_EXTRACT("i r", &int_num, &real_num);
```

If you instead want the object itself you can write `i|i`, `r|r`, `b|b` or
`B|B`. Since the type is in a union the casting will not occur.

### Sequence type checking

You can additionally check the types present inside a matched sequence by
following the type with a dot (`.`). If the type to check is not an `Array` or
`Vector` object no checking is done. Note that this kind of checking cannot
occur if the argument is casted to a C type.

```better-c
// 'array_of_ints' can be either an Array or Null object. If it is the
// former its elements can only be Int or Byte objects.
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
when used to check the contents of a sequence or when they are part of a union.
Additionally, any cast added manually will overwrite the cast of the shorthand.

```better-c
"S"   // matches Array, Vector and Str and casts the object to Array
"I|S" // matches Iter, Array, Vector and Str, no casting occurs
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
 * Initialize a function declaration.
 *
 * @brief For the name of the function the name of the function pointer is
 * used.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 */
#define Nst_FUNCDECLR(func_ptr, argc) { (void *)(func_ptr), argc, #func_ptr }

/**
 * Initialize a function declaration with a custom name.
 *
 * @param func_ptr: the function pointer to use
 * @param argc: the number of arguments the function accepts
 * @param name: the name to use as a C string
 */
#define Nst_NAMED_FUNCDECLR(func_ptr, argc, name)                             \
    { (void *)(func_ptr), argc, name }

/**
 * Initialize an object declaration.
 *
 * @brief For the name of the object the name of the function pointer is used.
 *
 * @param func_ptr: the pointer to a function that returns the value of the
 * constant, this function is of signature `Nst_ConstFunc`
 */
#define Nst_CONSTDECLR(func_ptr) { (void *)(func_ptr), -1, #func_ptr }

/**
 * Initialize an object declaration with a custom name.
 *
 * @param func_ptr: the pointer to a function that returns the value of the
 * constant, this function is of signature `Nst_ConstFunc`
 * @param name: the name to use as a C string
 */
#define Nst_NAMED_CONSTDECLR(func_ptr, name) { (void *)(func_ptr), -1, name }

/* End the declarations array. */
#define Nst_DECLR_END { NULL, 0, NULL }

/**
 * @return `Nst_true_ref()` if `expr` is `true` and `Nst_false_ref()`
 * otherwise. `expr` is a C boolean expression.
 */
#define Nst_RETURN_BOOL(expr)                                                 \
    return (expr) ? Nst_true_ref() : Nst_false_ref()

/* Results in `def_val` if obj is `Nst_null()` and in `val` otherwise. */
#define Nst_DEF_VAL(obj, val, def_val)                                        \
    (NstOBJ(obj) == Nst_null() ? (def_val) : (val))

/* Check if the type of an object is `type_name`. */
#define Nst_T(obj, type_name) ((obj)->type == Nst_type()->type_name)

/**
 * Create an object with custom data. This is a wrapper for `_Nst_obj_custom`.
 *
 * @brief The `size` and `name` parameters are derived from `type`.
 *
 * @param type: the type of the data to insert
 * @param data: the data to copy
 *
 * @return The new object or `NULL` on error.
 */
#define Nst_obj_custom(type, data)                                            \
    _Nst_obj_custom(sizeof(type), (void *)(data), #type)

/**
 * Create an object with custom data. This is a wrapper for
 * `_Nst_obj_custom_ex`.
 *
 * @brief The `size` and `name` parameters are derived from `type`.
 *
 * @param type: the type of the data to insert
 * @param data: the data to copy
 * @param dstr: the destructor to use for the type
 *
 * @return The new object or `NULL` on error.
 */
#define Nst_obj_custom_ex(type, data, dstr)                                   \
    _Nst_obj_custom_ex(sizeof(type), (void *)(data), #type, dstr)

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

/* The signature of a function used to get the constant of a library. */
typedef Nst_ObjRef *(*Nst_ConstFunc)(void);

/**
 * A structure representing an object declaration for a C library.
 *
 * @param ptr: the pointer to the function
 * @param arg_num: the number of arguments if the object is a function, `-1`
 * for other declarations
 * @param name: the name of the declared object
 */
NstEXP typedef struct _Nst_Declr {
    void *ptr;
    isize arg_num;
    const char *name;
} Nst_Declr;

/**
 * Check the types of the arguments and extracts their values.
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
NstEXP bool NstC Nst_extract_args(const char *types, usize arg_num,
                                  Nst_Obj **args, ...);

/**
 * Create an object with custom data.
 *
 * @param size: the size of the data to insert
 * @param data: the data to copy
 * @param name: the name of the object's type
 */
NstEXP Nst_ObjRef *NstC _Nst_obj_custom(usize size, void *data,
                                        const char *name);
/**
 * `_Nst_obj_custom` which allows to specify a destructor.
 *
 * @brief Note: the destructor takes the object itself, call
 * `Nst_obj_custom_data` to access the data to destroy.
 *
 * @brief Warning: The destructor **must not** free the object. It should just
 * destroy its data.
 */
NstEXP Nst_ObjRef *NstC _Nst_obj_custom_ex(usize size, void *data,
                                           const char *name, Nst_ObjDstr dstr);
/**
 * @return The data of an object created with `Nst_obj_custom`.
 */
NstEXP void *NstC Nst_obj_custom_data(Nst_Obj *obj);

bool _Nst_import_init(void);
void _Nst_import_quit(void);
void _Nst_import_close_libs(void);
bool _Nst_import_push_path(Nst_ObjRef *path);
void _Nst_import_pop_path(void);
void _Nst_import_clear_paths(void);

/* Import a library given its path. The path is expanded by the function. */
NstEXP Nst_ObjRef *NstC Nst_import_lib(const char *path);

/**
 * Get the absolute path of a library.
 *
 * @brief If the library is not found on the given path, the standard library
 * directory is checked.
 *
 * @param rel_path: the relative path used to import the library
 * @param path_len: the length in bytes of `rel_path`
 *
 * @return The path on success and `NULL` on failure. The error is set. This
 * function fails if the specified library is not found.
 */
NstEXP Nst_ObjRef *NstC Nst_import_full_lib_path(const char *rel_path,
                                                 usize path_len);
/**
 * Get the absolute path to a file system object.
 *
 * @brief Note: the absolute path is allocated on the heap and should be freed
 * with `Nst_free`.
 *
 * @param file_path: the relative path to the object
 * @param out_buf: the buf where the absolute path is placed
 * @param out_file_part: where the start of the file name inside the file path
 * is put, this may be `NULL` in which case it is ignored
 *
 * @return The length in bytes of the absolute path or `0` on failure. The
 * error is set.
 */
NstEXP usize NstC Nst_abs_path(const char *file_path, char **out_buf,
                               char **out_file_part);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !LIB_IMPORT_H
