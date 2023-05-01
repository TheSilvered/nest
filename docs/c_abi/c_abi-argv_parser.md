# `argv_parser.h`

The header that contains the function for command-line argument parsing.

## Functions

### `nst_parse_args`

**Synopsis**:

```better-c
i32 _nst_parse_args(i32 argc, i8 **argv,
                    bool *print_tokens,
                    bool *print_ast,
                    bool *print_bytecode,
                    bool *force_execution,
                    bool *monochrome,
                    bool *force_cp1252,
                    bool *no_default,
                    i32  *opt_level,
                    i8  **command,
                    i8  **filename,
                    i32  *args_start)
```

**Description**:

Parses the command-line arguments given to the program.

**Arguments**:

- `[in] argc` the number of arguments
- `[in] argv` the array of the arguments
- `[out] print_tokens` the `-t` or `--tokens` option
- `[out] print_ast` the `-a` or `--ast` option
- `[out] print_bytecode` the `-b` or `--bytecode` option
- `[out] force_execution` the `-f` or `--force-execution` option
- `[out] monochrome` the `-m` option
- `[out] force_cp1252` the `--cp1252` option
- `[out] no_default` the `--no-default` option
- `[out] opt_level` the `-O<lvl>` option
- `[out] command` the value after `-c`
- `[out] filename` the file name passed
- `[out] args_start` the index at which the arguments in `_args_` begin

**Return value**:

- `-1` is returned if the parsing failed
- `0` is returned if the parsing succeded and the program can run
- `1` is returned if the parsing succeded and the program should stop because a
  message was printed.

---

### `nst_supports_color`

**Synopsis**:

```better-c
bool nst_supports_color(void)
```

**Return value**:

Returns `true` if the output supports color and `false` if it does not. When the
`--monochrome` flag is used, this function returns `true`.

---

### `_nst_wargv_to_argv`

_This function is available **only on Windows**_

**Synopsis**:

```better-c
bool _nst_wargv_to_argv(int       argc,
                        wchar_t **wargv,
                        i8     ***argv,
                        i8      **argv_content)
```

**Description**:

This function turns Unicode Windows arguments into normal C strings encoded in
UTF-8. Both `argv` and `argv_content` need to be freed.

**Arguments**:

- `[in] argc`: the number of command-line arguments
- `[in] wargv`: the Unicode arguments
- `[out] argv`: the pointer where to store the arguments, the memory is
  allocated by the function
- `[out] argv_content`: the pointer where to store the contents of the arguments,
  the memory is allocated by the function

**Return value**:

The function returns `true` on success and `false` on failure.

---

### `_nst_set_console_mode`

_This function is available **only on Windows**_

**Synopsis**:

```better-c
void _nst_set_console_mode(void)
```
**Description**:

Sets the console output to be UTF-8 and to intercept ANSI escape sequences.
