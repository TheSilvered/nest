# Sequence utilities library (`stdsequtil.nest` - `seq`)

## Functions

### `[seq: Array|Vector, func: Func] @map`

Creates a new sequence of type `?::seq` where all items of `seq` are passed
through `func`.  
`func` must take exactly one argument.

### `[vect: Vector, index: Int, object: Any] @insert_at`

Inserts `objects` at `index` in `vect`. Returns `null`.

### `[vect: Vector, index: Int] @remove_at`

Removes the element at `index` in `vect` and returns it.

### `[seq: Str|Array|Vector, start: Int, stop: Int, step: Int] @slice`

Creates a new sequence of type `?::seq` that contains the elements from `start`
to `stop` separated by a gap of `step`.

### `[seq1: Array|Vector, seq2: Array|Vector] @merge`

Creates a new sequence that merges the two sequences together, one after the
other.

### `[seq: Arr|Vect] @sort`

Sorts `seq` in increasing order. The elements can be all numbers or all strings
but not a mixture of those. This is implemented using an algorithm called Timsort.

### `[vect: Vector] @empty`

Empties `vect` and returns it.

### `[seq: Vector|Array, func: Func] @filter`

Creates a new sequence (the type depends on the first argument) that has all the
elements from `seq` that when passed as an argument to `func` returned `true`.

### `[container: Vector|Array|Map, object: Any] @contains`

Returns `true` if `object` is inside `container`, for maps only the keys are
checked therefore `{ 'a': 1 } 1 @contains` is false.

### `[seq: Array|Vector] @any`

Returns true if any of the elements of `seq` casted to a boolean is true.

### `[seq: Array|Vector] @all`

Returns true if all the elements of `seq` casted to a boolean are true.

### `[iter: Iter] @from_iter`

Returns a vector of which elements are the objects yielded by `iter`.
