# `argv_parser.h`

Command-line arguments parser.

## Authors

TheSilvered

---

## Structs

### `Nst_CLArgs`

**Synopsis:**

```better-c
typedef struct _Nst_CLArgs {
    bool print_tokens, print_ast, print_bytecode;
    bool force_execution;
    Nst_EncodingID encoding;
    bool no_default;
    i32 opt_level;
    i8 *command, *filename;
    i32 args_start;
    i32 argc;
    i8 **argv;
} Nst_CLArgs
```

**Description:**

A structure representing the command-line arguments of Nest.

!!!note
    `command` and `filename` cannot both be set.

**Fields:**

- `print_tokens`: whether the tokens of the program should be printed
- `print_ast`: whether the AST of the program should be printed
- `print_bytecode`: whether the bytecode of the program should be printed
- `force_execution`: whether to execute the program when `print_tokens`,
  `print_ast` or `print_bytecode` are true
- `encoding`: the encoding of the file to open, ignored if it is passed through
  the command
- `no_default`: whether to initialize the program with default variables such as
  `true`, `false`, `Int`, `Str` etc...
- `opt_level`: the optimization level of the program 0 through 3
- `command`: the code to execute passed as a command line argument
- `filename`: the file to execute
- `args_start`: the index where the arguments for the Nest program start

---

## Functions

### `Nst_cl_args_init`

**Synopsis:**

```better-c
void Nst_cl_args_init(Nst_CLArgs *args, i32 argc, i8 **argv)
```

**Description:**

Initializes a [`Nst_CLArgs`](c_api-argv_parser.md#nst_clargs) struct with
default values.

**Parameters:**

- `args`: the struct to initialize
- `argc`: the number of arguments passed to main
- `argv`: the array of arguments passed to main

---

### `_Nst_cl_args_parse`

**Synopsis:**

```better-c
i32 _Nst_cl_args_parse(Nst_CLArgs *cl_args)
```

**Description:**

Parses command-line arguments.

**Parameters:**

- `cl_args`: the struct where to put the parsed arguments, it must be
  initialized with [`Nst_cl_args_init`](c_api-argv_parser.md#nst_cl_args_init)

**Returns:**

`-1` on failure, `0` on success where the program can continue, `1` on success
when the program should stop because an info message was printed.

---

### `Nst_supports_color`

**Synopsis:**

```better-c
bool Nst_supports_color(void)
```

**Returns:**

`true` if ANSI escapes are supported on the current console and `false`
otherwise.

---

### `_Nst_supports_color_override`

**Synopsis:**

```better-c
void _Nst_supports_color_override(bool value)
```

**Description:**

Ovverrides the value returned by
[`Nst_supports_color`](c_api-argv_parser.md#nst_supports_color).

---

### `_Nst_wargv_to_argv`

**Synopsis:**

```better-c
bool _Nst_wargv_to_argv(int argc, wchar_t **wargv, i8 ***argv)
```

**Description:**

**WINDOWS ONLY** Re-encodes Unicode arguments to UTF-8.

**Parameters:**

- `argc`: the length of `wargv`
- `wargv`: the arguments given
- `argv`: the pointer where the re-encoded arguments are put

**Returns:**

`true` on success and `false` on failure. No error is set.

---

### `_Nst_console_mode_init`

**Synopsis:**

```better-c
void _Nst_console_mode_init(void)
```

**Description:**

**WINDOWS ONLY** Initializes the console.
