# Sequence utilities library

## Importing

```nest
|#| 'stdsequtil.nest' = sequ
```

## Functions

### `@map`

**Synopsis**:

`[seq: Array|Vector, func: Func] @map -> Array|Vector`

**Description**:

Creates a new sequence of type `?::seq` where all items of `seq` are passed
through `func`.  
`func` must take exactly one argument.

**Arguments**:

- `seq`: the sequence containing the items to be mapped
- `func`: the function used to map each object

**Return value**:

The function returns a new sequence containing the mapped items.

**Example**:

```nest
|#| 'stdsequtil.nest' = sequ

{ 1, 2, 3, 4 } = arr
arr (##n => n n *) @sequ.map = squares
>>> (squares '\n' ><) --> { 1, 4, 8, 16 }
```

---

### `@insert_at`

**Synopsis**:

`[vect: Vector, index: Int, object: Any] @insert_at -> null`

**Description**:

Inserts `object` at `index` in `vect`.

**Arguments**.

- `vect`: the vector to insert the object into
- `index`: the index where to insert the object
- `object`: the object to insert

**Example**:

```nest
|#| 'stdsequtil.nest' = sequ

<{ 1, 2, 3, 5 }> = vec
vec 4 -1 @sequ.insert_at
>>> (vec '\n' ><) --> <{ 1, 2, 3, 4, 5 }>
```

---

### `@remove_at`

**Synopsis**:

`[vect: Vector, index: Int] @remove_at -> Any`

**Description**:

Removes the element at `index` in `vect` and returns it.

---

### `@slice`

**Synopsis**:

`[seq: Str|Array|Vector, start: Int?, stop: Int?, step: Int?] @slice -> Str|Array|Vector`

**Description**:

Creates a new sequence of type `?::seq` that contains the elements from `start`
to `stop` separated by a gap of `step`.

**Arguments**:

- `seq`: the sequence to take a slice of
- `start`: the starting index of the slice, if set to `null` it corresponds to
    the start of the sequence or to the end if the step is negative
- `stop`: the end index of the slice, it is excluded. If set to `null` it is
    set to the end of the sequence or to the start if the step is negative
- `step`: the step between elements of the slice, if set to `null` it defaults
    to 1

Any index that falls outside the sequence is clamped back in.

**Return value**:

The function returns the slice slice of the sequence.

**Example**:

```nest
|#| 'stdsequtil.nest' = sequ

'Hello' null null -1 @sequ.slice --> 'olleH'
{ 1, 2, 3, 4 } 10 1 @sequ.slice --> {,}
<{ 'a', 'b', 'c', 'd', 'e' }> 0 null 2 @sequ.slice --> <{ 'a', 'c', 'e' }>
```

---

### `[seq1: Array|Vector, seq2: Array|Vector] @merge`

Creates a new sequence that merges the two sequences together, one after the
other.

---

### `[seq: Arr|Vect] @sort`

Sorts `seq` in increasing order. The elements can be all numbers or all strings
but not a mixture of those. This is implemented using an algorithm called Timsort.

---

### `[vect: Vector] @empty`

Empties `vect` and returns it.

---

### `[seq: Vector|Array, func: Func] @filter`

Creates a new sequence (the type depends on the first argument) that has all the
elements from `seq` that when passed as an argument to `func` returned `true`.

---

### `[container: Vector|Array|Map, object: Any] @contains`

Returns `true` if `object` is inside `container`, for maps only the keys are
checked therefore `{ 'a': 1 } 1 @contains` is false.

---

### `[seq: Array|Vector] @any`

Returns true if any of the elements of `seq` casted to a boolean is true.

---

### `[seq: Array|Vector] @all`

Returns true if all the elements of `seq` casted to a boolean are true.

---

### `[seq: Str|Array|Vector, object: Any] @count`

Returns the number of times `object` appears inside `seq`.
