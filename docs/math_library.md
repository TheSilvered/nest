# Math library

## Importing

```text
|#| 'stdmath.nest' = math
```

If an argument is annotated with `Number`, it is equivalent to `Byte|Int|Real`.

## Functions

### `[n: Number] @floor`

Calculates the floor of `n`.

### `[n: Number] @ceil`

Calculates the ceil of `n`.

### `[n: Number] @round`

Rounds `n` to the nearest integer.

### `[n: Number] @exp`

Returns Euler's constant `e` raised to the power of `n`.

### `[n: Number] @log`

Returns the natural logarithm of `n`.

### `[n: Number, m: Number] @logn`

Returns the logarithm of `n` with the base `m`, calculated with
`(n @log) (m @log) /`.

### `[n: Number] @log2`

Returns the logarithm base 2 of `n`.

### `[n: Number] @log10`

Returns the logarithm base 10 of `n`.

### `[a: Int, b: Int] @divmod`

Returns a 2-element array with the first being the result of `a b /` and the
second being the result of `a b %`, equal to `{ a b /, a b % }`

### `[n: Number] @sin`

Returns the sine of `n` radians.

### `[n: Number] @cos`

Returns the cosine of `n` radians.

### `[n: Number] @tan`

Returns the tangent of `n` radians.

### `[n: Number] @asin`

Returns the area sine of `n` radians.

### `[n: Number] @acos`

Returns the area cosine of `n` radians.

### `[n: Number] @atan`

Returns the area tangent of `n` radians.

### `[n: Number] @sinh`

Returns the hyperbolic sine of `n` radians.

### `[n: Number] @cosh`

Returns the hyperbolic cosine of `n` radians.

### `[n: Number] @tanh`

Returns the hyperbolic tangent of `n` radians.

### `[n: Number] @asinh`

Returns the area hyperbolic sine of `n` radians.

### `[n: Number] @acosh`

Returns the area hyperbolic cosine of `n` radians.

### `[n: Number] @atanh`

Returns the area hyperbolic tangent of `n` radians.

### `[y: Number, x: Number] @atan2`

Calculates the angle in radians of the vector from `{ 0, 0 }` to `{ x, y }`
relative to the x axis.

### `[a: Array|Vector, b: Array|Vector] @dist_2d`

Calculates the euclidean distance of 2D points `a` and `b`. The two arrays must
have a length of 2 and the coordinates must be of type `Int` or `Real`.

### `[a: Array|Vector, b: Array|Vector] @dist_3d`

Calculates the euclidean distance of 3D points `a` and `b`. The two arrays must
have a length of 3 and the coordinates must be of type `Int` or `Real`.

### `[a: Array|Vector, b: Array|Vector] @dist_nd`

Calculates the euclidean distance of points `a` and `b` with the same number of
dimensions. The two arrays must have the same length and the coordinates must be
of type `Int` or `Real`.

### `[n: Number] @deg`

Transforms `n` radians in degrees.

### `[n: Number] @rad`

Transforms `n` degrees in radians.

### `[a: Number, b: Number] @min`

Returns the smallest number between `a` and `b`.

### `[a: Number, b: Number] @max`

Returns the biggest number between `a` and `b`.

### `[sequence: Array|Vector] @max_seq`

Returns the smallest number in `sequence`.

### `[sequence: Array|Vector] @min_seq`

Returns the biggest number in `sequence`.

### `[sequence: Array|Vector] @sum_seq`

Returns the sum of the elements in `sequence`.

### `[n: Real] @frexp`

Returns an array of size two with the mantissa of `n` as the first element and
the exponent as the second.

### `[m: Real, e: Int] @ldexp`

Inverse of `frexp`, returns a `Real` number with `m` as the mantissa and `e` as
the exponent.

### `[n: Number, min1: Number, max1: Number, min2: Number, max2: Number] @map`

Maps `n` from a range from `min1` to `max2`, to a range from `min2` to `max2`.  
Uses the formula `(n min1 -) (max1 min1 -) / (max2 min2 -) * min2 +`.  
If `min1` and `max1` are equal, `min2` is returned.

### `[n: Number, min: Number, max: Number] @clamp`

Returns a value such that `min <= value <= max`. If `n` is greater than `max`,
`max` is returned, similarly if `n` is smaller than `min` then `min` is returned.
If `n` is between `min` and `max` inclusive, it is returned.

### `[n1: Number, n2: Number] @gcd`

Returns the greatest common divisor between `n1` and `n2`.

### `[n1: Number, n2: Number] @lcm`

Returns the least common multiple between `n1` and `n2`.

### `[seq: Array|Vector] @gcd_seq`

Returns the greatest common divisor between all the elements in `seq`.

### `[seq: Array|Vector] @lcm_seq`

Returns the least common multiple between all the elements in `seq`.

### `[number: Number] @abs`

Returns the absolute value of `number`.

### `[c1: Number, c2: Number] @hypot`

Calculates the hypotenuse of a right given the two catheti.

## Constants

### `PI`

π, defined as the ratio between the radius of the circle and its
semicircumference.  
It is about equal to `3.14159`.

### `TAU`

τ, defined as the ratio between the radius of the circle and its circumference.  
It is about equal to `6.28318` or to `2π`.

### `E`

Euler's constant, about equal to `2.71828`.

### `PHI`

φ, the golden ratio, about equal to `1.61803`.

### `GOLDEN_RATIO`

The same as `PHI`.
