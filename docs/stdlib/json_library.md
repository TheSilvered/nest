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

### `@clear_options`

**Synopsis:**

```nest
[] @clear_options -> null
```

**Description:**

Puts all JSON options to their default value.

---

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

### `@get_option`

**Synopsis:**

```nest
[option: Int] @get_option -> Bool
```

**Returns:**

The value of the option requested, if the option does not exist a `Value Error`
is thrown. The options are available through [`OPTION`](#option).

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

### `@set_option`

**Synopsis**

`[option: Int, value: Bool|null] @set_option -> null`

**Description:**

Sets an option with `value`. If value is `null` the option is given its
default value. The options are available through [`OPTION`](#option).

---

## Constants

### `OPTION`

A map containing the options that can be enabled with
[`set_option`](#set_option).

- `comments`: allows for C-style comments in the JSON file, single line
  comments begin with `//` and end at the end of the line, they can be inserted
  in the same line as code; block comments begin with `/*` and end with `*/`,
  by default this option is disabled
- `trailing_commas`: allows for trailing commas in objects and arrays, by
  default this option is disabled
- `nan_and_inf`: allows `NaN` and `Infinity` to be used as number literals that
  map to [`math.NAN`](math_library.md#nan) and
  [`math.INF`](math_library.md#inf) respectively; this options also allows to
  dump any infinite or NaN values, by default this option is disabled

!!!warning
    These options are not standard and are not implemented in many parsers.
