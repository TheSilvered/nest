# Random library

## Importing

```nest
|#| 'stdrand.nest' = rand
```

## Functions

### `@choice`

**Synopsis:**

```nest
[sequence: Str|Array|Vector] @choice -> Any
```

**Returns:**

A random element choosing between the ones in `sequence`.

---

### `@random`

**Synopsis:**

```nest
[] @random -> Int
```

**Returns:**

A random integer in the range `[-2^63, 2^63)`.

---

### `@rand_int`

**Synopsis:**

```nest
[min: Int, max: Int] @rand_int -> Int
```

**Returns:**

A random integer between `min` and `max`. If `max` is smaller than `min`, an
error is thrown.

---

### `@rand_perc`

**Synopsis:**

```nest
[] @rand_perc -> Real
```

**Returns:**

A random real number in the range `[0.0, 1.0)`.

---

### `@seed`

**Synopsis:**

```nest
[seed: Int] @seed -> null
```

**Description:**

Sets the seed for the pseudo random number generator. By default it is
`@@time_ns` from the function in `stdtime.nest`

---

### `@shuffle`

**Synopsis:**

```nest
[sequence: Array|Vector, new_seq: Bool?] @shuffle -> Array|Vector
```

**Description:**

Shuffles `sequence`. If `new_seq` is omitted or is `false` a new shuffled
sequence is created otherwise it is shuffled in-place.

**Returns:**

The shuffled sequence.
