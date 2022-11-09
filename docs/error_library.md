# Error library (`stderr.nest` - `err`)

## Functions

### `[func: Func, args: Array|Vector] @try`

Calls `func` giving it `args` as the arguments.
It returns a map containing `value`, `error` and `traceback`.
`value` is the value returned by the function or `null` if an error occurred.
`error` is `null` if everything was successful otherwise it is a map containing
`name`, `message` and `pos`.
`traceback` is an array of positions that trace back the cause of the error.

A position is a map that contains 3 keys:
- `file` is the path of the file that the position refers to
- `start` is an array of length 2 with the first number being the line and the
  second being the column of the start of the interested expression
- `end` is also an array with the line and column of the end of the expression

Note that the lines and the columns start from 0.

### `[name: Str, message: Str] @throw`

This function throws an error with the name set as `name` and the message set as
`message`.

For example this code:
```
# example.nest
|#| 'stderr.nest' = err
'Random Error' 'I just felt like throwing an error' @err.throw
```

whould produce this output:
```
File "example.nest" at line 3:
 3 | 'Random Error' 'I just felt like throwing an error' @err.throw
Random Error - I just felt like throwing an error
```

Here `Random Error` is the name and `I just felt like throwing an error` is the
message.

## Constants

### `SYNTAX_ERROR`

This is the string used by Nest when throwing syntax errors.

### `VALUE_ERROR`

This is the string used by Nest when throwing value errors.

### `TYPE_ERROR`

This is the string used by Nest when throwing type errors.

### `CALL_ERROR`

This is the string used by Nest when throwing call errors.

### `MEMORY_ERROR`

This is the string used by Nest when throwing memory errors.

### `MATH_ERROR`

This is the string used by Nest when throwing math errors.

### `IMPORT_ERROR`

This is the string used by Nest when throwing import errors.

> The messages used inside the interpreter are not available as they are not
> pre-allocated.
