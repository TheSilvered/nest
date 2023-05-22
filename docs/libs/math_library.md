# Math library

## Importing

```nest
|#| 'stdmath.nest' = math
```

## Trigonometric functions

### `@acos`

**Synopsis**:

`[n: Byte|Int|Real] @acos -> Real`

**Return value**:

Returns the area cosine of `n` radians.

---

### `@asin`

**Synopsis**:

`[n: Byte|Int|Real] @asin -> Real`

**Return value**:

Returns the area sine of `n` radians.

---

### `@atan`

**Synopsis**:

`[n: Byte|Int|Real] @atan -> Real`

**Return value**:

Returns the area tangent of `n` radians.

---

### `@acosh`

**Synopsis**:

`[n: Byte|Int|Real] @acosh -> Real`

**Return value**:

Returns the area hyperbolic cosine of `n` radians.

---

### `@asinh`

**Synopsis**:

`[n: Byte|Int|Real] @asinh -> Real`

**Return value**:

Returns the area hyperbolic sine of `n` radians.

---

### `@atanh`

**Synopsis**:

`[n: Byte|Int|Real] @atanh -> Real`

**Return value**:

Returns the area hyperbolic tangent of `n` radians.

---

### `@atan2`

**Synopsis**:

`[y: Byte|Int|Real, x: Byte|Int|Real] @atan2 -> Real`

**Return value**:

Calculates the angle in radians of the vector from `{ 0, 0 }` to `{ x, y }`
relative to the x axis.

---

### `@cos`

**Synopsis**:

`[n: Byte|Int|Real] @cos -> Real`

**Return value**:

Returns the cosine of `n` radians.

---

### `@sin`

**Synopsis**:

`[n: Byte|Int|Real] @sin -> Real`

**Return value**:

Returns the sine of `n` radians.

---

### `@tan`

**Synopsis**:

`[n: Byte|Int|Real] @tan -> Real`

**Return value**:

Returns the tangent of `n` radians.

---

### `@cosh`

**Synopsis**:

`[n: Byte|Int|Real] @cosh -> Real`

**Return value**:

Returns the hyperbolic cosine of `n` radians.

---

### `@sinh`

**Synopsis**:

`[n: Byte|Int|Real] @sinh -> Real`

**Return value**:

Returns the hyperbolic sine of `n` radians.

---

### `@tanh`

**Synopsis**:

`[n: Byte|Int|Real] @tanh -> Real`

**Return value**:

Returns the hyperbolic tangent of `n` radians.

---

## Other Functions

### `@abs`

**Synopsis**:

`[number: Byte|Int|Real] @abs -> Byte|Int|Real`

**Return value**:

Returns the absolute value of `number`.

---

### `@ceil`

**Synopsis**:

`[n: Byte|Int|Real] @ceil -> Int`

**Description**:

Calculates the ceil of `n`.

---

### `@clamp`

**Synopsis**:

`[n: Byte|Int|Real, min: Byte|Int|Real, max: Byte|Int|Real] @clamp -> Real`

**Return value**:

Returns a value such that `min <= value <= max`. If `n` is greater than `max`,
`max` is returned, similarly if `n` is smaller than `min` then `min` is returned.
If `n` is between `min` and `max` inclusive, it is returned.

---

### `@deg`

**Synopsis**:

`[n: Byte|Int|Real] @deg -> Real`

**Return value**:

Transforms `n` radians in degrees.

---

### `@dist_nd`

**Synopsis**:

`[a: Array|Vector, b: Array|Vector] @dist_nd -> Real`

**Return value**:

Calculates the euclidean distance of points `a` and `b` with the same number of
dimensions. The two arrays must have the same length and the coordinates must be
of type `Int` or `Real`.

---

### `@dist_2d`

**Synopsis**:

`[a: Array|Vector, b: Array|Vector] @dist_2d -> Real`

**Return value**:

Calculates the euclidean distance of 2D points `a` and `b`. The two arrays must
have a length of 2 and the coordinates must be of type `Int` or `Real`.

---

### `@dist_3d`

**Synopsis**:

`[a: Array|Vector, b: Array|Vector] @dist_3d -> Real`

**Return value**:

Calculates the euclidean distance of 3D points `a` and `b`. The two arrays must
have a length of 3 and the coordinates must be of type `Int` or `Real`.

---

### `@divmod`

**Synopsis**:

`[a: Int, b: Int] @divmod -> Array`

**Description**:

Returns a 2-element array with the first being the result of `a b /` and the
second being the result of `a b %`, equal to `{ a b /, a b % }`

---

### `@exp`

**Synopsis**:

`[n: Byte|Int|Real] @exp -> Real`

**Description**:

Returns Euler's constant `e` raised to the power of `n`.

---

### `@floor`

**Synopsis**:

`[n: Byte|Int|Real] @floor -> Int`

**Description**:

Calculates the floor of `n`.

---

### `@frexp`

**Synopsis**:

`[n: Real] @frexp -> Array`

**Return value**:

Returns an array of size two with the mantissa of `n` as the first element and
the exponent as the second.

---

### `@gcd`

**Synopsis**:

`[n1: Byte|Int|Real|Vector|Array, n2: Byte|Int|Real|null] @gcd -> Byte|Int|Real`

**Return value**:

Returns the greatest common divisor between `n1` and `n2`. If `n2` is `null`,
`n1` must be a sequence and the greatest common divisor of the elements inside
it is returned.

---

### `@hypot`

**Synopsis**:

`[c1: Byte|Int|Real, c2: Byte|Int|Real] @hypot -> Real`

**Return value**:

Calculates the hypotenuse of a right triangle given the two catheti.

---

### `@lcm`

**Synopsis**:

`[n1: Byte|Int|Real|Vector|Array, n2: Byte|Int|Real|null] @lcm -> Byte|Int|Real`

**Return value**:

Returns the least common multiple between `n1` and `n2`. If `n2` is `null`, `n1`
must be a sequence and the least common multiple of the elements inside it is
returned.

---

### `@ldexp`

**Synopsis**:

`[m: Real, e: Int] @ldexp -> Real`

**Return value**:

Inverse of `frexp`, returns a `Real` number with `m` as the mantissa and `e` as
the exponent.

---

### `@ln`

**Synopsis**:

`[n: Byte|Int|Real] @ln -> Real`

**Description**:

Returns the natural logarithm of `n`.

---

### `@log`

**Synopsis**:

`[n: Byte|Int|Real, m: Byte|Int|Real|null] @log -> Real`

**Description**:

Returns the logarithm of `n` with base `m`. If `m` is `null` base `10` is used.

---

### `@map`

**Synopsis**:

To make the synopsis more readable, `Number` means `Byte|Int|Real`.

`[n: Number, min1: Number, max1: Number, min2: Number, max2: Number] @map -> Real`

**Description**:

Maps `n` from a range from `min1` to `max2`, to a range from `min2` to `max2`.  
Uses the formula `(n min1 -) (max1 min1 -) / (max2 min2 -) * min2 +`.  
If `min1` and `max1` are equal, `min2` is returned.

---

### `@max`

**Synopsis**:

`[a: Any, b: Any] @max -> Any`

**Return value**:

Returns the biggest object between `a` and `b`. If `b` is `null`, `a` must be
either an `Array` or a `Vector` and the biggest object inside it is returned
instead.

**Example**:

```nest
|#| 'stdmath.nest' = math

5 2 @math.max --> 5
{ 2, 3, 5, 1 } @math.max --> 5
'foo' 'bar' @math.max --> 'foo'
```

---

### `@min`

**Synopsis**:

`[a: Any, b: Any] @min -> Any`

**Return value**:

Returns the smallest object between `a` and `b`. If `b` is `null`, `a` must be
either an `Array` or a `Vector` and the smallest object inside it is returned
instead.

**Example**:

```nest
|#| 'stdmath.nest' = math

5 2 @math.min --> 2
{ 2, 3, 5, 1 } @math.min --> 1
'foo' 'bar' @math.min --> 'bar'
```

---

### `@rad`

**Synopsis**:

`[n: Byte|Int|Real] @rad -> Real`

**Return value**:

Transforms `n` degrees in radians.

---

### `@round`

**Synopsis**:

`[n: Byte|Int|Real] @round -> Int`

**Description**:

Rounds `n` to the nearest integer. Numbers ending in `.5` get rounded up.

---

### `@sum`

**Synopsis**:

`[sequence: Array|Vector] @sum -> Byte|Int|Real`

**Description**:

Returns the sum of the elements in `sequence`. The type of the return value
depends on the type of the elements inside the sequence.

---

## Constants

### `E`

Euler's constant, about equal to `2.71828`.

---

### `GOLDEN_RATIO`

The same as `PHI`.

---

### `PI`

π, defined as the ratio between the radius of the circle and its
semicircumference.  
It is about equal to `3.14159`.

---

### `PHI`

φ, the golden ratio, about equal to `1.61803`.

---

### `TAU`

τ, defined as the ratio between the radius of the circle and its circumference.  
It is about equal to `6.28318` or to `2π`.
