# Introduction

The C API allows you to write C libraries for Nest or to use Nest inside your C
or C++ program. These documents explain all the symbols defined inside the
library.

## Type definitions

Instead of using standard C types, the file `typedefs.h` defines some clearer
types inspired by Rust:

| Type name | C Equivalent |
| --------- | ------------ |
| `i8`      | `int8_t`     |
| `u8`      | `uint8_t`    |
| `i16`     | `int16_t`    |
| `u16`     | `uint16_t`   |
| `i32`     | `int32_t`    |
| `u32`     | `uint32_t`   |
| `i64`     | `int64_t`    |
| `u64`     | `uint64_t`   |
| `f32`     | `float`      |
| `f64`     | `double`     |
| `usize`   | `size_t`     |
| `isize`   | `ptrdiff_t`  |

## Code style

### Line length

Each line can be at most 79 characters long, it can be exceeded only be one
punctuation character that is not a comment.

```better-c
/* -------------------- This line is 79 characters long -------------------- */
                                                                             //
// This is good                                                              //
bool res = Nst_translate_cp(                                                 //
    &Nst_cp_utf8,                                                            //
    &Nst_cp_utf16,                                                           //
    (void *)str, len,                                                        //
    (void **)&out_str, NULL);                                                //
                                                                             //
// This is also good                                                         //
NstEXP bool NstC Nst_translate_cp(Nst_CP *from, Nst_CP *to, void *from_buf,  //
                                  usize from_len, void **to_buf, usize *to_len);
                                                                             //
// This is too long                                                          //
bool res = Nst_translate_cp(&Nst_cp_utf8, &Nst_cp_utf16, (void *)str, len, (void **)&out_str, NULL);
```

### Indentation

Each level of indentation must be of exactly four spaces, not tabs. There is no
limit to how many levels of indentation you can have.

### Type names

When there is a corresponding type, use the type definition inside `typedefs.h`.
You can use built-in types when there are type warnings.

### Function declarations and definitions

The function definition should keep the return type and the function name on
the same line, the arguments, if they don't fit into the line, can span
multiple lines and are always indented to the first one.

```better-c
// This is good
NstEXP bool NstC Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, u32 sep,
                                  i64 *out_num, Nst_StrView *out_rest);

// This is wrong
NstEXP bool *NstC
Nst_sv_parse_int(Nst_StrView sv, u8 base, u32 flags, u32 sep, i64 *out_num,
                 Nst_StrView *out_rest);

// This is also wrong
NstEXP bool NstC Nst_sv_parse_int(
    Nst_StrView sv,
    u8 base,
    u32 flags,
    u32 sep,
    i64 *out_num,
    Nst_StrView *out_rest
);
```

Furthermore, when implementing the function the curly braces should always
appear in their own lines.

```better-c
// This is good
i8 *Nst_wchar_t_to_char(wchar_t *str, usize len)
{
    ...
}

// This is wrong
i8 *Nst_wchar_t_to_char(wchar_t *str, usize len) {
    ...
}
```

### Nomenclature

All exported names should begin with `Nst_`, shorter integers, `NstC`,
`NstEXP` and `NstOBJ` are the only exceptions.

Macros, aside from `Nst_` and `_Nst_`, should use uppercase with the words
separated by underscores (Ex. `Nst_RETURN_COND`).

Functions, aside from `Nst_` and `_Nst_`, should use snake case, with lowercase
words separated by underscores (Ex. `Nst_encoding_from_name`).

Types (including structs, enums, typedefs and unions), aside from `Nst_` or
`_Nst_` should use pascal case where each word has its first letter capitalized
(Ex. `Nst_StrView`).

When declaring a struct it should always be in this format:

```better-c
typedef struct _Nst_StructName {
    ...
} Nst_StructName;
```

where `StructName` is the name of the struct. This means that any structure in
Nest can be accessed using the name of the alias preceded by an underscore:
`struct _Nst_Obj` and `Nst_Obj` are equivalent.

### Function calls

When calling a functions there should be no whitespace between the name and the
opening bracket, before the commas separating the arguments or around the
closing bracket. There should always be one space after the comma.
If the line is too long, arguments can be split on multiple lines that end with
the commas that separate them. The closing bracket and semicolon should be on
the same line as the last argument. You may put relate arguments on the same
line, otherwise each argument goes on a separate line.

```better-c
// This is good
Nst_calloc_c(_Nst_MAP_MIN_SIZE, Nst_MapNode, NULL);

// This is also good
bool res = Nst_translate_cp(
    &Nst_cp_utf8,
    &Nst_cp_utf16,
    (void *)str, len,
    (void **)&out_str, NULL);

// This is wrong
bool res = Nst_translate_cp(
    &Nst_cp_utf8,
    &Nst_cp_utf16,
    (void *)str, len,
    (void **)&out_str, NULL
);

// This is also wrong
Nst_calloc_c (_Nst_MAP_MIN_SIZE, Nst_MapNode, NULL);
Nst_calloc_c( _Nst_MAP_MIN_SIZE, Nst_MapNode, NULL );
Nst_calloc_c(_Nst_MAP_MIN_SIZE , Nst_MapNode , NULL);
```

### Macros

A macro should always require a semicolon when put on its own line.
When defining a macro, if it spans multiple lines the backslashes should be put
as the last character of the line though in this case you should prefer a
function over a macro.

If the macro needs to be wrapped inside a `do { ... } while (0)`, the `do {`
should be in the same line as the name of the macro and `} while (0)` on the
last line, at the same indentation of the rest of the macro's body using always
a zero `0` and *not* `false`. The body itself should be at one level of
indentation.

```better-c
// This is good
#define Nst_OBJ_IS_TRACKED(obj) (GGC_OBJ(obj)->ggc_list != NULL)

// This is also good
#define Nst_GGC_OBJ_INIT(obj) do {                                            \
    obj->p_prev = NULL;                                                       \
    obj->ggc_list = NULL;                                                     \
    obj->ggc_ref_count = 0;                                                   \
    Nst_SET_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED);                             \
    Nst_ggc_track_obj((Nst_GGCObj *)(obj));                                   \
    } while (0)

// This is wrong
#define Nst_GGC_OBJ_INIT(obj, trav_func, track_function)                      \
    do {                                                                      \
        obj->p_prev = NULL;                                                   \
        obj->ggc_list = NULL;                                                 \
        obj->ggc_ref_count = 0;                                               \
        Nst_SET_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED);                         \
        Nst_ggc_track_obj((Nst_GGCObj *)(obj));                               \
    } while (0)

#define Nst_GGC_OBJ_INIT(obj, trav_func, track_function) do {                 \
    obj->p_prev = NULL;                                                       \
    obj->ggc_list = NULL;                                                     \
    obj->ggc_ref_count = 0;                                                   \
    Nst_SET_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED);                             \
    Nst_ggc_track_obj((Nst_GGCObj *)(obj));                                   \
    } while (false)

#define Nst_GGC_OBJ_INIT(obj, trav_func, track_function) do { \
    obj->p_prev = NULL; \
    obj->ggc_list = NULL; \
    obj->ggc_ref_count = 0; \
    Nst_SET_FLAG(obj, Nst_FLAG_GGC_IS_SUPPORTED); \
    Nst_ggc_track_obj((Nst_GGCObj *)(obj)); \
    } while (0)
```

### Documentation

The header file should always contain a short description of what the file
contains and the author in this format.

```better-c
/**
 * @file filename.h
 *
 * @brief A brief description
 *
 * @author The author's name
 */
```

Above each public definition there must be an exhaustive description of how the
thing works and behaves. If one line is sufficient, use C89-style comments.
Always put a space after `/*` and before `*/`.

```better-c
/* Nst_CheckBytesFunc for ASCII */
NstEXP i32 NstC Nst_check_ascii_bytes(u8 *str, usize len);
```

When the description does not fit into one line but the function is not
complex, you should use a multi line comment with the following format:

```better-c
/**
 * Sets the global operation error creating a string object from the given
 * message and using "Syntax Error" as the name.
 */
NstEXP void NstC Nst_set_syntax_error_c(const i8 *msg);
```

If the function is more complex you can use a more complete form of
documentation that explains the parameters, the return value and gives some
specifications:

```better-c
/**
 * Compiles the AST.
 *
 * @brief Both ast and error are expected to be not NULL.
 *
 * @param ast: the AST to compile, will be freed by the function
 * @param is_module: whether the AST is of an imported module or of the main
 * file
 * @param error: the error set if one occurs
 *
 * @return The function returns the compiled Nst_InstList or NULL if an error
 * occurred.
 */
NstEXP Nst_InstList *NstC Nst_compile(Nst_Node *ast, bool is_module,
                                      Nst_Error *error);
```

Note that tags (`@brief`, `@return`, ...) only work in multi-line documentation
blocks so this does not work:

```better-c
/* @return The value of a Nest `Str` object. */
NstEXP u8 *NstC Nst_str_value(Nst_Obj *str);
```

and it must be written like this:

```better-c
/**
 * @return The value of a Nest `Str` object.
 */
NstEXP u8 *NstC Nst_str_value(Nst_Obj *str);
```
