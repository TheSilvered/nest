# Coroutine library (`stdco.nest` - `co`)

## Functions

### `[function: Func] @create`

Creates a `Coroutine` object from `function`.

### `[args: Array|Vector, co: Coroutine] @call`

Calls the function of `co` with `args`.

### `[co: Coroutine, return_value: Any] @pause`

Pauses `co` which must be the top function on the call stack and must have been
called with `call`, and returns `return_value`

### `[co: Coroutine] @get_state`

Returns the current state of the coroutine. The return value is part of the
`STATE` map.

## Constants

### `Coroutine`

The coroutine type.

### `STATE`

The possible states of a coroutine.

| Name      | Meaning                                                  |
| --------- | -------------------------------------------------------- |
| suspended | the coroutine was never called                           |
| running   | the coroutine is running                                 |
| paused    | the coroutine was paused                                 |
| ended     | the coroutine ended either with an error or successfully |

### `STR_STATE`

The inverse of `STATE`, the keys are the possible states and the values are
their corresponding names.
