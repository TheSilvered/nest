# Sequence utilities library

## Importing

```nest
|#| 'stdsequtil.nest' = sequ
```

## Functions

### `@all`


**Synopsis**:

`[seq: Array|Vector] @all -> Bool`

**Return value**:

Returns true if all the elements of `seq` are truthy.

---

### `@any`

**Synopsis**:

`[seq: Array|Vector] @any -> Bool`

**Return value**:

Returns true if any of the elements of `seq` is truthy.

---

### `@contains`

**Synopsis**:

`[container: Array|Vector|Map|Str, object: Any] @contains -> Bool`

**Return value**:

Returns `true` if `object` is inside `container`, and `false` otherwise.  
For maps only the keys are checked, therefore `{ 'a': 1 } 1 @contains` is false.  
For strings, the functionr returns true if `object` is a substring of `container`.

---

### `@count`

**Synopsis**:

`[seq: Str|Array|Vector, object: Any] @count -> Int`

Returns the number of times `object` appears inside `seq`.

---

### `@empty`

**Synopsis**:

`[vect: Vector] @empty -> Vector`

**Description**:

Empties `vect` and returns it.

---

### `@filter`

**Synopsis**:

`[seq: Array|Vector, func: Func] @filter -> Array|Vector`

**Description**:

Creates a new sequence of the same type of `seq` that has all the elements from
`seq` that when passed as an argument to `func` returned a truthy value.  
`func` must take exactly one argument.

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

### `@lscan`

**Synopsis**:

`[seq: Array|Vector|Str, func: Func, start_val: Any, max_items: Int?] @lscan -> Array|Vector`

**Description**:

Creates a new sequence where each element is the result of passing the previous
object and the current object through `func`. For the first object `start_val`
is used.  
The operation happens from the start to the end of the sequence.  
`func` must take exactly two arguments where the first is the previous value and
the second the current one.

```nest
{ x1, x2, ... } f s @lscan == { s, s x1 @f, (s x1 @f) x2 @f, ... }
```

**Arguments**:

- `seq`: the sequence to scan
- `func`: the function to scan the sequence with
- `start_val`: the value that is passed to the sequence along the fist object
- `max_items`: the maximum number of items in the end sequence

**Return value**:

The function returns the scanned sequence that includes `start_val`.

**Example**:

```nest
|#| 'stdsequtil.nest' = sequ

{ 1, 2, 3, 4 } (##a b => a b +) 0 @sequ.lscan --> { 0, 1, 3, 6, 10 }
'hello' (##a b => a b ><) '' 4 @sequ.lscan --> { '', 'h', 'he', 'hel' }
```

---

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

### `@merge`

**Synopsis**:

`[seq1: Array|Vector, seq2: Array|Vector] @merge -> Array|Vector`

**Description**:

Creates a new sequence that merges the two sequences together, one after the
other.

**Return value**:

The function returns a new sequence of type `Array` if both `seq1` and `seq2`
are arrays and `Vector` otherwise, containing all the elements inside `seq1`
followed by the elements inside `seq2`.

---

### `@remove_at`

**Synopsis**:

`[vect: Vector, index: Int] @remove_at -> Any`

**Description**:

Removes the element at `index` in `vect` and returns it.

---

### `@rscan`

**Synopsis**:

`[seq: Array|Vector|Str, func: Func, start_val: Any, max_items: Int?] @lscan -> Array|Vector`

**Description**:

Creates a new sequence where each element is the result of passing the previous
object and the current object through `func`. For the first object `start_val`
is used.  
The operation happens from the end to the start of the sequence.  
`func` must take exactly two arguments.

!!!note
    The arguments passed to `func` are reversed compared to `lscan`.

**Arguments**:

- `seq`: the sequence to scan
- `func`: the function to scan the sequence with
- `start_val`: the value that is passed to the sequence along the fist object
- `max_items`: the maximum number of items in the end sequence

**Return value**:

The function returns the scanned sequence that includes `start_val`.

**Example**:

```nest
|#| 'stdsequtil.nest' = sequ

{ 1, 2, 3, 4 } (##a b => a b +) 0 @sequ.rscan --> { 10, 9, 7, 4, 0 }
'hello' (##a b => a b ><) '' 4 @sequ.rscan --> { 'llo', 'lo', 'o', '' }
```

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

### `@sort`

**Synopsis**:

`[seq: Array|Vector] @sort -> Array|Vector`

**Description**:

Sorts `seq` in increasing order. The elements can be all numbers or all strings
but not a mixture of those. It uses a stable sorting algorithm called Timsort.

**Return value**:

The function returns the sorted sequence of type `Array` or `Vector` depending
on the type of `seq`.
