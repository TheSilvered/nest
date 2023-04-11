# Iterator utilities library

## Importing

```nest
|#| 'stditutil.nest' = itu
```

## Functions

### `@chain`

**Synopsis**:

`[sequence: Iter|Array|Vector|Str] @chain -> Iter`

**Return value**:

The function returns an iterator that chains the objects inside of the elements
inside of the sequence.

**Example**:

```nest
|#| 'stditutil.nest' = itu

{ 'Hi!', 1 -> 4 } @itu.chain --> 'H', 'i', '!', 1, 2, 3
'Hi!' @itu.chain --> 'H', 'i', '!' this is the same as { 'H', 'i', '!'} @itu.chain
'Hi!' @itu.enumerate @itu.chain --> 0, 'H', 1, 'i', 2, '!'
```

---

### `@count`

**Synopsis**:

`[start: Int, step: Int] @count -> Iter`

**Return value**:

The function returns an iterator that counts indefinitely from `start`,
advancing by `step`.

**Example**:

```nest
|#| 'stditutil.nest' = itu

0 2 @itu.count --> 0, 2, 4, 6, 8, ...
```

---

### `@cycle`

**Synopsis**:

`[sequence: Str|Array|Vector] @cycle -> Iter`

**Return value**:

The function returns an iterator that cycles through the elements of the given
object restarting from the first when the sequence ends.

**Example**:

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.cycle --> 'H', 'i', '!', 'H', 'i', ...
```

---

### `@enumerate`

**Synopsis**:

`[iterator: Str|Array|Vector|Iter, start: Int?, step: Int?] @enumerate -> Iter`

**Return value**:

Returns a 2-element array where the first element is the object returned by
the `iterator` and the second is the index of the current iteration. The latter
by default starts from 0 with a step of 1 but this behaviour can be changed by
passing the additional arguments.

If `iterator` is a `Str`, `Array` or `Vector` it is automatically casted to an
`Iter`.

**Example**:

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.enumerate --> { 0, 'H' }, { 1, 'i' }, { 2, '!' }
'Hi!' 5 -1 @itu.enumerate --> { 5, 'H' }, { 4, 'i' }, { 3, '!' }
'Hi!' @itu.reversed @itu.enumerate --> { 0, '!' }, { 1, 'i' }, { 2, 'H' }
```

---

### `@items`

**Synopsis**:

`[map: Map] @items -> Iter`

**Return value**:

Returns all the key-value pairs in a map, the order is not the one in which you
put the objects in.

**Example**:

```nest
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.items --> { 'key_1', 1 }, { 'key_2', 2 }
{ 'key_2': 2, 'key_1': 1 } @itu.items --> { 'key_2', 2 }, { 'key_1', 1 }
```

---

### `@iter_is_done`

**Synopsis**:

`[iter: Iter] @iter_is_done`

**Description**:

Calls the `_is_done_` function of an iterator.

---

### `@iter_get_val`

**Synopsis**:

`[iter: Iter] @iter_get_val`

**Description**:

Calls the `_get_val_` function of an iterator.

---

### `@iter_start`

**Synopsis**:

`[iter: Iter] @iter_start`

**Description**:

Calls the `_start_` function of an iterator.

---

### `@keys`

**Synopsis**:

`[map: Map] @keys -> Iter`

**Return value**:

Returns all the keys in a map, the order is not the one in which you put the
objects in.

**Example**:

```nest
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.keys --> 'key_1', 'key_2'
{ 'key_2': 2, 'key_1': 1 } @itu.keys --> 'key_2', 'key_1'
```

---

### `@repeat`


**Synopsis**:

`[object: Any, times: Int] @repeat -> Iter`

**Return value**:

The function returns an iterator that repeats `object` the number of times
specified by `times`.

```nest
|#| 'stditutil.nest' = itu

'Hi!' 3 @itu.repeat --> 'Hi!', 'Hi!', 'Hi!'
```

---

### `@reversed`

**Synopsis**:

`[sequence: Str|Array|Vector] @reversed -> Iter`

**Return value**:

Returns the elements of a sequence in reverse order, from the last to the first.

**Example**:

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.reversed --> '!', 'i', 'H'
```

---

### `@values`

**Synopsis**:

`[map: Map] @values -> Iter`

**Return value**:

Returns all the values in a map, the order is not the one in which you put the
objects in.

**Example**:

```nest
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.values --> 1, 2
{ 'key_2': 2, 'key_1': 1 } @itu.values --> 2, 1
```

---

### `@zip`

**Synopsis**:

`[seq_1: Array|Vector, seq_2: null] @zip -> Iter`  
`[seq_1: Array|Vector|Str|Iter, seq_2: Array|Vector|Str|Iter] @zip -> Iter`

**Return value**:

In the first case, with `seq_2` that is `null`, `seq_1` must contain all
objects of type `Array`, `Vector`, `Str` or `Iter` and must be of length equal
or greater to two. The iterator returned in this case returns an array of the
same length of `seq_1` containing the objects returned by each of the iterators
that seq_1 contains. The iterator stops when the end of the shortest iterator
is reached.

In the second case the iterator returns an array of length two for each
iteration containing the objects returned by the two sequences. This iterator,
similarly to the first one, stops when the end of the of the shortest sequence
is reached.

**Example**:

```nest
|#| 'stditutil.nest' = itu

'Hi!' { 1, 2, 3 } @itu.zip --> { 'H', 1 }, { 'i', 2 }, { 'i', 3 }
{ 'Hi!', { 1, 2, 3 }, <{ 9, 8, 7, 6 }> } @itu.zip --> { 'H', 1, 9 }, { 'i', 2, 8 }, { 'i', 3, 7 }
```
