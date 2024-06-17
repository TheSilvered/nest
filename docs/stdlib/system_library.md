# System library

## Importing

```nest
|#| 'stdsys.nest' = sys

-- To access _raw_exit and _DEBUG use the following
|#| '__C__:stdsys.cnest' = __sys
```

## Functions

### `@del_env`

**Synopsis:**

```nest
[name: Str] @del_env -> null
```

**Description:**

Deletes the environment variable `name`. If it does not exist the environment
is left unchanged and the function succeeds.

`name` cannot contain NUL characters (`\0`) and equals signs (`=`).

---

### `@exit`

**Synopsis:**

```nest
[code: Int?] @exit -> null
```

**Description:**

Ends the execution of the program and terminates with an exit code of `code`.
If `code` is `null` it defaults to `0`.

---

### `@get_addr`

**Synopsis:**

```nest
[object: Any] @get_addr -> Int
```

**Returns:**

The address of the object in memory.

---

### `@get_capacity`

**Synopsis:**

```nest
[container: Vector|Map] @get_capacity -> Int
```

**Returns:**

The number of elements that can still be inserted without the object expanding.

---

### `@get_cwd`

**Synopsis:**

```nest
[] @get_cwd -> Str
```

**Returns:**

Gets the current working directory.

---

### `@get_env`

**Synopsis:**

```nest
[name: Str] @get_env -> Str?
```

**Returns:**

The value of the environment variable `name`. If it does not exist, `null` is
returned instead.

`name` cannot contain NUL characters (`\0`) and equals signs (`=`).

---

### `@get_ref_count`

**Synopsis:**

```nest
[object: Any] @get_ref_count -> Int
```

**Returns:**

The reference count of `object`.

---

### `@hash`

**Synopsis:**

```nest
[object: Any] @hash -> Int
```

**Returns:**

The hash of the object. If the object is not hashable, `-1` is returned.

---

### `@set_cwd`

**Synopsis:**

```nest
[cwd: Str] @_set_cwd -> null
```

**Description:**

Sets the current working directory to `cwd`.

---

### `@set_env`

**Synopsis:**

```nest
[name: Str, value: Str, overwrite: Bool?] @set_env -> null
```

**Description:**

Sets the value of the `name` environment variable to be `value`. Both `name`
and `value` cannot contain NUL characters and `name` cannot contain an equals
sign (`=`).

If `overwrite` is `false` and in the environment `name` is already defined, its
value is not changed. Overwrite is `true` by default.

---

### `@system`

**Synopsis:**

```nest
[cmd: Str] @system -> Int
```

**Description:**

Executes `cmd` in a sub-shell.

**Returns:**

The return value is the exit status of the shell.

On Linux is always the exit status, on Windows, it depends on the default
shell.

---

### `@_raw_exit`

**Synopsis:**

```nest
[code: Int?] @_raw_exit -> null
```

**Description:**

Similar to [`exit`](#exit), but instead of throwing a special error, the C
function is called.

The exit code is `code`. If `code` is `null` it defaults to `0`.

This function cannot be accessed by importing the normal system library, you
need to [import the C source](#importing) instead.

!!!warning
    This function exits the process early rather than ending it with a special
    error that lets Nest free properly any allocated memory.

---

## Constants

### `ENDIANNESS`

The endianness of the compiled executable, can be either `little` or `big`.

---

### `PLATFORM`

The current platform, can be either `windows` or `linux`.

---

### `SUPPORTS_COLOR`

Whether the console supports ANSI escape sequences.

---

### `VERSION`

The current version of Nest as a string such as `'0.15.0 x64'`.

---

### `_DEBUG`

`true` if running a debug build of Nest and `false` otherwise.

This constant cannot be accessed by importing the normal system library, you
need to [import the C source](#importing) instead.
