# JSON library

## Importing

```nest
|#| 'stdjson.nest' = json
```

## Type correlations

JSON to Nest type correlations:

| JSON            | Nest    |
| --------------- | ------- |
| `object`        | `Map`   |
| `array`         | `Array` |
| `string`        | `Str`   |
| `number (int)`  | `Int`   |
| `number (real)` | `Real`  |
| `true`          | `true`  |
| `false`         | `false` |
| `null`          | `null`  |

Nest to JSON type correlations:

| Nest     | JSON            |
| -------- | --------------- |
| `Map`    | `object`        |
| `Array`  | `array`         |
| `Vector` | `array`         |
| `Str`    | `string`        |
| `Byte`   | `number (int)`  |
| `Int`    | `number (int)`  |
| `Real`   | `number (real)` |
| `true`   | `true`          |
| `false`  | `false`         |
| `null`   | `null`          |

## Functions

### `@dump_f`

**Synopsis:**

```nest
[path: Str, object: Any, indent: Int?, encoding: Str?] @dump_f -> null
```

**Description:**

Writes to the file at `path` the serialization to JSON of `object` encoded with
`encoding`. This function overwrites any existing content on the file.
`encoding`, if `null`, defaults to `'ext-utf8'`.

`indent` specifies the indentation level when the object contains maps or
sequences. If it is set to 0, everything will be written in one continuous line.
If set to -1, the smallest representation will be used (removing the spaces
after commas and colons). By default `indent` is set to 0.

**Arguments:**

- `path`: the path of the file to write the contents to
- `indent`: the indentation to use to format the file

---

### `@dump_s`

**Synopsis:**

```nest
[object: Any, indent: Int?] @dump_s -> Str
```

**Description:**

Generates a JSON string that serializes `object` using `indent` to format it.
If `object` cannot be serialized, an error is thrown.

`indent` specifies the indentation level when the object contains maps or
sequences. If it is set to 0, everything will be written in one continuous line.
If set to -1, the smallest representation will be used (removing the spaces
after commas and colons).
By default `indent` is set to 0.

**Arguments:**

- `object`: the object to serialize
- `indent`: the indentation of the output string

**Returns:**

The string that contains the serialized object.

**Example:**

```nest
|#| 'stdjson.nest' = json

{1, 2, 3} @json.dump_s = s1
{1, 2, 3} -1 @json.dump_s = s2
{1, 2, 3} 2 @json.dump_s = s3

>>> (s1 '\n' ><)
>>> (s2 '\n' ><)
>>> (s3 '\n' ><)
```

Output:

```json
[1, 2, 3]
[1,2,3]
[
  1,
  2,
  3
]
```

---

### `@get_options`

**Synopsis:**

```nest
[] @get_options -> Int
```

**Returns:**

The options set with `set_options`. To check if a given option is enabled use
a bit-wise and (`&`) between it and all the return value of this function.

**Example:**

```nest
|#| 'stdjson.nest' = json

-- enables comments without changing any other option
@@json.get_options json.OPTIONS.comments | @json.set_options

-- checks if trailing commas are allowed
@@json.get_options json.OPTIONS.trailing_commas & ?
    >>> 'Trailing commas are allowed\n'
```

---

### `@load_f`

**Synopsis:**

```nest
[path: Str, encoding: Str?] @load_f -> Any
```

**Description:**

Opens the file at `path` with the specified `encoding` and parses its contents
as JSON data. If `encoding` is `null` it is determined automatically.

**Arguments:**

- `path`: the path to the file to open
- `encoding`: the encoding the file is opened with

**Returns:**

The parsed data as a Nest object according to the
[table above](#type-correlations).

**Example:**

The file `example.json`:

```json
[1, 2, 3, 4]
```

The code:

```nest
|#| 'stdjson.nest' = json

>>> ('example.json' @json.load_s '\n' ><) --> <{1, 2, 3, 4}>
```

---

### `@load_s`

**Synopsis:**

```nest
[string: Str] @load_s -> Any
```

**Description:**

Parses the contents of `string` as JSON data.

**Arguments:**

- `string`: the string to parse

**Returns:**

The parsed data as a Nest object according to the
[table above](#type-correlations).

**Example:**

```nest
|#| 'stdjson.nest' = json

>>> ('[1, 2, 3, 4]' @json.load_s '\n' ><) --> <{1, 2, 3, 4}>
```

---

### `@set_options`

**Synopsis**

`[options: Int] @set_options -> null`

**Description:**

Sets any option contained in [`OPTIONS`](#options).
To set more than one option you can use the `|` operator. The options given are
set to true and the ones omitted ones are set to false. Passing a `0` disables
everything. Every option is disabled by default.

**Example:**

```nest
|#| 'stdjson.nest' = json

json.OPTIONS.comments json.OPTIONS.trailing_commas | @json.set_options
```

---

## Constants

### `OPTIONS`

A map containing the options that can be enabled with
[`set_options`](#set_options).

- `comments`: does not raise an error when a comment is found. Two types of
  comments are allowed: single-line comments that start with `//` and end at
  the end of the line, and multi-line comments that start with `/*` and end
  with `*/`
- `trailing_commas`: does not raise an error if a comma is found after the last
  element of an object or array
- `nan_and_inf`: allows `NaN` and `Infinity` to be used as number literals that
  map to [`math.NAN`](math_library.md#nan) and
  [`math.INF`](math_library.md#inf) respectively; this options also allows to
  dump any infinite or NaN values

!!!warning
    These options are not standard and are not implemented in many parsers.
