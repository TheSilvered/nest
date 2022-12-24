# Random library (`stdrand.nest` - `rand`)

## Functions

### `[] @random`

A random integer between `0` and `RAND_MAX`.

### `[min: Int, max: Int] @rand_int`

A random integer between `min` and `max`. `max` must be greate or equalt to `min`.

### `[] @rand_perc`

A random real number between `0.0` and `1.0`.

### `[sequence: Str|Array|Vector] @choice`

Returns a random element choosing between the ones in `sequence`.

### `[sequence: Array|Vector] @shuffle`

Shuffles `sequence` in-place, it does not create a new one.

### `[seed: Int] @seed`

Sets the seed for the pseudo random number generator. By default it is
`@@time_ns` from the function in `stdtime.nest`
