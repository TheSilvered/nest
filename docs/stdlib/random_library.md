# Random library

## Importing

```nest
|#| 'stdrand.nest' = rand
```

## Functions

### `@choice`

**Synopsis:**

`[sequence: Str|Array|Vector] @choice -> Any`

**Returns:**

A random element choosing between the ones in `sequence`.

---

### `@random`

**Synopsis:**

`[] @random -> Int`

**Returns:**

A random integer in the range [-2^63, 2^63).

---

### `@rand_int`

**Synopsis:**

`[min: Int, max: Int] @rand_int -> Int`

**Returns:**

A random integer between `min` and `max`. If `max` is smaller than `min`, an
error is thrown.

---

### `@rand_perc`

**Synopsis:**

`[] @rand_perc -> Real`

**Returns:**

A random real number in the range \[0.0, 1.0\).

---

### `@seed`

**Synopsis:**

`[seed: Int] @seed -> null`

**Description:**

Sets the seed for the pseudo random number generator. By default it is
`@@time_ns` from the function in `stdtime.nest`

---

### `@shuffle`

**Synopsis:**

`[sequence: Array|Vector] @shuffle -> Array|Vector`

**Description:**

Shuffles `sequence` in-place, it does not create a new one.

**Returns:**

The shuffled sequence, the same object that was passed in.
