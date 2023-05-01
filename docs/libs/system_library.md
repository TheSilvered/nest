# System library

## Importing

```nest
|#| 'stdsys.nest' = sys

-- To access _raw_exit use the following
|#| '__C__:stdsys.cnest' = __sys
```

## Functions

### `@exit`

**Synopsis**:

`[code: Int?] @exit -> null`

**Description**:

Ends the execution of the program and terminates with an exit code of `code`.  
If `code` is `null` it defaults to `0`.

---

### `@getenv`

**Synopsis**:

`[name: Str] @getenv -> Str?`

**Return value**:

The function returns the environment variable `name`, if it does not exist,
`null` is returned.

---

### `@get_addr`

**Synopsis**:

`[object: Any] @get_addr -> Int`

**Return value**:

Returns the address of the object in memory.

---

### `@get_ref_count`

**Synopsis**:

`[object: Any] @get_ref_count -> Int`

**Return value**:

Returns the reference count of `object`

---

### `@hash`

**Synopsis**:

`[object: Any] @hash -> Int`

**Return value**:

Returns the hash of the object, used in maps. If the object is unhashable, -1 is
returned.

---

### `@system`

**Synopsis**:

`[cmd: Str] @system -> Int`

**Description**:

Executes `cmd` in a sub-shell.

**Return value**:

The return value is the exit status of the shell.  
On Linux is always the exit status, on Windows, it depends on the default shell.

---

### `@_get_cwd`

**Synopsis**:

`[] @_get_cwd -> Str`

**Description**:

Gets the current working directory. If `_set_cwd` was never called, this is
equivalent to `_cwd_`.

---

### `@_set_cwd`

**Synopsis**:

`[cwd: Str] @_set_cwd -> null`

**Description**:

Sets the current working directory. The changes do not reflect in `_cwd_`.

---

### `@_raw_exit`

**Synopsis**:

`[code: Int?] @_raw_exit -> null`

**Description**:

Similar to [`exit`](#exit), but instead of throwing a special error, the C
function is called.  
This function cannot be accessed by importing the normal system library, you
need to import the C source instead.

!!!warning
    This function exits the process early rather than ending it with a special
    error that lets Nest free properly any allocated memory.

---

## Constants

### `PLATFORM`

The current platform, can be either `windows` or `linux`.

---

### `SUPPORTS_COLOR`

Whether the console supports ANSI escape sequences.

---

### `VERSION`

The current version of Nest in form of a string such as `'0.12.0 x64'`.
