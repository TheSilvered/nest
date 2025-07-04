# Sequence utilities library

## Importing

```nest
|#| 'stdsequtil.nest' = sequ
```

## Functions

### `@all`

**Synopsis:**

```nest
[seq: Array|Vector] @all -> Bool
```

**Returns:**

`true` if all the elements of `seq` are truthy.

---

### `@any`

**Synopsis:**

```nest
[seq: Array|Vector] @any -> Bool
```

**Returns:**

`true` if at least one element in `seq` is truthy.

---

### `@copy`

**Synopsis:**

```nest
[cont: Array|Vector|Map] @copy -> Array|Vector|Map
```

**Description:**

Creates a shallow copy of a container.

**Arguments:**

- `cont`: the container to copy

**Returns:**

A shallow copy of `cont`.

---

### `@count`

**Synopsis:**

```nest
[seq: Str|Array|Vector, object: Any] @count -> Int
```

**Returns:**

The number of times `object` appears inside `seq`. When `seq` is a `Str` the
function returns the number of non-overlapping occurrences of `object` in the
string. If `object` is not a string and `seq` is `0` is returned.

---

### `@deep_copy`

**Synopsis:**

```nest
[cont: Array|Vector|Map] @copy -> Array|Vector|Map
```

**Description:**

Creates a deep copy of a container keeping recursive objects the same relative
to each other.

**Arguments:**

- `cont`: the container to copy

**Returns:**

A deep copy of `cont`.

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ
|#| 'stdsys.nest' = sys

{null, {1, 2, 3}} = arr
arr = arr.0

arr @sequ.deep_copy = new_arr

>>> (new_arr new_arr.0 ==) -- true
>>> ((new_arr @sys.get_addr) (new_arr.0 @sys.get_addr) == '\n' ><) -- true
>>> ((arr @sys.get_addr) (new_arr.0 @sys.get_addr) == '\n' ><) -- false
```

---

### `@empty`

**Synopsis:**

```nest
[vect: Vector] @empty -> Vector
```

**Description:**

Removes all objects from `vect`.

**Returns:**

`vect`.

---

### `@enum`

**Synopsis:**

```nest
[elements: Vector|Array, start: Int?] @enum -> Map
```

**Description**:

`elements` is a sequence of unique strings that will serve as the keys of the
map. If there is a duplicate in `elements` a `Value Error` is thrown. The
values associated with the elements will be unique integers starting from
`start` with the first element and increasing by 1. If `start` is not given or
is `null` the counter starts at 0.

**Returns:**

A map where the strings in `elements` are the keys which are associated with
a unique integer.

---

### `@extend`

**Synopsis:**

```nest
[vect: Vector, seq: Vector|Array|Str|Iter] @extend -> Vector
```

**Description:**

Extends `vect` with all of the items inside `seq`.

**Arguments:**

- `vect`: the vector to extend
- `seq`: the sequence containing the items to extend the vector with

**Returns:**

The extended vector, the same object as `vect`.

---

### `@filter`

**Synopsis:**

```nest
[seq: Array|Vector, func: Func] @filter -> Array|Vector
```

**Description:**

Creates a new sequence of the same type of `seq` that has all the elements from
`seq` which, when passed as an argument to `func`, returned a truthy value.

`func` must take exactly one argument.

**Returns:**

A new sequence of type `?::seq` that contains the filtered elements.

---

### `@filter_i`

**Synopsis:**

```nest
[seq: Array|Vector|Str|Iter, func: Func] @filter -> Iter
```

**Description:**

Works like [`filter`](sequence_utilities_library.md#filter) but an iterator is
created instead of a new sequence.

**Returns:**

The newly created iterator.

---

### `@insert_at`

**Synopsis:**

```nest
[vect: Vector, index: Int, object: Any] @insert_at -> null
```

**Description:**

Inserts `object` at `index` in `vect` moving all following objects by one spot.
Negative indices are allowed. If `index` falls outside the boundaries of `vect`
an error is thrown.

**Arguments**.

- `vect`: the vector to insert the object into
- `index`: the index where to insert the object
- `object`: the object to insert

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ

<{1, 2, 3, 5}> = vec
vec 4 -1 @sequ.insert_at
>>> (vec '\n' ><) --> <{1, 2, 3, 4, 5}>
```

---

### `@lscan`

**Synopsis:**

```nest
[seq: Array|Vector|Str, func: Func, start_val: Any, max_items: Int?] @lscan -> Array|Vector
```

**Description:**

Creates a new sequence where each element is the result of passing the previous
object and the current object through `func`. For the first object `start_val`
is used.
The operation happens from the start to the end of the sequence.
`func` must take exactly two arguments where the first is the previous value and
the second the current one.

```nest
{x1, x2, ...} f s @lscan == {s, s x1 @f, (s x1 @f) x2 @f, ...}
```

**Arguments:**

- `seq`: the sequence to scan
- `func`: the function to scan the sequence with
- `start_val`: the value that is passed to the sequence along the fist object
- `max_items`: the maximum number of items in the end sequence

**Returns:**

The scanned sequence that includes `start_val`.

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ

{1, 2, 3, 4} (##a b => a b +) 0 @sequ.lscan --> {0, 1, 3, 6, 10}
'hello' (##a b => a b ><) '' 4 @sequ.lscan --> {'', 'h', 'he', 'hel'}
```

---

### `@map`

**Synopsis:**

```nest
[seq: Array|Vector, func: Func, in_place: Bool?] @map -> Array|Vector
```

**Description:**

Maps all the objects in `seq` to new ones generated by passing the original
object through `func` and using the return value as the mapped object. `func`
must take exactly one argument otherwise an error is thrown.

If `in_place` is omitted or is `false` the objects are inserted in a new
sequence otherwise `seq` is modified in-place.

**Arguments:**

- `seq`: the sequence containing the items to be mapped
- `func`: the function used to map each object
- `in_place`: whether the function should modify the sequence in-place or
  create another one

**Returns:**

A sequence containing the mapped items.

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ

{1, 2, 3, 4} = arr
arr (##n => n n *) @sequ.map = squares
>>> (squares '\n' ><) --> {1, 4, 8, 16}
```

---

### `@map_i`

**Synopsis:**

```nest
[seq: Array|Vector|Str|Iter, func: Func] @map_i -> Iter
```

**Description:**

Creates an iterator which maps all the values in `seq` by passing them to
`func`. `func` must take exactly one argument.

**Arguments:**

- `seq`: the sequence containing the items to be mapped
- `func`: the function used to map each object

**Returns:**

A new iterator object.

---

### `@merge`

**Synopsis:**

```nest
[seq1: Array|Vector, seq2: Array|Vector] @merge -> Array|Vector
```

**Description:**

Creates a new sequence that merges the two sequences together, one after the
other.

**Returns:**

A new sequence of type `Array` if both `seq1` and `seq2` are arrays and
`Vector` otherwise, containing all the elements inside `seq1` followed by the
elements inside `seq2`.

---

### `@remove_at`

**Synopsis:**

```nest
[vect: Vector, index: Int] @remove_at -> Any
```

**Description:**

Removes the element at `index` in `vect` by moving all following elements one
spot back.

**Returns:**

The removed element.

---

### `@reverse`

**Synopsis:**

```nest
[sequence: Str|Array|Vector, in_place: Bool?] @reverse -> Str|Array|Vector
```

**Description:**

Reverses the elements of an array or a vector or reverses the characters in a
string. If `in_place` is `true` the elements of `sequence` are reversed
in-place without creating a new object, if `in_place` is `false` or is
omitted a new object is created with the objects in reverse order.

If `in_place` is `true` and `sequence` is a string an error is thrown because
strings are immutable and cannot be modified in-place.

!!!note
    Strings are reversed in terms of characters, not in terms of bytes: the
    bytes that compose multi-byte characters are not reversed.

**Returns:**

A new sequence created with the objects reversed if `in_place` was `true` or
the sequence passed in otherwise. The type of the object returned depends on
the type of `sequence`.

---

### `@reverse_i`

**Synopsis:**

```nest
[sequence: Str|Array|Vector] @reverse_i -> Iter
```

**Returns:**

An iterator which goes through the elements of a sequence in reverse order,
from the last one to the first.

**Example:**

```nest
|#| 'stditutil.nest' = itu

'Hi!' @itu.reversed --> '!', 'i', 'H'
```

---

### `@rscan`

**Synopsis:**

```nest
[seq: Array|Vector|Str, func: Func, start_val: Any, max_items: Int?] @lscan -> Array|Vector
```

**Description:**

Creates a new sequence where each element is the result of passing the previous
object and the current object through `func`. For the first object `start_val`
is used.
The operation happens from the end to the start of the sequence.
`func` must take exactly two arguments.

!!!note
    The arguments passed to `func` are reversed compared to `lscan`.

**Arguments:**

- `seq`: the sequence to scan
- `func`: the function to scan the sequence with
- `start_val`: the value that is passed to the sequence along the fist object
- `max_items`: the maximum number of items in the end sequence

**Returns:**

The scanned sequence that includes `start_val`.

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ

{1, 2, 3, 4} (##a b => a b +) 0 @sequ.rscan --> {10, 9, 7, 4, 0}
'hello' (##a b => a b ><) '' 4 @sequ.rscan --> {'llo', 'lo', 'o', ''}
```

---

### `@slice`

**Synopsis:**

```nest
[seq: Str|Array|Vector, start: Int?, stop: Int?, step: Int?] @slice -> Str|Array|Vector
```

**Description:**

Creates a new sequence of type `?::seq` that contains the elements from `start`
to `stop` separated by a gap of `step`. Negative indices are allowed for
`start` and `stop`. `stop` is excluded while `start` is included. A negative
`step` is allowed. `start` and `stop` are clamped inside the boundaries of
`seq`.

`start`, if set to `null`, defaults to the start of the sequence when the step
is positive and to the end when it is negative.

`stop`, if set to `null`, defaults to the end of the sequence when the step is
positive and to the start when it is negative.

`step`, if set to `null`, defaults to `1`.

**Arguments:**

- `seq`: the sequence to take a slice of
- `start`: the starting index of the slice
- `stop`: the end index of the slice
- `step`: the step between elements of the slice

**Returns:**

The function returns the slice of the sequence.

**Example:**

```nest
|#| 'stdsequtil.nest' = sequ

'Hello' null null -1 @sequ.slice --> 'olleH'
{1, 2, 3, 4} 10 1 @sequ.slice --> {,}
<{'a', 'b', 'c', 'd', 'e'}> 0 null 2 @sequ.slice --> <{'a', 'c', 'e'}>
```

---

### `@slice_i`

**Synopsis:**

```nest
[seq: Str|Array|Vector, start: Int?, stop: Int?, step: Int?] @slice_i -> Iter
```

**Description:**

Works exactly like [`slice`](sequence_utilities_library.md#slice) but instead
of creating a new sequence an iterator is created.

**Returns:**

The new iterator.

---

### `@sort`

**Synopsis:**

```nest
[seq: Array|Vector, mapping_func: Func?, new_seq: Bool?] @sort -> Array|Vector
```

**Description:**

Sorts `seq` in increasing order. When `mapping_func` is not `null` it is
used to map the objects of `seq` to keys used in the comparisons, the original
objects are sorted according to the value of the corresponding key.

`mapping_func` must take exactly one argument.

If `new_seq` is not given or `false`, `seq` is sorted in-place otherwise a new,
sorted sequence is created.

**Returns:**

The sorted sequence which is `seq` if `new_seq` is `false`.
