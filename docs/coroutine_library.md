# Coroutine library

## Importing

```nest
|#| 'stdco.nest' = co
```

## Functions

### `[function: Func] @create`

Creates a `Coroutine` object from `function`.

```nest
|#| 'stdco.nest' = co

#f self [
    >>> 'Hello,'
    self null @co.pause
    >>> ' world!\n'
]

f @co.create = f_co
```

### `[args: Array|Vector, co: Coroutine] @call`

Calls the function of `co` with `args`. If the coroutine is paused, the
arguments are completely ignored and if it is running, the function throws an
error.

```nest
{ f_co } f_co @co.call --> 'Hello,'
<{}> f_co @co.call --> ' world!\n'
```

### `[co: Coroutine, return_value: Any] @pause`

Pauses `co` which must be the top function on the call stack and must have been
called with `call`, and returns `return_value`

```nest
|#| 'stdco.nest' = co

#print_state coroutine [
    >>> (co.STR_STATE.(coroutine @co.get_state) '\n' ><)
]

#f self [
    self @print_state --> running
    self null @co.pause
    self @print_state --> running
]

f @co.create = f_co

f_co @print_state --> suspended
{ f_co } f_co @co.call
f_co @print_state --> paused
<{}> f_co @co.call
f_co @print_state --> ended
```

### `[co: Coroutine] @get_state`

Returns the current state of the coroutine.  
To get the name of the state use the [`STR_STATE`](#str_state) map.  
To check the state use the [`STATE`](#state) map.

```nest
|#| 'stdco.nest'

-/ Prints the current state of a given coroutine /-
#print_state coroutine [
    coroutine @co.get_state = co_state
    co.STR_STATE.(co_state) = state_name
    >>> (state_name '\n' ><)
]

-/ Checks if the coroutine has ended /-
#has_ended coroutine [
    coroutine @co.get_state = co_state
    => co.STATE.ended co_state ==
]
```

### `[co: Coroutine] @generator`

Creates an `Iter` object given a coroutine and each time `pause` is called, a
new value is yielded by the iterator. The return value is ignored.

The function of the coroutine must take exactly one argument that is the
coroutine itself.

```nest
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

```nest
1
2
3
```

## Constants

### `Coroutine`

The coroutine type.

### `STATE`

The possible states of a coroutine.

| Name        | Meaning                                                  |
| ----------- | -------------------------------------------------------- |
| `suspended` | the coroutine was never called                           |
| `running`   | the coroutine is running                                 |
| `paused`    | the coroutine was paused                                 |
| `ended`     | the coroutine ended either with an error or successfully |

### `STR_STATE`

The inverse of `STATE`, the keys are the possible states and the values are
their corresponding names.
