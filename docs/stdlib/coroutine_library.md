# Coroutine library

## Importing

```nest
|#| 'stdco.nest' = co
```

## Functions

### `@call`

**Synopsis:**

```nest
[co: Coroutine, args: Array|Vector|null] @call -> Any
```

**Description:**

Calls the coroutine passing the arguments given. If the coroutine is paused the
arguments are ignored, if it is suspended or has ended the arguments are
required. If the coroutine is running an error is thrown since coroutines
cannot be recursive.

**Arguments:**

- `co`: the coroutine to be called
- `args`: the arguments passed to the function of the coroutine, this argument
  is ignored if it is paused

**Returns:**

Either the value passed to [`pause`](coroutine_library.md#pause) when the
coroutine was paused or the value that was returned by the function.

**Example:**

```nest
|#| 'stdco.nest' = co

## [
    1 @co.pause
    => 2
] @co.create = my_corotuine

>>> (my_coroutine {,} @co.call '\n' ><) --> 1
>>> (my_coroutine @co.call '\n' ><) --> 2
```

---

### `@create`

**Synopsis:**

```nest
[function: Func] @create -> Coroutine
```

**Description:**

Creates a `Coroutine` object from `function`.

**Arguments:**

- `function`: the function to turn into a coroutine, it must be written in Nest

**Returns:**

A new `Coroutine` object. An error is thrown if the given function is written
in C or C++.

**Example:**

```nest
|#| 'stdco.nest' = co

#func [
    >>> 'Hello,'
    null @co.pause
    >>> ' world!\n'
]

func @co.create = func_co
```

---

### `@generator`

**Synopsis:**

```nest
[co: Coroutine] @generator -> Iter
```

**Description:**

Creates an `Iter` object given a coroutine and each time `pause` is called, the
return value is yielded by the iterator. The return value of the function is
ignored.

The function of the coroutine must take exactly one argument that is the
coroutine itself.

**Arguments:**

- `co`: the coroutine to turn into an iterator

**Returns:**

The iterator created from the coroutine.

**Example:**

```nest
|#| 'stdco.nest' = co

#f self [
    1 @co.pause
    2 @co.pause
    3 @co.pause
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

---

### `@get_state`

**Synopsis:**

```nest
[co: Coroutine] @get_state -> Int
```

**Arguments:**

- `co`: the coroutine to get the state of

**Returns:**

The ID of the current state of the coroutine. To get the state as a string use
the return value of this function to index the [STR_STATE](#str_state) map.

**Example:**

```nest
|#| 'stdco.nest' = co

#print_state coroutine [
    >>> (co.STR_STATE.(coroutine @co.get_state) '\n' ><)
]

#f self [
    self @print_state --> running
    @co.pause
    self @print_state --> running
]

f @co.create = f_co

f_co @print_state --> suspended
f_co {f_co} @co.call
f_co @print_state --> paused
f_co @co.call
f_co @print_state --> ended
```

---

### `@pause`

**Synopsis:**

```nest
[return_value: Any] @pause -> null
```

**Description:**

Pauses the current coroutine and makes it return `return_value`. If the
coroutine was not called with `call` or if it is used outside of a coroutine an
error is thrown.

**Arguments:**

- `return_value`: the value that is returned by `call` when the coroutine
  pauses.

**Returns:**

`null`. `return_value` will only make the paused coroutine return the given
value, not the `pause` function itself.

**Example:**

```nest
|#| 'stdco.nest' = co

#func [
    >>> 'Hello,'
    10 @co.pause
    >>> ' world!\n'
]
func @co.create = func_co

func_co {,} @co.call = value_returned --> Hello,
func_co @co.call --> world!
>>> (value_returned '\n' ><) --> 10
```

---

### `@_get_co_type_obj`

**Synopsis:**

```nest
[] @_get_co_type_obj -> Type
```

**Returns:**

The type object of coroutines.

---

## Constants

### `Coroutine`

The coroutine type.

---

### `STATE`

The possible states of a coroutine.

| Name        | Meaning                                                  |
| ----------- | -------------------------------------------------------- |
| `suspended` | the coroutine was never called                           |
| `running`   | the coroutine is running                                 |
| `paused`    | the coroutine was paused                                 |
| `ended`     | the coroutine ended either with an error or successfully |

---

### `STR_STATE`

The inverse of `STATE`, the keys are the possible states and the values are
their corresponding names.
