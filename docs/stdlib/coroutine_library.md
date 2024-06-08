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
required. If the coroutine is already running an error is thrown.

**Arguments:**

- `co`: the coroutine to be called
- `args`: the arguments passed to the function of the coroutine, this argument
  is the return value of `yield` when the coroutine is resumed

**Returns:**

Either the value passed to [`yield`](coroutine_library.md#yield) when the
coroutine was paused or the value that was returned by the function.

**Example:**

```nest
|#| 'stdco.nest' = co

## [
    1 @co.yield
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
    null @co.yield
    >>> ' world!\n'
]

func @co.create = func_co
```

---

### `@generator`

**Synopsis:**

```nest
[co: Coroutine, args: Array|Vector|null] @generator -> Iter
```

**Description:**

Creates an `Iter` object given a coroutine and each time `yield` is called, the
return value is yielded by the iterator. The return value of the function is
ignored.

The `args` array is used to call the function of the coroutine. If it is `null`
an empty array is passed. Passing more arguments than the function accepts will
result in a `Call Error`.

**Arguments:**

- `co`: the coroutine to turn into an iterator
- `args`: the arguments to pass to the coroutine

**Returns:**

The iterator created from the coroutine.

**Example:**

```nest
|#| 'stdco.nest' = co

#f a b c [
    a @co.yield
    b @co.yield
    c @co.yield
]

f @co.create = f_co

... f_co {1, 2} @co.generator := i [
    >>> (i '\n' ><)
]
```

this program outputs:

```nest
1
2
null
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
    @co.yield
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

### `@yield`

**Synopsis:**

```nest
[return_value: Any] @yield -> null
```

**Description:**

Pauses the current coroutine and makes it return `return_value`. If the
coroutine was not called with `call` or if it is used outside of a coroutine an
error is thrown.

**Arguments:**

- `return_value`: the value that is returned by `call` when the coroutine
  pauses.

**Returns:**

The arguments passed to `call` to resume the coroutine. If no arguments are
passed `null` is returned otherwise they are returned inside an array. The
number of arguments passed is not limited by the number of arguments the
function accepts. `return_value` will only modify the return value of the
coroutine when it is yielded, not the return value of `yield` itself.

**Example:**

```nest
|#| 'stdco.nest' = co

#func [
    >>> 'Hello,'
    10 @co.yield = {name}
    >>> ' \(name)!\n'
]
func @co.create = func_co

func_co @co.call = value_returned --> Hello,
func_co {'Nest'} @co.call --> Nest!
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
