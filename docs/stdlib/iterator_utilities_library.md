# Iterator utilities library

## Importing

```nest
|#| 'stditutil.nest' = itu
```

## Functions

### `@chain`

**Synopsis:**

`[sequence: Iter|Array|Vector|Str] @chain -> Iter`

**Returns:**

An iterator that chains the objects inside of the elements inside of the
sequence.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{ 'Hi!', 1 -> 4 } @itu.chain --> 'H', 'i', '!', 1, 2, 3
'Hi!' @itu.chain --> 'H', 'i', '!' this is the same as { 'H', 'i', '!'} @itu.chain
'Hi!' @itu.enumerate @itu.chain --> 0, 'H', 1, 'i', 2, '!'
```

---

### `@count`

**Synopsis:**

`[start: Int, step: Int] @count -> Iter`

**Returns:**

An iterator that counts indefinitely from `start`, advancing by `step`.

**Example:**

```nest
|#| 'stditutil.nest' = itu

0 2 @itu.count --> 0, 2, 4, 6, 8, ...
```

---

### `@cycle`

**Synopsis:**

`[sequence: Str|Array|Vector] @cycle -> Iter`

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

`[iterator: Str|Array|Vector|Iter, start: Int?, step: Int?, invert_order: Bool?] @enumerate -> Iter`

**Returns:**

A 2-element array where the first element is the object returned by the
`iterator` and the second is the index of the current iteration. When
`invert_order` is set to a truthy value the first element is the object and the
second the iteration count.

`start` by default is set to `0` and `step` to `1` but this behaviour can be
changed by passing the additional arguments.

If `iterator` is a `Str`, `Array` or `Vector` it is automatically casted to an
`Iter`.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.enumerate --> { 0, 'H' }, { 1, 'i' }, { 2, '!' }
'Hi!' 5 -1 @itu.enumerate --> { 5, 'H' }, { 4, 'i' }, { 3, '!' }
'Hi!' @itu.reversed @itu.enumerate --> { 0, '!' }, { 1, 'i' }, { 2, 'H' }
'Hi!' 0 1 true @itu.enumerate --> { 'H', 0 }, { 'i', 1 }, { '!', 2 }
```

---

### `@iter_is_done`

**Synopsis:**

`[iter: Iter] @iter_is_done`

**Description:**

Calls the `_is_done_` function of an iterator.

---

### `@iter_get_val`

**Synopsis:**

`[iter: Iter] @iter_get_val`

**Description:**

Calls the `_get_val_` function of an iterator.

---

### `@iter_start`

**Synopsis:**

`[iter: Iter] @iter_start`

**Description:**

Calls the `_start_` function of an iterator.

---

### `@keys`

**Synopsis:**

`[map: Map] @keys -> Iter`

**Returns:**

All the keys in a map, the order is not the one in which you put the
objects in.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.keys --> 'key_1', 'key_2'
{ 'key_2': 2, 'key_1': 1 } @itu.keys --> 'key_2', 'key_1'
```

---

### `@new_iterator`

**Synopsis:**

`[start: Func, is_done: Func, get_val: Func, data: Any] @new_iterator -> Iter`

**Description:**

Creates a new iterator object that uses custom functions. Any of the given
functions must take exactly one parameter that will be `data`.

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

`[object: Any, times: Int] @repeat -> Iter`

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

`[sequence: Str|Array|Vector] @reversed -> Iter`

**Returns:**

The elements of a sequence in reverse order, from the last to the first.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.reversed --> '!', 'i', 'H'
```

---

### `@values`

**Synopsis:**

`[map: Map] @values -> Iter`

**Returns:**

All the values in a map, the order is not the one in which you put the
objects in.

**Example:**

```nest
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.values --> 1, 2
{ 'key_2': 2, 'key_1': 1 } @itu.values --> 2, 1
```

---

### `@zip`

**Synopsis:**

`[seq_1: Array|Vector|Str|Iter, seq_2: Array|Vector|Str|Iter?] @zip -> Iter`

**Returns:**

The function either accepts a sequence containing `Array`, `Vector`, `Str` or
`Iter` objects in `seq_1` with `seq_2` that is `null` or two sequences.

When `seq_2` is `null` the function returns an iterator that constructs an
array with the values at the same index in all of the sequences.

Otherwise an array of length two is created with the values at the same index
inside `seq_1` and `seq_2`.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' { 1, 2, 3 } @itu.zip --> { 'H', 1 }, { 'i', 2 }, { 'i', 3 }
{ 'Hi!', { 1, 2, 3 }, <{ 9, 8, 7, 6 }> } @itu.zip --> { 'H', 1, 9 }, { 'i', 2, 8 }, { 'i', 3, 7 }
```
