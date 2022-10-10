# Math library (`stdmath.nest` - `math`)

## Functions

### `[n: Int|Real] @floor`

Calculates the floor of `n`.

### `[n: Int|Real] @ceil`

Calculates the ceil of `n`.

### `[n: Int|Real] @round`

Rounds `n` to the nearest integer.

### `[n: Int|Real] @exp`

Returns Euler's constant `e` raised to the power of `n`.

### `[n: Int|Real] @log`

Returns the natural logarithm of `n`.

### `[n: Int|Real, m: Int|Real] @logn`

Returns the logarithm of `n` with the base `m`, calculated with
`(n @log) (m @log) /`.

### `[n: Int|Real] @log2`

Returns the logarithm base 2 of `n`.

### `[n: Int|Real] @log10`

Returns the logarithm base 10 of `n`.

### `[a: Int, b: Int] @divmod`

Returns a 2-element array with the first being the result of `a b /` and the
second being the result of `a b %`, equal to `{ a b /, a b % }`

### `[n: Int|Real] @sin`

Returns the sine of `n` radians.

### `[n: Int|Real] @cos`

Returns the cosine of `n` radians.

### `[n: Int|Real] @tan`

Returns the tangent of `n` radians.

### `[n: Int|Real] @asin`

Returns the area sine of `n` radians.

### `[n: Int|Real] @acos`

Returns the area cosine of `n` radians.

### `[n: Int|Real] @atan`

Returns the area tangent of `n` radians.

### `[y: Int|Real, x: Int|Real] @atan2`

Calculates the angle in radians of the vector from `{ 0, 0 }` to `{ x, y }`
relative to the x axis.

### `[n: Int|Real] @sinh`

Returns the hyperbolic sine of `n` radians.

### `[n: Int|Real] @cosh`

Returns the hyperbolic cosine of `n` radians.

### `[n: Int|Real] @tanh`

Returns the hyperbolic tangent of `n` radians.

### `[n: Int|Real] @asinh`

Returns the area hyperbolic sine of `n` radians.

### `[n: Int|Real] @acosh`

Returns the area hyperbolic cosine of `n` radians.

### `[n: Int|Real] @atanh`

Returns the area hyperbolic tangent of `n` radians.

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

### `[n: Int|Real] @deg`

Transforms `n` radians in degrees.

### `[n: Int|Real] @rad`

Transforms `n` degrees in radians.

### `[a: Int|Real, b: Int|Real] @min`

Returns the smallest number between `a` and `b`.

### `[a: Int|Real, b: Int|Real] @max`

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

### `[n: Int|Real, min1: Int|Real, max1: Int|Real, min2: Int|Real, max2: Int|Real] @map`

Maps `n` from a range from `min1` to `max2`, to a range from `min2` to `max2`.  
Uses the formula `(n min1 -) (max1 min1 -) / (max2 min2 -) * min2 +`.  
This implies that `min1` cannot be equal to `max1`.

### `[n: Int|Real, min: Int|Real, max: Int|Real] @clamp`

Returns a value such that `min <= value <= max`. If `n` is greater than `max`,
`max` is returned, similarly if `n` is smaller than `min` then `min` is returned.
If `n` is between `min` and `max` inclusive, it is returned.

## Constants

### `PI`

π, defined as the ratio between the radius of the circle and its
semicircumference.  
It is about equal to `3.141592653589793`.

### `TAU`

τ, defined as the ratio between the radius of the circle and its circumference.  
It is about equal to `6.283185307179586` or to `2π`.

### `E`

Euler's constant, about equal to `2.718281828459045`.

### `PHI`

φ, the golden ratio, about equal to `1.618033988749895`.

### `GOLDEN_RATIO`

The same as `PHI`.
