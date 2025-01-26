## 0.15.1

### Nest

**Additions**

- added a new syntax for raw strings with backticks

**Bug fixes**

- fixed expressions escapes in strings that would not close properly when the last character was `-`
- fixed an infinte loop occurring when escaping a non-ASCII character in the source code


## 0.15.0

**Nest can now be compiled with `clang`!** Check `make help` for more info.

### Nest

**Additions**

- added `is_space`, `lremove`, `rremove` and `rsplit` to `stdsutil.nest`
- added `del_env` and `get_capacity` to `stdsys.nest`
- added `enum` to `stdsequtil.nest`
- added `batch` and `batch_padded` to `stditutil.nest`
- added `std.nest`, a file that imports all the standard library available under standard import names (e.g. `std.io.println`)
- added `set_option`, `get_option`, `clear_options` and `OPTION` to `stdjson.nest`
- added `_debug_` and `_debug_arch_` predefined variables when compiling in debug mode, `_debug_` is set to `true` and `_debug_arch_` is set to `x64` on 64-bit platforms and `x86` on 32-bit ones
- added `reverse` and `reverse_i` to `stdsequtil.nest`
- added `rsplit` to `stdsutil.nest`
- added `fmt` to `stdsutil.nest` to format values
- added `encoding_info` to `stdcodecs.nest`
- merged `get_fpi` and `move_fpi` into `seek`
- added `time_creation`, `time_last_access` and `time_last_write` to `stdfs.nest`
- added `path.stem` to `stdfs.nest`

**Changes**

- now line endings will be ignored when the expression should not end, this includes the following cases:
  - the condition of while and do-while loops
  - the number of repeats in for loops
  - the iterator in for-as loops
  - the expression and the cases in switch statements
  - expressions inside parenthesis
  - inside maps, vectors and arrays
  - when a line ends with a local-stack operator
- changed the file object to string cast from `<IOFile ----- >` to `<IOFile[-----]>`
- renamed `putenv` in `stdsys.nest` to `set_env` and added an optional argument `overwrite`
- renamed `getenv` in `stdsys.nest` to `get_env`
- now `relative_path` in `stdfs.nest` will use the current working directory if no base is given
- renamed `pause` to `yield` in `stdco.nest`
- now the `args` argument of `co.call` is returned by `co.yield` when the function resumes
- now `co.generator` accepts a second argument which is the arguments to use when calling the function
- now `err.try` will accept fewer arguments than the function requires
- removed `get_options`, `set_options` and `OPTIONS` in `stdjson.nest`
- renamed `_get_cwd` and `_set_cwd` to `get_cwd` and `set_cwd` in `stdsys.nest`
- removed `reversed` from `stditutil.nest`
- improved execution times and output printing
- now when pressing `Ctrl-C` an error is thrown rather than the process exiting, this error cannot be caught with a normal try-catch statement but `err.try` is needed
- renamed `su.split` to `su.lsplit`
- reverted the merging of `su.ljust` and `su.rjust` into `su.justify`, now they are again two separate functions
- renamed `su.center` to `su.cjust`
- now when a string is not closed only the end is highlighted rather than the whole string
- now the second argument of `itu.repeat` is optional and it will repeat infinitely if not given, it would repeat infinitely even if the number was negative even if it was not specified in the documentation
- added `timestamp` argument to many `stdtime.nest` functions
- renamed `hours`, `minutes` and `seconds` to `hour`, `minute` and `second`
- reorganized `extension`, `filename`, `join`, `normalize` and `parent_path` to be inside `path` in `stdfs.nest`
- removed `remove_dir`, `remove_dirs` and `remove_file` in favor of `remove` and `remove_all`

**Bug fixes**

- fixed a crash that occurred when casting a `IOFile` object to a `Str`
- fixed a crash that occurred when using `co.pause` inside an expression
- now months are counted from `1` in `stdtime.nest`
- fixed an infinite loop when trying to optimize changed jumps
- fixed `su.lfind` and `su.rfind` that did not work with non-ASCII characters
- fixed `su.center` causing a crash with a negative width
- fixed `su.trim`, `su.ltrim` and `su.rtrim` not working with NUL characters
- fixed `su.is_title` returning an incorrect value when a word had a character that was not a space or a letter before it
- fixed `itu.cycle` not working with Unicode strings
- fixed `itu.chain` not resetting correctly after being iterated over the first time
- fixed many memory leaks
- fixed `cp-1251`, `utf32`, `latin1` encodings not working properly
- fixed `utf32` encoding name
- fixed encoding on `utf16`, `utf16le` and `utf16be` for `U+FFFF`
- fixed various bugs with virtual files
- now `io.get_flags`, `io.can_read`, `io.can_write`, `io.is_bin`, `io.can_seek` and `io.is_a_tty` will throw an error if the file is closed
- now `fs.path.normalize` will no longer normalize `\\?\` at the beginning of a path
- now `fs.path.join` will not add a slash if the first path is an empty string
- fixed `Real` and `Int` to `Byte` casts not working properly

### C API

**Additions**

- added `Nst_ErrorKind` enum
- added `Nst_error_add_positions` to add a pair of positions to a traceback
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
- added `Nst_string_char_len` and `Nst_string_utf8_char_len`
- added `Nst_inst_list_new`
- new parser, added many new node-related elements
  - `Nst_SeqNodeType`, `Nst_NodeData_Ac`, `Nst_NodeData_As`, `Nst_NodeData_Ca`, `Nst_NodeData_Cs`, `Nst_NodeData_Ex`, `Nst_NodeData_Fd`, `Nst_NodeData_Fl`, `Nst_NodeData_Ie`, `Nst_NodeData_Lo`, `Nst_NodeData_Ls`, `Nst_NodeData_Ml`, `Nst_NodeData_Rt`, `Nst_NodeData_Sl`, `Nst_NodeData_So`, `Nst_NodeData_Sw`, `Nst_NodeData_Tc`, `Nst_NodeData_Vl`, `Nst_NodeData_We`, `Nst_NodeData_Wl`, `Nst_NodeData_Ws`
  - `_Nst_node_ac_destroy`, `_Nst_node_ac_init`, `_Nst_node_as_destroy`, `_Nst_node_as_init`, `_Nst_node_ca_destroy`, `_Nst_node_ca_init`, `_Nst_node_cs_destroy`, `_Nst_node_cs_init`, `_Nst_node_ex_destroy`, `_Nst_node_ex_init`, `_Nst_node_fd_destroy`, `_Nst_node_fd_init`, `_Nst_node_fl_destroy`, `_Nst_node_fl_init`, `_Nst_node_ie_destroy`, `_Nst_node_ie_init`, `_Nst_node_lo_destroy`, `_Nst_node_lo_init`, `_Nst_node_ls_destroy`, `_Nst_node_ls_init`, `_Nst_node_ml_destroy`, `_Nst_node_ml_init`, `_Nst_node_rt_destroy`, `_Nst_node_rt_init`, `_Nst_node_sl_destroy`, `_Nst_node_sl_init`, `_Nst_node_so_destroy`, `_Nst_node_so_init`, `_Nst_node_sw_destroy`, `_Nst_node_sw_init`, `_Nst_node_tc_destroy`, `_Nst_node_tc_init`, `_Nst_node_vl_destroy`, `_Nst_node_vl_init`, `_Nst_node_we_destroy`, `_Nst_node_we_init`, `_Nst_node_wl_destroy`, `_Nst_node_wl_init`, `_Nst_node_ws_destroy`, `_Nst_node_ws_init`
  - `Nst_print_node`
- added `Nst_sbuffer_at`, `Nst_sbuffer_pop` and `Nst_sbuffer_shrink_auto`
- added `Nst_VERSION_MAJOR`, `Nst_VERSION_MINOR` and `Nst_VERSION_PATCH` definitions
- added `Nst_DECLR_END`
- added `Nst_ConstFunc`
- added `Nst_memset`
- added `Nst_fmt`, `Nst_vfmt` and `Nst_fmt_objs`
- added `Nst_MSVC`, `Nst_GCC` and `Nst_CLANG`

**Changes**

_New program execution system_:

Programs have been decoupled from the interpreter. Now any program can be executed by setting the `es` field (of type `Nst_ExecutionState`) of the global `Nst_IntrState`. All the information needed to execute a program is saved in the `Nst_ExecutionState` struct and this can be swapped into the interpreter when no recursive calls are active and it allows to execute a completely different program where it was left off.

_New error handling system_:

Previously errors were handled in a very messy way. There was `Nst_Error` which was used internally, `Nst_OpErr` which was used by libraries and `Nst_Traceback` which was used during execution. Now interpreter and program execution are separate and because of this the interpreter state can be initialized well before anything is executed on it. This removes the need for `Nst_Error` and `Nst_OpErr` which are replaced in the new `Nst_ExecutionState` and `Nst_IntrState` by a `Nst_Traceback` field that contains all the information needed.

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
- renamed `Nst_ggc_init` to `_Nst_ggc_init`
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
- renamed `Nst_IC_NEW_OBJ` to `Nst_IC_NEW_INT`
- replaced `Nst_IC_FOR_IS_DONE` with `Nst_IC_JUMPIF_IEND`
- removed `Nst_print_ast`
- renamed `Nst_RETURN_COND` to `Nst_RETURN_BOOL`
- removed `Nst_FUNC_SIGN` and `Nst_DEF_EXTRACT`
- removed `get_func_ptrs` from libraries, now `lib_init` returns the function pointers
- removed `Nst_DeclrList` and renamed `Nst_ObjDeclr` to `Nst_Declr`
- renamed `Nst_MAKE_FUNCDECLR` to `Nst_FUNCDECLR`
- renamed `Nst_MAKE_NAMED_FUNCDECLR` to `Nst_NAMED_FUNCDECLR`
- renamed `Nst_MAKE_OBJDECLR` to `Nst_CONSTDECLR`
- renamed `Nst_MAKE_NAMED_OBJDECLR` to `Nst_NAMED_CONSTDECLR`
- renamed many functions to better fit the naming style:
  - `_Nst_parse_args` to `_Nst_cl_args_parse`
  - `_Nst_override_supports_color` to `_Nst_supports_color_override`
  - `_Nst_set_console_mode` to `_Nst_console_mode_init`
  - `Nst_print_bytecode` to `Nst_inst_list_print`
  - `_Nst_init_objects` to `_Nst_globals_init`
  - `_Nst_del_objects` to `_Nst_globals_quit`
  - `_Nst_call_func` to `_Nst_func_call`
  - `_Nst_get_state` to `_Nst_state_get`
  - all `Nst_string_*` to `Nst_str_*`
- macros to check token type ranges have been made private
- removed `Nst_WIN` in favor of `Nst_MSVC`
- renamed `Nst_GNU_FMT` to `Nst_NIX_FMT` since it is used with clang too
- renamed `true_len` field in strings to `char_len`

**Bug fixes**

- fixed `Nst_obj_eq_c` and `Nst_obj_ne_c` leaving a dangling reference of the resulting boolean object
- fixed type objects not being freed properly
- fixed many constants not being destroyed at the end of the program
- fixed a possible read-after-free bug in the garbage collector
- now the standard library is imported when running Nest on Windows in `x86` debug mode
- fixed the value store in `ill_encoded_byte` retrieved with `Nst_io_result_get_details`
- fixed `Nst_str_copy` that swapped `true_len` and `len`

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
- added `read_symlink` function to `stdfs.nest`
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
- fixed Nest not finding files with non-ASCII characters (both command-line and libraries) on Windows
- fixed `su.is_charset` returning an incorrect result on some non-ASCII strings (e.g. `'à' 'èĠ' @su.is_charset` now returns `false`, before it would return `true`)
- fixed `io.println` not printing the full string if it contained a NUL character
- fixed some errors passing silently through `io.println`
- fixed `inf` and `nan` values not being casted correctly to strings
- fixed a bug where any byte using a hexadecimal literal would be interpreted as zero
- fixed `CP-1250` encoding not working
- fixed `join` in `stdsutil.nest` causing a `Memory Error` if the passed sequence was empty
- fixed `rtrim` and `trim` in `stdsutil.nest` that if used could later cause crashes
- fixed iterating through strings and creating sequences from strings that contain Unicode characters which would create invalid strings
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
- added `Nst_string_get_next_ch` to iterate over a string's characters without generating the `indexable_str` field
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
- optimized exponentiation
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

### Nest

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

---

## 0.9.2

### Nest

**Changes**

- now `&&` and `||` work consistently always returning either the first false element (for `&&`) or the first true element (for `||`)
  ```text
  0 10 || --> 10
  0 10 && --> 0
  ```
- added a custom syntax to create an empty array: `{,}`, changed the string cast of an empty array to `{,}`

**Bug Fixes**

- fixed the end position of value tokens containing a `Real`
- fixed many crashes reguarding coroutines

---

## 0.9.1

### Nest

**Changes**

- improved many casts to `Str`:
  - `IOFile`s now show their flags
  - arrays, vectors and maps, when empty no longer have a space between the opening and closing brackets (`{ }` becomes `{}`)
  - recursive vector references now use `<{.}>` instead of `{.}`
  - if the object does not have a custom cast, it now shows its address in memory (`Str :: (0 -> 10)` can return `<Iter object at 0x00000206A188B8D0>`)
  - now functions show their number of arguments (`Str :: (## a b [])` returns `<Func 2 args>`)
- improved import speed when importing a large number of modules and libraries

**Bug Fixes**

- fixed a crash or incorrect behaviour when comparing two almost identical strings where one has an extra `\0` character at the end
- fixed a crash when passing an empty sequence to `min_seq` or `max_seq` in `stdmath.nest`

---

## 0.9.0

### Nest

**Changes**

- added `generator` to `stdco.nest`
- added `virtual_iof`, `get_flags`, `_set_stdin`, `_set_stdout` and `_set_stderr` to `stdio.nest`, now stdin, stdout and stderr can be redirected to other files
- now `=>` no longer requires an expression after it, instead it will return `null`
- now casting an `IOFile` to a `Bool` will return `true` if the file is open and `false` otherwise
- now negative steps in `sequ.slice` are allowed and do not just return an empty sequence
- removed `from_iter` from `stdsequtil.nest` as it can be replaced with a cast to a vector
- now functions can be written like lambdas, with a signle expression, and lambdas can be written with a block:
  ```
  #add a b => a b +

  ##a b [
      >>> (a ' + ' b '\n' ><)
      => a b +
  ] = add_with_print
  ```
- now error messages will always show the full path to the file
- added the following type casts:
  -  `Map` to `Vector`
  - `Map` to `Array`
  - `Array` to `Map`
  - `Vector` to `Map`
  - `Iter` to `Vector`
  - `Iter` to `Array`
  - `Iter` to `Map`
  - `Real` to `Byte`

**Bug Fixes**

- fixed a crash that occurred when calling a coroutine while it was running, now an error is thrown instead
- fixed `rb+`, `r+b`, `wb+`, `w+b`, `ab+` and `a+b` in `io.open` being considered invalid file modes
- fixed a crash caused by `shrink_stack` not allocating enough memory
- fixed a crash caused by having a step of 0 in `sequ.slice`
- fixed an infinite loop that could occur when a switch statement of an imported file was not closed properly and then a file with a continue statement outside a loop was imported

---

## 0.8.0

### Nest

**Changes**

- **added Linux support!**
- added the unpacking assignment that can be written with curly braces: `{ 1, 2 } = { a, b }`, it also works in for-as loops
- added the sequence call operator (`*@`) that allows you to call a function passing the arguments as a sequence
- renamed `rand_seed` to `seed` in `stdrand.nest`
- added `canonical_path`, `relative_path`, `join`, `path`, `filename` and `extension` functions to `stdfs.nest`
- added `hypot` to `stdmath.nest`

**Bug Fixes**

- comparison operators no longer execute more than once each expression, the following code now prints `Called` only once where before it would be printed twice:
  ```
  #f [
      >>> 'Called\n'
      => 2
  ]

  1 @@f 3 <
  ```
- now all line endings (`\n`, `\r\n` or `\r`) are parsed correctly

---

## 0.7.3

### Nest

**Changes**

- added `gcd`, `lcm`, `gcd_seq`, `lcm_seq` and `abs` to `stdmath.nest`
- the keys in the map returned by `clock_time` and `gmt_clock_time` now are `hour`, `minute` and `second` instead of `hours`, `minutes` and `seconds`
- now runtime stacks (value stack, call stack and catch stack) shrink when they are small enough, before they could only grow

**Bug Fixes**

- now the process ends with exit code 1 when an error occurs
- now `-O3` will not optimize built-in constants if they are repurpused in any scope or if they might be changed through `_vars_` or `_globals_`, now the code will not change its behaviour if you have this enabled
  The following code before would have given different results depending on the optimization level, now everyghing is coherent:
  ```
  'true' = a
  false = _vars_.(a)
  >>> true
  ```
- when vectors are shrinked the program no longer crashes

---

## 0.7.2

### Nest

**Changes**

- now random numbers are generated in the range [-2^63, 2^63) using the Mersenne Twister algorithm, because of this `RAND_MAX` has been removed from `stdrand.nest`

**Bug Fixes**

- fixed `0o` being read as `0b`, now a syntax error is correctly thrown
- fixed hexadecimal literals skipping the next character in the code while lexing (`0 = var [] 0xabc(var +` was valid)
- fixed error positions on some numeric literals where the end was one column off
- fixed a crash that occurred when there was no identifier after `#`
- fixed a crash that occurred when there was a missing `}` or `}>` on repeated sequences
- fixed error message on for loops missing `]`, it now says `unmatched '['` instead of `unexpected token`
- fixed error messages for extractions and container assignments showing the wrong types
- fixed error positions for container assignments
- fixed error printing when there was an indented line

---

## 0.7.1

### Nest

**Changes**

- added `zipn` in `stditutil.nest` to simultaiously iterate over an arbitrary number of sequences
- now if-expressions use less instructions when both cases return `null`
- removed multiline comment spaces in the Sublime Text plugin

**Bug fixes**

- fixed `replace_substr` in `stdsutil.nest` that would sometimes crash
- fixed a crash when an error occurred during the optimization of the bytecode

---

## 0.7.0

### Nest

**Changes**

- Added `stdco.nest` library to create and run coroutines
- Added `starts_with`, `ends_with` and `str_to_bytearray` in `stdsutil.nest`
- Improved error printing on multiple lines
- Printing a lot of lines when printing errors is now much faster
- added binary, octal and hexadecimal integer literals and scientific notation for real literals
  - to write bytes with hexadecimal integer start with `0h` instead of `0x`
  - scientific notation still needs a literal with a decimal point and digitds on both sides
- now underscores (`_`) can be added inside numbers to make them more legible (`100_000_000`)
- changed "Add to PATH variable" to "Add to PATH environment variable" in the installer
- now `clock_time`, `gmt_clock_time`, `clock_datetime` and `gmt_clock_datetime` insert the keys from the least specifit to the most specific, for example calling `dt.clock_time` will now return `{ 'hours': 17, 'minutes': 32, 'seconds': 59 }` instead of `{ 'seconds': 59, 'minutes': 32, 'hours': 17 }`
- added operators when displaying bytecode
- now the bytecode of large programs is displayed faster

**Bug fixes**

- fixed a crash that happened when an error occurred inside a call from a C function
- fixed the indentation of carets when pointing to an indented error
- now when executing code from the `-c` argument no path is added to the import stack
- fixed crashes caused by the garbage collector deleting still reachable objects
- fixed `slice` in `stdsequtil` that would crash in many occasions
- fixed `contains` in `stdsequtil` that would crash when returning a type error

---

## 0.6.3

### Nest

### Fixes

- fixed a lot of memory leaks
- fixed maps causing a crash when iterated on after changing one of the middle keys
- fixed circular import error reporting showing twice the lines of the first file
- optimized `replace_substr` in `stdsutil.nest`
- fixed `sort` in `stdsequtil.nest` sometimes causing a crash

---

## 0.6.2

### Nest

**Changes**

- now you can have new lines before and after the expression in loops

### Fixes

- now functions defined in the main file access the variable of the file they were defined and not the file where they were called
- fixed maps not always decreasing the reference count of the keys when destroyed
- slight performance improvements
- try-catch no longer leaves values behind
- fixed if expression not setting a value when a statement does not return one

---

## 0.6.1

### Nest

**Changes**

- added the '!!' operator to throw an error and removed the `throw` function from `stderr.nest`
- added the try-catch statement but kept the `try` function as it gives more details on the error
- added some optimizations to the bytecode

**Bug fixes**

- fixed the `try` function not working
- fixed passing a non-existent file causing a crash
- fixed the x86 installer not showing the `add to PATH` option

---

## 0.6.0

### Nest

**Installer**

- you can now add Nest to PATH through the installer, the variable is removed if Nest is uninstalled and is not duplicated

**Changes**

- added the `stderr.nest` module, the 'throw' and 'try' functions are temporary as they will be substituted with a special syntax
- added the `\e` escape sequence that is equivalent to `\x1b` and can be used for ANSI escape codes such as `\e[33m` for the color yellow
- changed `~=` to `:=`
- the message displayed when `-c` is used incorrectly is now `Invalid usage of the option: -c` instead of `Invalid option: -c`
- swapped the start and the step in the range operator, it now is `[step] <start> -> <stop>`
- changed the behaviour of `-` for maps and vectors, now it will return the map or the vector to which it was applied instead of `true` or `false`


**Bug fixes**

- now when printing the call traceback of a function the whole call expression is highlighted instead of only the first letter
- fixed functions called from C libraries not being able to access other variables in their module
- fixed `..` in for loops and switch statements causing an infinite loop
- fixed a crash that could occur when a library called the function `nst_call_func`
- now when using a step of 0 with the range operator an error is thrown
- fixed circular references not throwing an error when encountered
- fixed some crashes related to error handling and reporting
- now Nest should use less memory when many libraries are imported

---

## 0.5.2

### Nest

**Changes**

- `Byte` objects can now be used as map keys
- the `==` operator can now be used with maps
- maps are now slightly faster when re-inserting the items (this happens when an item is dropped or the map changes the internal size)
- clearer explaination in the help message for `-O0` and `-O1`

**Bug fixes**

- fixed `..` statement causing the for-as loop to not call the `_advance_` function

---

## 0.5.1

### Nest

**Changes**

- added the functions `filter` and `contains` to `stdsequtil.nest`
- added the constant `VERSION` to `stdsys.nest`
- now the standard modules drop all the default variables
- now the error messages are colorful and this can be disabled with the `-m` or `--monochrome` flag in the command
- now the error messages for the wrong type of a C function tells the argument index starting from 1 instead of 0

**Bug fixes**

- fixed a crash that occurred when an argument of the wrong type was fed to a C function
- fixed a crash that occurred when comparing self-referencing arrays or vectors, now if a cycle is detected the equality is automatically false
- fixed error messages displaying in the wrong order when an error occurred while a nest module was tokenized, parsed or optimized

---

## 0.5.0

### Command improvements

- standardized command-line arguments parsing according to GNU specifications
  - `--` marks the end of the arguments
  - you can combine multiple one-letter options under one dash (such as `-bf`)
- added `-f` or `--force-execution` option to force the execution of the program when `-t`, `-a` or `-b` are used
- added `-O0` to `-O3` command line arguments to specify the optimization level, if specified more than one only the last one counts
- added `-c` option that executes the next argument as nest code (`nest -c ">>> 'Hello, world!'"` prints `Hello, world!`)
- now you can specify more than one compilation insight flag (`-t`, `-a` or `-b`) meaning that the command `nest -ta file.nest` will print both the tokens and the abstract syntax tree

### Nest

**Changes**

- now maps maintain the order of the elements that were put in, this affects map iterators in `stditutil.nest` and the casting to string
- renamed `IOfile` to `IOFile` to match the naming convention of other types
- now real numbers are printed with up to 15 significant digits (before it was 6)
- real numbers are compared up to 15 significant digits (now `0.1 0.2 + 0.3 ==` is `true`)
- added documentation for `split` and `join` in `string_utilities_library.md`
- now vectors allow newlines after the comma
- now you can put the argument names of a function definition on separate lines without backslashes
- now the `repr` function in `stdsutil.nest` will accept any type of object
- now numbers can have a leading plus sign (`1 2 +3` and `1 2 3` are equivalent)
- added byte literal by putting `b` or `B` after an integer
- now you can use a byte literal when casting a string to a byte (`Byte :: 'A'` and `Byte :: '65b'` are equivalent)
- now bytes inside of maps, arrays or vectors will be printed with the literal instead of the character
- improved error messages for operators and function calls
- now `bytearray_to_str` in `stdsutil.nest` does not raise an error when there is a NUL byte in the array

**Bug fixes**

- fixed item dropping for maps that sometimes resulted in a crash
- fixed `_cwd_` having the wrong length
- functions from external modules now reference the module's global variables rather than the ones of the module they are imported to
- now `-9223372036854775808` is considered a valid integer literal
- fixed nest module imports sometimes failing
- now the error message for an index outside of bounds for a string displays the correct index
- fixed for loop not popping the iterator off the stack, this resulted in nested loops not working sometimes
- opening an invalid DLL now no longer opens a pop-up error window
- now all the functions in in `stdsutil.nest` end at the end of the string at not at the first NUL byte

**Other**

- added a VSCode plugin, to paste in `%USERPROFILE%\.vscode\extensions`
- updated the syntax highlighting for sublime text

---

## 0.4.1

### Nest

- added lambda expressions
- fixed error traceback on call stack causing a crash
- fixed for-as loop crashing when a non-iterator object was given
- you can once again index strings
- updated documentation

---

## 0.4.0

### Nest

- added the `stdsys.nest` standard library
- added the `stdsequtil.nest` standart library
- added `min`, `max`, `min_seq`, `max_seq`. `sum_seq`, `frexp`, `ldexp`, `map` and `clamp` functions to `stdmath.nest`
- added `iter_start`, `iter_is_done`, `iter_get_val` and `iter_advance` functions to `stditutil.nest`
- improved error tracing with C function calls
- improved single-line escapes in comment parsing, now an even number of backslashes correctly escapes the newline
- added octal character escapes in string literals

---

## 0.3.1

### Nest

- fixed `nst_map_drop`, now the reference is removed correctly
- fixed `_vars_` object deleted when it shouldn't

---

## 0.3.0

### Nest

- nest now compiles internally to bytecode
- fixed `_vars_` being deleted even when returned by a function
- added `_globals_`, in functions accesses the global variables, in the global scope is `null`
- greatly improved memory usage
- the program is now optimized
   - expressions with known values are evaluated
   - the bytecode optimizes the instructions used
   - built-in variables are swapped with their value if it is never changed
   - more error checking is performed before runtime
   - unreachable code is removed
- the `-v` option was renamed to `-V`
- added the `-?` option, equivalent to `-h` or `--help`
- added `-b` and `--bytecode` options to see the compilation result
- better printing for tokens and ast when using the `-t` or `-a` options
- from now on there will be a windows 32-bit build as well

---

## 0.2.2

### Nest

- `Byte` objects now support `+`, `-`, `*`, `/`, `%` and `^`
- `==` now works correctly on `Real` objects
- fixed a bug that caused a crash when trying to parse an expression

---

## 0.2.1

### Nest

- fixed `<`, `<=`, `>` and `>=` yeilding unexpected results when comparing reals and integers
- now `IOfile` objects throw errors with unsupported operations such as `read` on a file opened with `rb`
- increased performance

---

## 0.2.0

### Nest

- added the ability to import modules and call functions from C libraries
- now `-` will remove a key from a map, `{ 'a': 1, 'b': 2 } 'a' - --> true, the map is now { 'b': 2 }`
- added switch statement
- added `split` and `join` functions to `stdsutil.nest`
- added `mkdirs`, `rmdir_recursive`, `copy`, `rename`, `list_dir`, `list_dir_recursive`, `absolute_path` and `equivalent` functions to `stdfs.nest`
- added `_vars_`, a variable pointing to the dictionary with all the current variables
- fixed maps not changing their maximum size after increasing it
- now the current working directory changes when importing a module
- fixed `replace_substr` failing when replacing the first occurrence caused the string to be bigger than the original
- fixed not-closed multiline comments not resulting in a syntax error

---

## 0.1.4

### Nest

- added `Map` to `Str` cast
- added ability to pass a `Str` object to `cycle`, `zip`, `enumerate` and `reverse` in `stditutil.nest`
- added ability to pass a `Str` object to `choice`, in `stdrand.nest`
- added `repr` function to `stdsutil.nest`
- changed `yearday` to `year_day` and `weekday` to `week_day` for `date`, `clock_datetime` and `gmt_clock_datetime` in `stdtime.nest`
- fixed `year` being the same as `month` for `date`, `clock_datetime` and `gmt_clock_datetime` in `stdtime.nest`
- fixed parser parsing some expressions incorrectly
- fixed `move_fptr` and `get_fptr` requesting the incorrect number of arguments
- fixed math module not working
- fixed `shuffle` in `stdrand.nest` where an object could not remain in its position

---

## 0.1.3

### Nest

- added `stdmath.nest` library
- added `stditutils.nest` library (iterator utities)
- added `-:` operator for negation `-:a` is equal to `(0 a -)`
- added `@@` operator to call a function with no arguments: `10 @@func +` is equal to `10 (@func) +`
- added an optional `free_lib` function to C libraries to free any globally allocated memory
- modified `_cwd_`, now it shows the full path and not the relative path to the main file
- added a native `Array` or `Vector` to `Str` cast and vice versa
- fixed `flush` function in `stdio.nest` expecting two arguments but taking only one

---

## 0.1.2

### Nest

- now circular imports correctly show an error
- added `stdtime.nest` to the standard library
- added `flush` function to `stdio.nest`
- now `_cwd_` containst the full path and not the relative path

---

## 0.1.1

### Nest

- fixed maps halting when searching the index in rare occasions
- added a third character argument to ljust and rjust in stdsutil
- fixed `shuffle` in `stdrand.nest`

---

## 0.1.0

### Nest

First Release of Nest.
