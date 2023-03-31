# Error library

## Importing

```nest
|#| 'stderr.nest' = err
```

## Functions

### `@try`

**Synopsis**:

`[func: Func, args: Array|Vector] @try -> Map`

**Description**:

Calls `func` passing `args` as the arguments and catching any erros that occur.

**Arguments**:

- `func`: the function to be called
- `args`: the arguments to be passed to the function

**Return value**:

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

!!!note
  the lines start from 0 and the columns from 1 and the end position is inclusive.

**Example**:

The file `example.nest` contains this code:
```nest
|#| 'stderr.nest' = err

#f ["Random Error" !! "hello this is a message"]
#g [@f]
>>> (g {,} @err.try)
```

The formatted output looks like this:

```nest
{
    'value': null,
    'error': {
        'name': 'Random Error',
        'message': 'hello this is a message',
        'pos': {
            'file': 'example.nest',
            'start': { 2, 4 },
            'end': { 2, 46 }
        }
    },
    'traceback': {
        {
            'file': 'example.nest',
            'start': { 3, 4 },
            'end': { 3, 5 }
        }
    }
}
```

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
> created in advance.
