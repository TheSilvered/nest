# 0.14.0

## Nest

### Additions

- added `IEND` constant to `stditutil.nest`
- added `map_i`, `slice_i` and `filter_i` to `stdsequtil.nest`

### Changes

- now `itu.new_iterator` takes three arguments instead of four
- now `itu.repeat` will repeat forever if `count` is negative
- now `itu.cycle` will also accept `Iter` objects
- removed `iter_is_done` from `stditutil.nest`
- now the iterator of a for-as loop is implicitly casted to `Iter`
- now `\U` in strings will only accept 6 hex characters instead of 8, since U+10FFFF is the highest codepoint accepted
- now string length and indexing will match the characters and not the bytes (ex. `'àèì'.1` now is `'è'` and before was `'\xa0'`)
  - removed `to_iter`, `get_len` and `get_at` from `stdcodecs.nest` since they are no longer needed
- removed `ljust` and `rjust` from `stdsutil.nest` in favour of `justify`
  - with `justify` a positive length means justify left and a negative one means justify right
- now `su.repr` will display in `\xhh`, `\uhhhh` or `\uhhhhhh` unprintable characters (control characters, non-characters, surrogates)
- fixed `>>>` that would not fully print a string that contains a NUL character

### Bug fixes

- reworked garbage collector to work more reliably and remove many types of crashes
- fixed error printing sometimes adding an empty line after a fully red one
- fixed incorrect behaviour of `sequ.slice` on some occasions
- fixed a crash that occurred when an error was thrown inside a function in `sequ.rscan`
- fixed `json.load_f` and `json.dump_f` not working correctly with UTF-8 file names on Windows
- fixed Nest not finding files with non-ASCII charcters (both command-line and libraries) on Windows
- fixed `su.is_charset` returning an incorrect result on some non-ASCII strings (ex. `'à' 'èĠ' @su.is_charset` now returns `false`, before it would return `true`)

## C API

### Additions

- added `p_next` field to `Nst_Obj`
- added object pools to improve performance
- added `_Nst_obj_free` to free an object's memory or add it to its type's pool
- added `Nst_TYPE_STR` to get the string of a type object
- added `CONT_TYPE` and `Nst_ContType` for types of containers
- added `IEnd` field to `Nst_TypeObjs` and `IEnd_iend` to `Nst_Consts`
- added `Nst_true_ref`, `Nst_false_ref`, `Nst_null_ref`, `Nst_iend` and `Nst_iend_ref` to `global_consts.h`
- added `Nst_FLAG` macro to create the value of the flag of an object
- added `indexable_str` and `true_len` fields to `Nst_StrObj`.
- added `Nst_string_new_len` to create strings with a known character length
- added `Nst_string_get_next_ch` to iterate over a string's characters without generaing the `indexable_str` field
- added `Nst_FLAG_STR_IS_ASCII`, `Nst_FLAG_STR_INDEX_16`, `Nst_FLAG_STR_INDEX_32` and `Nst_FLAG_STR_CAN_INDEX` variants to `Nst_StrFlags`

### Changes

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

### Bug fixes

- fixed `Nst_is_non_character` returning `true` on some valid characters
