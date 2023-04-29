# Introduction

## Nomenclature

All functions and global variables start with the prefix `nst_` and use snake
case. Functions that are prefixed with `_nst_` are used internally and should
not be used in outside programs. An exception are the functions that have a
wrapper that implicitly casts the arguments to the correct types.

All structs and enums are redefined as a type that start with the prefix `Nst_`
and use pascal case. To use the original struct the prefix is `_Nst_`. For
example `Nst_OpErr` and `struct _Nst_OpErr` are equivalent.

Most macros start with the prefix `NST_` and use uppercase snake case. Macros
that start with `_NST_`, like functions, are used internally and should not be
used.  
The macros that do not start with these prefixes are `OBJ`, `GGC_OBJ`, `AS_INT`,
`AS_REAL`, `AS_BYTE`, `AS_BOOL`, `STR`, `VECTOR`, `ARRAY`, `SEQ`, `ITER`,
`IOFILE`, `FUNC` and `MAP` which simply cast a type of object to another. The
ones beginning with `AS_` also extract the value, this means that
`AS_INT(num_obj)` will cast `num_obj` to a `Nst_Int`, not a `Nst_IntObj *`.  
The macros used as include guards also do not start with `NST_`.  
There are other macros that start with `nst_` and these implicitly cast the
arguments of a function to the correct type, for example if `map` is of type
`Nst_Obj *` and `key` of type `Nst_StrObj *` you would have to write
`_nst_map_get(MAP(map), OBJ(key))` but with the macro you can just call the
function: `nst_map_get(map, key)`.

## Type definitions

Instead of using standard C types, the file `typedefs.h` defines some clearer
types inspired by Rust:

| Type name | C Equivalent         |
| --------- | -------------------- |
| `i8`      | `char`               |
| `u8`      | `unsigned char`      |
| `i16`     | `short`              |
| `u16`     | `unsigned short`     |
| `i32`     | `long`               |
| `u32`     | `unsigned long`      |
| `i64`     | `long long`          |
| `u64`     | `unsigned long long` |
| `f32`     | `float`              |
| `f64`     | `double`             |
| `usize`   | `size_t`             |
| `isize`   | `ptrdiff_t`          |

## Code style

- use 4 spaces for indentation, not tabs
- prefer the types defined in `typedefs.h` over the built-in name but they can
  be used when the code gives warnings or is harder to read because of casts
- no line should end with whitespace
- function definition: return type and name on the same line, arguments possibly
  on the same line but if it is too long the types may be aligned on new lines
  with the first type and the name should be aligned with the one following the
  longest type
  ```better-c
  Nst_LList *nst_tokenizef(i8             *filename,
                           bool            force_cp1252,
                           i32            *opt_level,
                           bool           *no_default,
                           Nst_SourceText *src_text,
                           Nst_Error      *error)
  {
      ...
  }
  ```
- the asterisk of pointers should have a space only at the left
- there should be a space in the parenthesis of `if`, `while` and `switch`
- all binary operators should be surrounded by spaces but they can be omitted
  to show the lowest priority
- there should be a space after commas, colons and semicolons if they are not
  part of a ternary conditional expression
- all braces (`{` and `}`) should be on their own line and may never be omitted,
  they keep the indentation of the parent block
  ```better-c
  // like this
  if ( condition_1 || condition_2 )
  {
      i32 *ptr = &var;
  }

  // not like this
  if (condition_1 || condition_2) {
      i32 *ptr = &var;
  }

  // or this
  if (condition_1 || condition_2)
      i32 *ptr = &var;
  ```
- the return statement should not contain redundant parenthesis
- breaking long function or macro calls should happen after the opening
  parenthesis and the closing parenthesis should be on the same line as the
  last argument
  ```better-c
  // like this
  nst_string_new(
      buf,
      buf_len,
      true,
      err);

  // not this
  nst_string_new(buf,
                 buf_len,
                 true,
                 err);

  // or this
  nst_string_new(
      buf,
      buf_len,
      true,
      err
  );
  ```

## Undocumented files

- `obj_ops.h`
- `runtime_stack.h`
- `sequence.h`
- `simple_types.h`
- `str.h`
- `tokens.h`
- `var_table.h`

## Documented files

- `argv_parser.h`
- `compiler.h`
- `encoding.h`
- `error.h`
- `error_internal.h`
- `format.h`
- `function.h`
- `ggc.h`
- `global_consts.h`
- `hash.h`
- `instructions.h`
- `interpreter.h`
- `iter.h`
- `lexer.h`
- `lib_import.h`
- `llist.h`
- `map.h`
- `mem.h`
- `nest.h`
- `nodes.h`
- `obj.h`
- `optimizer.h`
- `parser.h`
- `typedefs.h`
