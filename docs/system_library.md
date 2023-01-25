# System library (`stdsys.nest` - `sys`)

## Functions

### `[cmd: Str] @system`

Executes `cmd` in a sub-shell. The return value is the exit status of the shell.

On Linux is always the exit status, on Windows, it depends on the default shell.

### `[code: Int?] @exit`

Ends the execution of the program and terminates with an exit code of `code`.  
If `code` is `null` it defaults to `0`.

### `[name: Str] @getenv`

The same as the homonym C function. It returns the environment variable `name`,
if it does not exist, `null` is returned.

### `[object: Any] @get_ref_count`

Returns the reference count of `object`

### `[object: Any] @get_addr`

Returns the address of the object in memory.

### `[object: Any] @hash`

Returns the hash of the object, used in maps.

## Constants

### `VERSION`

The current version of Nest in form of a string.
