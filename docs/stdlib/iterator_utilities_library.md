# Iterator utilities library

## Importing

```nest
|#| 'stditutil.nest' = itu
```

## Functions

### `@chain`

**Synopsis:**

```nest
[sequence: Iter|Array|Vector|Str] @chain -> Iter
```

**Returns:**

An iterator that chains the objects inside of the elements inside of the
sequence.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{'Hi!', 1 -> 4} @itu.chain --> 'H', 'i', '!', 1, 2, 3
'Hi!' @itu.chain --> 'H', 'i', '!' this is the same as {'H', 'i', '!'} @itu.chain
'Hi!' @itu.enumerate @itu.chain --> 0, 'H', 1, 'i', 2, '!'
```

---

### `@count`

**Synopsis:**

```nest
[start: Int, step: Int?] @count -> Iter
```

**Returns:**

An iterator that counts indefinitely from `start`, advancing by `step`. If
`step` is `null` it defaults to `0`.

**Example:**

```nest
|#| 'stditutil.nest' = itu

0 2 @itu.count --> 0, 2, 4, 6, 8, ...
```

---

### `@cycle`

**Synopsis:**

```nest
[sequence: Str|Array|Vector|Iter] @cycle -> Iter
```

**Returns:**

An iterator that cycles through the elements of the given object restarting
from the first when the sequence ends.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.cycle --> 'H', 'i', '!', 'H', 'i', ...
```

---

### `@enumerate`

**Synopsis:**

```nest
[iterator: Str|Array|Vector|Iter, start: Int?, step: Int?, invert_order: Bool?] @enumerate -> Iter
```

**Arguments:**

- `iterator`: the iterator to enumerate
- `start`: the starting number for the numeration, the function will still
  start from the first element
- `step` the step of the enumeration, the function will still step through all
  the elements

**Returns:**

An iterator that returns a 2-element array where the first element is the
object returned by the `iterator` and the second is the index of the current
iteration. When `invert_order` is set to a truthy value the first element is
the object and the second the iteration count.

`start` by default is set to `0` and `step` to `1` but this behaviour can be
changed by passing the additional arguments.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.enumerate --> {0, 'H'}, {1, 'i'}, {2, '!'}
'Hi!' 5 -1 @itu.enumerate --> {5, 'H'}, {4, 'i'}, {3, '!'}
'Hi!' @itu.reversed @itu.enumerate --> {0, '!'}, {1, 'i'}, {2, 'H'}
'Hi!' 0 1 true @itu.enumerate --> {'H', 0}, {'i', 1}, {'!', 2}
```

---

### `@iter_is_done`

**Synopsis:**

```nest
[iter: Iter] @iter_is_done
```

**Description:**

Calls the `_is_done_` function of an iterator.

---

### `@iter_get_val`

**Synopsis:**

```nest
[iter: Iter] @iter_get_val
```

**Description:**

Calls the [`get_val`](iterator_utilities_library.md#new_iterator) function of
an iterator.

---

### `@iter_start`

**Synopsis:**

```nest
[iter: Iter] @iter_start
```

**Description:**

Calls the [`start`](iterator_utilities_library.md#new_iterator) function of an
iterator.

---

### `@keys`

**Synopsis:**

```nest
[map: Map] @keys -> Iter
```

**Returns:**

An iterator which goes through all the keys in a map.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{'key_1': 1, 'key_2': 2} @itu.keys --> 'key_1', 'key_2'
{'key_2': 2, 'key_1': 1} @itu.keys --> 'key_2', 'key_1'
```

---

### `@new_iterator`

**Synopsis:**

```nest
[start: Func, get_val: Func, data: Any] @new_iterator -> Iter
```

**Description:**

Creates a new iterator object that uses custom functions. Any of the given
functions must take exactly one parameter that will be `data`.

`start` should initialize data to start the iterator and is always called
before iterating.

`get_val` should return the current value of the iterator. To signal that the
iterator has ended this function should return
[`IEND`](iterator_utilities_library.md#iend). If it may return
[`IEND`](iterator_utilities_library.md#iend) before reaching the actual end, it
should be replaced by `null`.

**Arguments:**

- `start`: the function called when the iterator is started
- `is_done`: the function called before any iteration to check if it has ended
- `get_val`: the function called to get the value of each iteration
- `data`: the object passed to the functions when they get called

**Returns:**

The new custom iterator.

---

### `@repeat`

**Synopsis:**

```nest
[object: Any, times: Int] @repeat -> Iter
```

**Returns:**

An iterator that repeats `object` the number of times specified by `times`.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' 3 @itu.repeat --> 'Hi!', 'Hi!', 'Hi!'
```

---

### `@reversed`

**Synopsis:**

```nest
[sequence: Str|Array|Vector] @reversed -> Iter
```

**Returns:**

An iterator which goes through the elements of a sequence in reverse order,
from the last to the first.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.reversed --> '!', 'i', 'H'
```

---

### `@values`

**Synopsis:**

```nest
[map: Map] @values -> Iter
```

**Returns:**

An iterator which goes through all the keys in a map.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{'key_1': 1, 'key_2': 2} @itu.values --> 1, 2
{'key_2': 2, 'key_1': 1} @itu.values --> 2, 1
```

---

### `@zip`

**Synopsis:**

```nest
[seq_1: Array|Vector|Str|Iter, seq_2: Array|Vector|Str|Iter] @zip -> Iter
[seq: Array|Vector.Array|Vector|Str|Iter] @zip -> Iter
```

**Returns:**

The first kind of this function accepts two arguments and returns an iterator
that zips them together pairing the objects of `seq_1` with the objects of
`seq_2` at the same index and that ends when the shortes of the two sequences
ends.

The second kind only accepts the first argument (the second must be `null`
since there are no actual overloads in Nest) and returns an iterator that
zips together all the sequences inside `seq` putting all the arguments with the
same index in an array. The iterator ends when the shortest of all the
sequences has ended.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' {1, 2, 3} @itu.zip --> {'H', 1}, {'i', 2}, {'i', 3}
{'Hi!', 1 -> 3, <{9, 8, 7, 6}>} @itu.zip --> {'H', 1, 9}, {'i', 2, 8}
```

---

## Constants

### `IEND`

This is the value returned by an iterator that signals its end.
