# Math library

## Importing

```nest
|#| 'stdmath.nest' = math
```

## Trigonometric functions

### `@acos`

**Synopsis:**

```nest
[n: Byte|Int|Real] @acos -> Real
```

**Returns:**

The arc cosine of `n` radians.

---

### `@asin`

**Synopsis:**

```nest
[n: Byte|Int|Real] @asin -> Real
```

**Returns:**

The arc sine of `n` radians.

---

### `@atan`

**Synopsis:**

```nest
[n: Byte|Int|Real] @atan -> Real
```

**Returns:**

The arc tangent of `n` radians.

---

### `@acosh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @acosh -> Real
```

**Returns:**

The arc hyperbolic cosine of `n` radians.

---

### `@asinh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @asinh -> Real
```

**Returns:**

The arc hyperbolic sine of `n` radians.

---

### `@atanh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @atanh -> Real
```

**Returns:**

The arc hyperbolic tangent of `n` radians.

---

### `@atan2`

**Synopsis:**

```nest
[y: Byte|Int|Real, x: Byte|Int|Real] @atan2 -> Real
```

**Returns:**

The angle in radians of the vector from `{0, 0}` to `{x, y}` relative to the x
axis.

---

### `@cos`

**Synopsis:**

```nest
[n: Byte|Int|Real] @cos -> Real
```

**Returns:**

The cosine of `n` radians.

---

### `@sin`

**Synopsis:**

```nest
[n: Byte|Int|Real] @sin -> Real
```

**Returns:**

The sine of `n` radians.

---

### `@tan`

**Synopsis:**

```nest
[n: Byte|Int|Real] @tan -> Real
```

**Returns:**

The tangent of `n` radians.

---

### `@cosh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @cosh -> Real
```

**Returns:**

The hyperbolic cosine of `n` radians.

---

### `@sinh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @sinh -> Real
```

**Returns:**

The hyperbolic sine of `n` radians.

---

### `@tanh`

**Synopsis:**

```nest
[n: Byte|Int|Real] @tanh -> Real
```

**Returns:**

The hyperbolic tangent of `n` radians.

---

## Other Functions

### `@abs`

**Synopsis:**

```nest
[number: Byte|Int|Real] @abs -> Byte|Int|Real
```

**Returns:**

The absolute value of `number`.

---

### `@ceil`

**Synopsis:**

```nest
[n: Byte|Int|Real] @ceil -> Int
```

**Description:**

Calculates the ceil of `n`.

---

### `@clamp`

**Synopsis:**

```nest
[n: Byte|Int|Real, min: Byte|Int|Real, max: Byte|Int|Real] @clamp -> Real
```

**Returns:**

A value such that `min <= value <= max`. If `n` is greater than `max`, `max` is
returned, similarly if `n` is smaller than `min` then `min` is returned.
If `n` is between `min` and `max` inclusive, `n` itself is returned.

---

### `@deg`

**Synopsis:**

```nest
[n: Byte|Int|Real] @deg -> Real
```

**Returns:**

Transforms `n` radians in degrees.

---

### `@dist_nd`

**Synopsis:**

```nest
[a: Array|Vector, b: Array|Vector] @dist_nd -> Real
```

**Returns:**

Calculates the euclidean distance of points `a` and `b` with the same number of
dimensions. The two arrays must have the same length and the coordinates must
be of type `Byte`, `Int` or `Real`.

---

### `@dist_2d`

**Synopsis:**

```nest
[a: Array|Vector, b: Array|Vector] @dist_2d -> Real
```

**Returns:**

Calculates the euclidean distance of 2D points `a` and `b`. The two arrays must
have a length of 2 and the coordinates must be of type `Byte`, `Int` or `Real`.

---

### `@dist_3d`

**Synopsis:**

```nest
[a: Array|Vector, b: Array|Vector] @dist_3d -> Real
```

**Returns:**

Calculates the euclidean distance of 3D points `a` and `b`. The two arrays must
have a length of 3 and the coordinates must be of type `Byte`, `Int` or `Real`.

---

### `@divmod`

**Synopsis:**

```nest
[a: Int, b: Int] @divmod -> Array
```

**Description:**

Returns a 2-element array with the first being the result of `a b /` and the
second being the result of `a b %`, equal to `{a b /, a b %}`

---

### `@exp`

**Synopsis:**

```nest
[n: Byte|Int|Real] @exp -> Real
```

**Description:**

Returns Euler's constant `e` raised to the power of `n`.

---

### `@floor`

**Synopsis:**

```nest
[n: Byte|Int|Real] @floor -> Int
```

**Description:**

Calculates the floor of `n`.

---

### `@frexp`

**Synopsis:**

```nest
[n: Real] @frexp -> Array
```

**Returns:**

An array of size two with the mantissa of `n` as the first element and the
exponent as the second.

---

### `@gcd`

**Synopsis:**

```nest
[a: Byte|Int|Real, b: Byte|Int|Real] @gcd -> Byte|Int|Real
[seq: Array|Vector.Byte|Int|Real] @gcd -> Byte|Int|Real
```

**Returns:**

The first type returns the greatest common divisor between `a` and `b`.

The second type returns the greatest common divisor between all the elements in
`seq`.

---

### `@hypot`

**Synopsis:**

```nest
[c1: Byte|Int|Real, c2: Byte|Int|Real] @hypot -> Real
```

**Returns:**

Calculates the hypotenuse of a right triangle given the two catheti.

---

### `@is_inf`

**Synopsis:**

```nest
[n: Real] @is_inf -> Bool
```

**Returns:**

`true` if the given real number represents an infinity, either positive or
negative, and `false` otherwise.

---

### `@is_nan`

**Synopsis:**

```nest
[n: Real] @is_nan -> Bool
```

**Returns:**

`true` if the given real number is not a number and `false` otherwise.

---

### `@lcm`

**Synopsis:**

```nest
[a: Byte|Int|Real, b: Byte|Int|Real] @lcm -> Byte|Int|Real
[seq: Array|Vector.Byte|Int|Real] @lcm -> Byte|Int|Real
```

**Returns:**

The first type returns the least common multiple between `a` and `b`.

The second type returns the least common multiple between all the elements in
`seq`.

---

### `@ldexp`

**Synopsis:**

```nest
[m: Real, e: Int] @ldexp -> Real
```

**Returns:**

Inverse of `frexp`, returns a `Real` number with `m` as the mantissa and `e` as
the exponent.

---

### `@ln`

**Synopsis:**

```nest
[n: Byte|Int|Real] @ln -> Real
```

**Description:**

Returns the natural logarithm of `n`.

---

### `@log`

**Synopsis:**

```nest
[n: Byte|Int|Real, m: Byte|Int|Real|null] @log -> Real
```

**Description:**

Returns the logarithm of `n` with base `m`. If `m` is `null` base `10` is used.

---

### `@map`

**Synopsis:**

```nest
[n: Num, min1: Num, max1: Num, min2: Num, max2: Num] @map -> Real
```

!!!note
    To make the synopsis more readable, `Num` is `Byte|Int|Real`.

**Description:**

Maps `n` from the range `[min1, max1]`, to the range  `[min2, max2]`. Uses the
formula `(n min1 -) (max1 min1 -) / (max2 min2 -) * min2 +`. If `min1` and
`max1` are equal, `min2` is returned.

---

### `@max`

**Synopsis:**

```nest
[a: Any, b: Any] @max -> Any
[seq: Array|Vector] @max -> Any
```

**Returns:**

The first type returns the biggest object between `a` and `b`.

The second type returns the biggest object inside `seq`.

**Example:**

```nest
|#| 'stdmath.nest' = math

5 2 @math.max --> 5
{2, 3, 5, 1} @math.max --> 5
'foo' 'bar' @math.max --> 'foo'
```

---

### `@min`

**Synopsis:**

```nest
[a: Any, b: Any] @min -> Any
[seq: Array|Vector] @min -> Any
```

**Returns:**

The first type returns the smallest object between `a` and `b`.

The second type returns the smallest object inside `seq`.

**Example:**

```nest
|#| 'stdmath.nest' = math

5 2 @math.min --> 2
{2, 3, 5, 1} @math.min --> 1
'foo' 'bar' @math.min --> 'bar'
```

---

### `@rad`

**Synopsis:**

```nest
[n: Byte|Int|Real] @rad -> Real
```

**Returns:**

Transforms `n` degrees in radians.

---

### `@round`

**Synopsis:**

```nest
[n: Byte|Int|Real] @round -> Int
```

**Description:**

Rounds `n` to the nearest integer. Numbers ending in `.5` get rounded up.

---

### `@sum`

**Synopsis:**

```nest
[sequence: Array|Vector] @sum -> Any
```

**Description:**

Returns the sum of the elements in `sequence`. The type of the return value
depends on the type of the elements inside the sequence.

---

## Constants

### `E`

Euler's constant, about equal to `2.71828`.

---

### `INF`

A positive infinity for double-precision floating point numbers.

---

### `NAN`

Not a number for double-precision floating point numbers.

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
