# Iterator utilities library (`stditutil.nest` - `itu`)

## Functions

### `[start: Int, step: Int] @count`

Counts indefinitely from `start`, advancing by `step`.

```text
|#| 'stditutil.nest' = itu

0 2 @itu.count --> 0, 2, 4, 6, 8, ...
```

### `[sequence: Str|Array|Vector] @cycle`

Cycles through the elements of the given object restarting from the first when
the sequence ends.

```text
|#| 'stditutil.nest' = itu

'Hi!' @itu.cycle --> 'H', 'i', '!', 'H', 'i' ...
```

### `[object: Any, times: Int] @repeat`

Repeats the first argument a certain number of times.

```text
|#| 'stditutil.nest' = itu

'Hi!' 3 @itu.repeat --> 'Hi!', 'Hi!', 'Hi!'
```

### `[sequence: Array|Vector] @chain`

Chains any number of sequences (`Str`, `Array` or `Vector`), iterating over
their objects:

```text
|#| 'stditutil.nest' = itu

{ 'Hi!' { 1, 2, 3 } } @itu.chain --> 'H', 'i', '!', 1, 2, 3
```

### `[sequence_1: Str|Array|Vector, sequence_2: Str|Array|Vector] @zip`

Returns a 2-element array with an object from the same index of the two
sequences. It stops when the shortest sequence ends.

```text
|#| 'stditutil.nest' = itu

'Hi!' { 1, 2, 3 } @itu.zip --> { 'H', 1 }, { 'i', 2 }, { 'i', 3 }
```

### `[sequences: Array|Vector] @zipn`

`sequences` can contain arrays, vector or strings.

Returns a n-element array (where n is the length of `sequences`) with an object
from the same index of each sequence in `sequences`. It stops when the shortest
sequence ends.

```text
|#| 'stditutil.nest' = itu

{ 'Hi!', { 1, 2, 3 }, <{ 9, 8, 7, 6 }> } @itu.zipn --> { 'H', 1, 9 }, { 'i', 2, 8 }, { 'i', 3, 7 }
```

### `[sequence: Str|Array|Vector] @enumerate`

Returns a 2-element array with the index and the object at that index in the
sequence.

```text
|#| 'stditutil.nest' = itu

'Hi!' @itu.enumerate --> { 0, 'H' }, { 1, 'i' }, { 2, '!' }
```

### `[map: Map] @keys`

Returns all the keys in a map, the order is not the one in which you put the
objects in.

```text
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.keys --> 'key_2', 'key_1'
{ 'key_2': 2, 'key_1': 1 } @itu.keys --> 'key_2', 'key_1'
```

### `[map: Map] @values`

Returns all the values in a map, the order is not the one in which you put the
objects in.

```text
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.values --> 2, 1
{ 'key_2': 2, 'key_1': 1 } @itu.values --> 2, 1
```

### `[map: Map] @items`

Returns all the key-value pairs in a map, the order is not the one in which you
put the objects in.

```text
|#| 'stditutil.nest' = itu

{ 'key_1': 1, 'key_2': 2 } @itu.items --> { 'key_2', 2 }, { 'key_1', 1 }
{ 'key_2': 2, 'key_1': 1 } @itu.items --> { 'key_2', 2 }, { 'key_1', 1 }
```

### `[sequence: Str|Array|Vector] @reverse`

Returns the elements of a sequence in reverse order, from the last to the first.

```text
|#| 'stditutil.nest' = itu

'Hi!' @itu.reverse --> '!', 'i', 'H'
```

### `[iter: Iter] @iter_start`

Calls the `_start_` function of an iterator.

### `[iter: Iter] @iter_get_val`

Calls the `_get_val_` function of an iterator.

### `[iter: Iter] @iter_is_done`

Calls the `_is_done_` function of an iterator.

### `[iter: Iter] @iter_advance`

Calls the `_advance_` function of an iterator.
