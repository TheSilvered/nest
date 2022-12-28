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

### `[co: Coroutine] @generator`

Creates an `Iter` object given a coroutine and each time `pause` is called, a
new value is yielded by the iterator. The return value is ignored.

The function of the coroutine must take exactly one argument that is the
coroutine itself.

```text
|#| 'stdco.nest' = co

#f self [
    self 1 @co.pause
    self 2 @co.pause
    self 3 @co.pause
]

f @co.create = f_co

... f_co @co.generator := i [
    >>> (i '\n' ><)
]
```

this program outputs:

```text
1
2
3
```

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
