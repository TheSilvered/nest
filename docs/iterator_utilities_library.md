# Iterator utilities library (`stditutil.nest` - `itu`)

## Functions

### `[start: Int, step: Int] @count`

Counts intefinitely from `start`, advancing by `step`.

```
0 2 @count --> 0, 2, 4, 6, 8, ...
```

### `[sequence: Str|Array|Vector] @cycle`

Cycles through the elements of the given object restarting from the first when
the sequence ends.

```
'Hi!' @cycle --> 'H', 'i', '!', 'H', 'i' ...
```

### `[object: Any, times: Int] @repeat`

Repeats the first argument a certain number of times.

```
'Hi!' 3 @repeat --> 'Hi!', 'Hi!', 'Hi!'
```

### `[sequence: Array|Vector] @chain`

Chains any number of sequences (`Str`, `Array` or `Vector`), iterating over
their objects:

```
{ 'Hi!' { 1, 2, 3 } } @chain --> 'H', 'i', '!', 1, 2, 3
```

### `[sequence_1: Str|Array|Vector, sequence_2: Str|Array|Vector] @zip`

Returns a 2-element array with an object from the same index of the two
sequences. It stops when the shortest sequence ends.

```
'Hi!' { 1, 2, 3 } @zip --> { 'H', 1 }, { 'i', 2 }, { 'i', 3 }
```

### `[sequence: Str|Array|Vector] @enumerate`

Returns a 2-element array with the index and the object at that index in the
sequence.

```
'Hi!' @enumerate --> { 0, 'H' }, { 1, 'i' }, { 2, '!' }
```

### `[map: Map] @keys`

Returns all the keys in a map, the order is not the one in which you put the
objects in.

```
{ 'key_1': 1, 'key_2': 2 } @keys --> 'key_2', 'key_1'
{ 'key_2': 2, 'key_1': 1 } @keys --> 'key_2', 'key_1'
```

### `[map: Map] @values`

Returns all the values in a map, the order is not the one in which you put the
objects in.

```
{ 'key_1': 1, 'key_2': 2 } @values --> 2, 1
{ 'key_2': 2, 'key_1': 1 } @values --> 2, 1
```

### `[map: Map] @items`

Returns all the key-value pairs in a map, the order is not the one in which you
put the objects in.

```
{ 'key_1': 1, 'key_2': 2 } @items --> { 'key_2', 2 }, { 'key_1', 1 }
{ 'key_2': 2, 'key_1': 1 } @items --> { 'key_2', 2 }, { 'key_1', 1 }
```

### `[sequence: Str|Array|Vector] @reverse`

Returns the elements of a sequence in reverse order, from the last to the first.

```
'Hi!' @reverse --> '!', 'i', 'H'
```
