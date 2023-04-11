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
