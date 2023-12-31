## 0.15.0

### Nest

**Changes**

- changed the file object to string cast from `<IOFile ----- >` to `<IOFile[-----]>`

**Bug fixes**

- fixed a crash that occurred when casting a `IOFile` object to a `Str`

### C API

**Additions**

- added `Nst_ErrorKind` enum
- added `Nst_error_add_positions` to add a pair of positions to a traceback
- added `Nst_ggc_was_init` to check if the garbage collector was initialized
- added `Nst_IntrState` struct
- added `Nst_gstate_get_es` and `Nst_state_set_es`
- added `runner.h` along with the following functions:
  - `Nst_es_init`
  - `Nst_es_destroy`
  - `Nst_es_init_vt`
  - `Nst_execute`
  - `Nst_es_set_cwd`
  - `Nst_es_push_module`
  - `Nst_es_push_func`
  - `Nst_es_push_paused_coroutine`
  - `Nst_es_force_function_end`
- added `Nst_cl_args_init`
- added `_Nst_override_supports_color`
- added `_Nst_EM_WRONG_ARG_NUM_FMT` macro to format correctly wrong argument numbers
- added `Nst_init`, `Nst_quit` and `Nst_was_init`
- added `Nst_assert` and `Nst_assert_c` to make assertions

**Changes**

_New program execution system_:

Programs have been decoupled from the interpreter. Now any program can be
executed by setting the `es` field (of type `Nst_ExecutionState`) of the global
`Nst_IntrState`. All the information needed to execute a program is saved in
the `Nst_ExecutionState` struct and this can be swapped into the interpreter
when no recursive calls are active and it allows to execute a completely
different program where it was left off.

_New error handling system_:

Previously errors were handled in a very messy way. There was `Nst_Error` which
was used internally, `Nst_OpErr` which was used by libraries and
`Nst_Traceback` which was used during execution. Now interpreter and program
execution are separate and because of this the interpreter state can be
initialized well before anything is executed on it. This removes the need for
`Nst_Error` and `Nst_OpErr` which are replaced in the new `Nst_ExecutionState`
and `Nst_IntrState` by a `Nst_Traceback` field that contains all the
information needed.

_General changes_:

- removed `error` parameter from `Nst_compile`
- removed `Nst_Error` and `Nst_OpErr` structs
- replaced the `error` field in `Nst_Traceback` with `error_occurred`, `error_name` and `error_msg`
- removed `Nst_print_error`
- changed `Nst_print_traceback` to take a pointer instead of a plain traceback
- changed `Nst_error_occurred` to return a `Nst_ErrorKind` instead of a `bool`
- changed `Nst_error_get` to return a `Nst_Traceback` instead of a `Nst_OpErr`
- changed all `Nst_set_internal_*` functions to get a `Nst_Traceback` instead of a `Nst_Error` for the first argument
- removed `Nst_set_internal_error_from_op_err`
- changed `Nst_get_state` to return `Nst_IntrState`
- changed `Nst_ExecutionState` to now only contain information about the running program
- removed `error` argument from `Nst_tokenizef`, `Nst_tokenize` and `Nst_normalize_encoding`
- removed `error` argument from `Nst_optimize_ast` and `Nst_optimize_bytecode`
- removed `error` argument from `Nst_parse`
- added `v_stack` argument to
  - `Nst_vstack_init`
  - `Nst_vstack_push`
  - `_Nst_vstack_push`
  - `Nst_vstack_pop`
  - `Nst_vstack_peek`
  - `Nst_vstack_dup`
  - `Nst_vstack_destroy`
- added `f_stack` argument to
  - `Nst_fstack_init`
  - `Nst_fstack_push`
  - `Nst_fstack_pop`
  - `Nst_fstack_peek`
  - `Nst_fstack_destroy`
- combined `func`, `call_start`, `call_end`, `vt`, `idx` and `cstack_size` arguments of `Nst_fstack_push` into `call`
- added `c_stack` argument to
  - `Nst_cstack_init`
  - `Nst_cstack_push`
  - `Nst_cstack_peek`
  - `Nst_cstack_pop`
  - `Nst_cstack_destroy`
- combined `inst_idx`, `v_stack_size` and `f_stack_size` arguments of `Nst_cstack_push` into `frame`
- renamed `Nst_ggc_delete_objs` to `_Nst_ggc_delete_objs`
- removed `_Nst_unload_libs`
- removed `argc` and `argv` arguments from `_Nst_parse_args`
- added `arg_num` argument to `Nst_call_func`
- renamed `Nst_run_func_context` to `Nst_run_paused_coroutine` and modified its arguments
- removed `Nst_state_init` and `Nst_state_free`
- renamed `Nst_ENDIANNESS` to `Nst_BYTEORDER`

**Bug fixes**

- fixed `Nst_obj_eq_c` and `Nst_obj_ne_c` leaving a dangling reference of the resulting boolean object
- fixed type objects not being freed properly
- fixed many constants not being destroyed at the end of the program
- fixed a possible read-after-free bug in the garbage collector

---

## 0.14.0

### Nest

**Additions**

- added `IEND` constant to `stditutil.nest`
- added `map_i`, `slice_i`, `filter_i`, `extend`, `copy` and `deep_copy` to `stdsequtil.nest`
- added `is_nan` and `is_inf` functions to `stdmath.nest`
- added `inf_and_nan` option to `stdjson.nest` to allow for `NaN` and `Infinity` to be treated as numeric literals
- added `is_inf` and `is_nan` functions to `stdmath.nest`
- added the 'contains' stack operator `<.>` to check if an object is inside another
- added `read_symlink` functio to `stdfs.nest`
- added `gmt_date` to `stdtime.nest`

**Changes**

- now `itu.new_iterator` takes three arguments instead of four
- now `itu.repeat` will repeat forever if `count` is negative
- now `itu.cycle` will also accept `Iter` objects
- removed `iter_is_done` from `stditutil.nest`
- now the iterator of a for-as loop is implicitly casted to `Iter`
- now `\U` in strings will only accept 6 hex characters instead of 8, since U+10FFFF is the highest codepoint accepted
- now string length and indexing will match the characters and not the bytes (e.g. `'àèì'.1` now is `'è'` and before was `'\xa0'`)
  - removed `to_iter`, `get_len` and `get_at` from `stdcodecs.nest` since they are no longer needed
- removed `ljust` and `rjust` from `stdsutil.nest` in favour of `justify`
  - with `justify` a positive length means justify left and a negative one means justify right
- now `su.repr` will display in `\xhh`, `\uhhhh` or `\Uhhhhhh` unprintable characters (control characters, non-characters, surrogates)
- added an argument to `sequ.map`, `sequ.sort` and `rand.shuffle` to specify whether the sequence should be modified in-place or a new one should be created
- added an argument to `json.load_f` and `json.dump_f` to specify the encoding of the file
- renamed `allow_comments` and `allow_trailing_commas` in `json.OPTIONS` to `comments` and `trailing_commas` respectively
- removed `contains` from `stdsequtil.nest` in favour of the new stack operator
- added `from` and `to` arguments to `lfind` and `rfind` in `stdsutil.nest`
- now casting a `nan` or `inf` value to an `Int` or `Byte` will throw an error

**Bug fixes**

- reworked the garbage collector to work more reliably and remove many types of crashes
- fixed error printing sometimes adding an empty line after a fully red one
- fixed incorrect behaviour of `sequ.slice` on some occasions
- fixed a crash that occurred when an error was thrown inside a function in `sequ.rscan`
- fixed `json.load_f` and `json.dump_f` not working correctly with UTF-8 file names on Windows
- fixed Nest not finding files with non-ASCII charcters (both command-line and libraries) on Windows
- fixed `su.is_charset` returning an incorrect result on some non-ASCII strings (e.g. `'à' 'èĠ' @su.is_charset` now returns `false`, before it would return `true`)
- fixed `io.println` not printing the full string if it contained a NUL character
- fixed some errors passing silently through `io.println`
- fixed `inf` and `nan` values not being casted correctly to strings
- fixed a bug where any byte using a hexadecimal literal would be interpreted as zero
- fixed `CP-1250` encoding not working
- fixed `join` in `stdsutil.nest` causing a `Memory Error` if the passed sequence was empty
- fixed `rtrim` and `trim` in `stdsutil.nest` that if used could later cause crashes
- fixed iterating through strings and creating sequences from strings that contain unicode characters which would create invalid strings
- fixed many inconsistencies between the documentation and the functions in the standard library
- fixed `<` and `>` sometimes not working correctly with non-ASCII characters
- fixed octal escapes higher than `\177` not working and possibly causing a crash
- fixed error printing causing a crash when non-ASCII character were present on the line being printed

### C API

**Additions**

- added `p_next` field to `Nst_Obj`
- added object pools to improve performance
- added `_Nst_obj_free` to free an object's memory or add it to its type's pool
- added `Nst_TYPE_STR` to get the string of a type object
- added `CONT_TYPE` and `Nst_ContType` for types of containers
- added `IEnd` field to `Nst_TypeObjs` and `IEnd_iend` to `Nst_Consts`
- added `c_inf`, `c_nan`, `c_neginf` and `c_negnan` to `Nst_StrConsts`
- added `Nst_true_ref`, `Nst_false_ref`, `Nst_null_ref`, `Nst_iend` and `Nst_iend_ref` to `global_consts.h`
- added `Nst_FLAG` macro to create the value of the flag of an object
- added `indexable_str` and `true_len` fields to `Nst_StrObj`.
- added `Nst_string_new_len` to create strings with a known character length
- added `Nst_string_get_next_ch` to iterate over a string's characters without generaing the `indexable_str` field
- added `Nst_FLAG_STR_IS_ASCII`, `Nst_FLAG_STR_INDEX_16`, `Nst_FLAG_STR_INDEX_32` and `Nst_FLAG_STR_CAN_INDEX` variants to `Nst_StrFlags`
- added `Nst_seq_copy` and `_Nst_seq_copy` to create a shallow copy of sequences
- added `Nst_map_copy` and `_Nst_map_copy` to create a shallow copy of a map
- added `Nst_fopen_unicode` to open files from UTF-8 strings on all platforms
- added `Nst_single_byte_cp` to always get a single-byte encoding
- added `Nst_NestCallable` type as the type of C functions callable from Nest
- added `Nst_sbuffer_copy` and `Nst_buffer_copy` to copy the contents of buffers
- added `Nst_buffer_append_str` to append a string of known length to a buffer
- added `Nst_obj_contains` and `_Nst_obj_contains` to implement the `<.>` operator
- added `Nst_TT_CONTAINS` token type
- added `Nst_string_rfind` to find a substring from the right
- added `Nst_CLEAR_FLAGS` macro to remove all non-reserved flags from a macro

**Changes**

- now `Nst_TypeObj` is no longer an alias of `Nst_StrObj` and contains the pool for the type and the destructor
- moved the `traverse_func` parameter from the object to its type
- moved the `destructor` parameter from the object to its type
- removed `destructor` argument from `_Nst_obj_alloc` and `Nst_obj_alloc`
- changed `_Nst_obj_destroy` to only call the destructor, and not free the memory
- removed `Nst_obj_destroy` and kept only `_Nst_obj_destroy`
- removed `ggc_next` from `Nst_GGCObj`, instead the new `p_next` is used
- renamed `ggc_prev` to `p_prev` in `Nst_GGCObj`
- removed `t_*` from `Nst_StrConsts`
- removed `is_done` field from `Nst_IterObj`
- removed `Nst_iter_is_done` and `_Nst_iter_is_done`
- removed `is_done` argument from `Nst_iter_new`
- removed `Nst_iter_range_is_done`, `Nst_iter_seq_is_done`, `Nst_iter_str_is_done` and `Nst_iter_map_is_done`
- now type shorthands for `Nst_extract_arg_values` that are part of type unions will not cast the object automatically
- renamed `_Nst_string_get_next_ch` and `Nst_string_get_next_ch` to `_Nst_string_next_ch` and `Nst_string_next_ch` respectively
- optimized exponentation
- rename `_Nst_VECTOR_MIN_SIZE` to `_Nst_VECTOR_MIN_CAP`

**Bug fixes**

- fixed `Nst_is_non_character` returning `true` on some valid characters
- fixed `Nst_extract_arg_values` sometimes trying to cast an invalid value
- fixed `Nst_extract_arg_values` crashing when deleting any allocated objects if an error occurred
- fixed `Nst_encoding_from_name` failing to identify valid encoding names
- fixed some bugs with UTF-16 and its variants not working properly
- added some missing documentation for `_Nst_VECTOR_MIN_CAP`, `_Nst_VECTOR_GROWTH_RATIO`, `SEQ`, `ARRAY` and `VECTOR`
- fixed `Nst_string_compare` not working with non-ASCII characters

---

## 0.13.2

### Nest

**Bug fixes**

- fixed input

### C API

**Bug fixes**

- fixed `Nst_sbuffer_expand_to` expanding the buffer when not necessary

---

## 0.13.1

### Nest

**Bug fixes**

- fixed `slice` in `stdsequtil.nest` that could cause a crash if an `Array` or `Vector` was passed

---

## 0.13.0

### Command-line arguments

- removed `--cp1252` in favour of `--encoding`

### Nest

**Additions**

- added `ENDIANNESS` constant and `putenv` function to `stdsys.nest`
- added `new_iterator` to `stditutils.nest`
- added `descriptor`, `encoding`, `is_a_tty`, `is_bin`, `can_read`, `can_write` and `can_seek` to `stdio.nest`
- added string expressions

**Changes**

- the `stdutf8.nest` library has been replaced by the `stdcodecs.nest` library
- added `encoding` argument to `str_to_bytearray` and `bytearray_to_str`
- added `encoding` and `buf_size` arguments to `open`
- optimized interpreter
- now strings are encoded with UTF-8
- now casting `Map` to `Iter` will iterate over the map's items and not create a new iterator
- added `map_func` argument to `sequ.sort`

**Bug fixes**

- fixed many memory leaks
- fixed CP1252 encoding being decoded incorrectly
- fixed a crash that could occur when returning inside a try-catch block
- fixed a crash that could occur when the last statement of a while loop was `..`
- fixed some nodes and instructions missing the position

### C API

**Additions**

- added many new encodings
- added Nst_SizedBuffer
- added `file.h` and removed `Nst_IOFile` logic from `simple_types.h`
- added functions to set the global error and to check it
- added the option to track the position where an object was initialized
- added a `help` rule in makefiles

**Changes**

- now functions and macros begin with Nst_ instead of nst_ and NST_ respectively
- now the operation error is no longer an argument and is instead set globally
- removed error and internal error macros
- removed `Nst_Int`, `Nst_Real`, `Nst_Byte` and `Nst_Bool`; use `i64`, `f64`, `u8` and `bool` instead
- now all exported symbols are documented in the headers
- improved the functionality of `Nst_extract_arg_values`

**Bug fixes**

- fixed inconsistent behaviour of `Nst_extract_arg_values`

**Tools**

- updated `mkclib.nest` to work with the new names and error propagation
- added `generate_docs.nest` to generate Markdown documentation from the headers

---

## 0.12.1

### Nest

**Additions**

- added an experimental and undocumented library `_stdgui.nest` that only has text labels, it will be gradually getting more features
- added many functions to `stdfs.nest`
  - `exists`
  - `is_block_device`
  - `is_char_device`
  - `is_named_pipe`
  - `is_socket`
  - `is_symlink`
  - `make_dir_symlink`
  - `make_file_symlink`
  - `make_hard_link`
  - `normalize`

**Changes**

- **now `<<<` supports Unicode characters on Windows**
- renamed `log` to `ln` in `stdmath.nest`
- renamed `logn` to `log` in `stdmath.nest`, the `base` argument is now optional and defaults to 10
- removed `log2` and `log10`
- added a fourth argument to `enumerate` in `stdsutil.nest` to invert the order of the index and item in the array
- now any functions that expect a `Bool` object as one of the arguments will accept any object that is then casted to a boolean
- the C ABI became the C API

**Bug fixes**

- fixed many links in the documentation
- fixed `step` argument of `enumerate` in `stditutil.nest`
- fixed do-while loops getting compiled like normal while loops
- fixed some casts from `Str` to `Int` and `Byte` being considered valid when they weren't and vice versa
- fixed `count` in `stdsequtil.nest` that could cause undefined behaviour
- fixed `<<<` having a weird behaviour
- fixed many functions in `stdfs.nest`, `stdsys.nest` and `stdio.nest` not supporting unicode strings on Windows

---

### C API

**Additions**

- added `nst_vector_create`, `nst_array_create`, `nst_vector_create_c` and `nst_array_create_c` to `sequence.h`
- added `NST_UNSET_ERROR` to `lib_import.h`
- added `nst_number_to_u8`, `nst_number_to_int`, `nst_number_to_i32`, `nst_number_to_i64`, `nst_number_to_f32`, `nst_number_to_f64` and `nst_obj_to_bool` to `simple_types.h`
- added many functions to `encoding.h`:
    - `nst_check_ascii_bytes`
    - `nst_ascii_to_utf32`
    - `nst_ascii_from_utf32`
    - `nst_utf8_to_utf32`
    - `nst_utf8_from_utf32`
    - `nst_utf16_to_utf32`
    - `nst_utf16_from_utf32`
    - `nst_check_utf32_bytes`
    - `nst_utf32_to_utf32`
    - `nst_utf32_from_utf32`
    - `nst_check_1252_bytes`
    - `nst_1252_to_utf32`
    - `nst_1252_from_utf32`
    - `nst_translate_cp`
    - `nst_cp`
    - `nst_char_to_wchar_t`
    - `nst_wchar_t_to_char`

**Changes**

- removed `err` argument from `nst_map_get_str` and `nst_map_drop_str`
- added `err` argument to `nst_map_set` and `_nst_map_resize`
- now `_nst_vector_resize` and `nst_vector_append` return `bool`

**Bug fixes**

- now `nst_iter_new` and `nst_tok_new_value` never leave dangling references if the memory allocation fails
- now `nst_get_full_path` always sets `err` on failure
- now strings that contain a NUL byte are fully hashed

---

## 0.12.0

### Nest

**Additions**

- added `sleep_ms`, `monotonic_time` and `monotonic_time_ns` to `stdtime.nest`
- added `lscan` and `rscan` to `stdsequtil.nest`
- added icons to the installer and the executable on Windows
- added the `_raw_exit` function to `stdsys.cnest`, it cannot be accessed by importing `stdsys.nest`
- added `SUPPORTS_COLOR` to `stdsys.nest`

**Changes**

- now command-line arguments can contain Unicode characters on Windows
- now the UTF-8 with BOM encoding is supported
- now `>>>` will not flush stdout each time
- improved memory usage when importing C libraries
- modified the behaviour of `sys.exit` that now throws an error that will be ignored by try-catch blocks but can be caught with `err.try`
- added a third optional argument to `err.try` that when set to true catches `sys.exit`
- changed many function names in `stdfs.nest` to better fit the naming convention or to make them clearer
    - `isdir` → `is_dir`
    - `mkdir` → `make_dir`
    - `mkdirs` → `make_dirs`
    - `rmdir` → `remove_dir`
    - `rmdir_recursive` → `remove_dirs`
    - `isfile` → `is_file`
    - `rmfile` → `remove_file`
    - `list_dir_recursive` → `list_dirs`
    - `path` → `parent_path`
- renamed `virtual_iof` to `virtual_file` in `stdio.nest`
- now `open` in `stdio.nest` permits omitting the second parameter
- now `Byte` objects support `~` and `-:`
- now binary libraries end with `.cnest` instead of `.dll` or `.so` to allow cross-platform importing
- inverted order of coroutine and args in `co.call`
- the documentation is now hosted on ReadTheDocs
- now the functions in `stdfs.nest` will throw a system error if something fails
- now `count` in `stdsequtil.nest` will count any non-overlapping substring if both arguments are strings

**Bug Fixes**

- fixed many bugs and crashes in `stdjson.nest`
- fixed many memory leaks
- now all memory allocations refer to the main process
- fixed `dt.year` returning the years passed from 1900 instead of the current year (it would return 123 instead of 2023)
- fixed `sys.exit` causing a segmentation fault
- fixed possible crash when catching a circular import error
- now failed memory allocations are correctly handled
- now deeply nested structures will not crash the parser
- fixed general object to `Str` cast on Linux not displaying the address correctly, now it is the same as Windows
- fixed a crash with `sequ.slice` when the resulting slice would have a negative size
- now colors work properly on Windows consoles that require the activation of ANSI escapes
- fixed arguments on Windows not being decoded correctly
- fixed error message for using `%` with incompatible types

---

## 0.11.2

### Nest

**Additions**

- added `to_title` and `is_title` to `stdsutil.nest`
- added `count` to `stdsequtil.nest`
- added `println` to `stdio.nest`

**Changes**

- now identifiers can contain unicode characters (`💻` can be used as a variable name)
- now `contains` in `stdsequtil.nest` also accepts two strings as an input
- now `$` accepts also a function and returns the maximum number of arguments it takes (`>>> $(## [])` prints `0`)
- now `virtual_iof` accepts an optional second argument that specifies the size of the buffer

**Bug Fixes**

- fixed a crash happening when a lambda was not used and the optimization level was above 1
- fixed a crash happening when calling a C function that expected at least one argument but was given none
- fixed many issues reguarding the `Str` to `Byte` cast
- fixed the error message for `-:` writing `'-'` instead of  `'-:'`
- now errors that contain unicode characters will have the correct number of spaces and carets
- fixed number positions only including the first character (again)
- fixed `-//-` that would not close correctly the multiline comment

---

## 0.11.1

### Nest

**Changes**

- removed `_advance_` from iterators and made them around 15% faster
- improved `itu.chain` to take iterators and strings

---

## 0.11.0

**Additions**

- added `\u` and `\U` string escapes
- added the `stdutf8.nest` library
- added `parse_int` to `stdsutil.nest`

**Changes**

- improved `Str` to `Int`, `Str` to `Real` and `Str` to `Byte` casts to support underscores and non-decimal literals
- now string representations will use `\e` instead of `\x1b` and non-control UTF-8 characters are recognized and remain untouched

**Bug Fixes**

- fixed a crash that occurred when getting the value of an ended string iterator

---

## 0.10.0

### Nest

**Additions**

- added `bin`, `oct` and `hex` to `stdsutil.nest` to have the binary, octal and hexadecimal representation of integers respectively
- added `center` to `stdsutil.nest`
- added `_get_cwd` and `_set_cwd` to `stdsys.nest`
- added the `stdjson.nest` module that can load and dump json data

**Changes**

- from now on the documentation will be packaged into the release to always keep correct information
- now `co.call` accepts `null` for the first argument and behaves like passing an empty array
- modified many functions of the standard library to take optional arguments
- now if you call a function with less arguments than expected the remaining ones are filled with `null` values
- now the CP1252 encoding is officially supported along with UTF-8 and ASCII
- now `Real` to `Str` will add `.0` if the number does not contain a dot or is not in scientific notation (`Str :: 1.0` now is `'1.0'` where before it was `'1'`)

**Bug Fixes**

- fixed `co.call` resulting in undefined behaviour when not passing a sequence as the first argument
- now coroutines are correctly set as `ended` when an error occurs inside the coroutine
- fixed `Func` to `Str` cast using 'vars' instead of 'var' when the function only accepted one argument
- fixed error printing when a file that had its new lines changed from CRLF to LF and an error occurred on the last line
- fixed repr of strings containing unprintable characters being incorrect and possibly causing crashes
