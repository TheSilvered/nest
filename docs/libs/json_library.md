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

`[path: Str, object: Any, indent: Int?] @dump_f -> null`

**Description:**

Writes to the file at `path` the serialization to JSON of `object`. This
function overwrites any existing content on the file.
`indent` works exactly like it does in [`dump_s`](#dump_s).

**Arguments**:

- `path`: the path of the file to write the contents to
- `indent`: the indentation to use to format the file, see the argument with the
            same name in [`dump_s`](#dump_s) for specifications.

---

### `@dump_s`

**Synopsis:**

`[object: Any, indent: Int?] @dump_s -> Str`

**Description:**

Generates a JSON string that serializes `object` using `indent` to format it.
If `object` cannot be serialized, an error is thrown.

`indent` specifies the indentation level when the object contains maps or
sequences. If it is set to 0, everything will be written in one continuous line.
If set to -1, the smallest representation will be used (removing the spaces
after commas and colons).
By default `indent` is set to 0.

**Arguments**:

- `object`: the object to serialize
- `indent`: the indentation of the output string. If it is set to 0, everything
            will be written in one continuous line but keeping spaces after
            commas and colons. If set to -1, the smallest representation is used
            by removing even the spaces that 0 keeps.

**Return value**:

The function returns the string that contains the serialized object.

**Example**:

```nest
|#| 'stdjson.nest' = json

{ 1, 2, 3 } @json.dump_s = s1
{ 1, 2, 3 } -1 @json.dump_s = s2
{ 1, 2, 3 } 2 @json.dump_s = s3

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

`[] @get_options -> Int`

**Return value**:

Returns the options set with `set_options`. To check if a given option is
enabled use a bit-wise and (`&`) between it and all the return value of this
function.

**Example**:

```nest
|#| 'stdjson.nest' = json

-- allows comments without disabling allow_trailing_commas if it is enabled
@@json.get_options json.OPTIONS.allow_comments | @json.set_options

-- checks if trailing commas are allowed
@@json.get_options json.OPTIONS.allow_trailing_commas & ?
    >>> 'Trailing commas are allowed\n'
```

---

### `@load_f`

**Synopsis:**

`[path: Str] @load_f -> Any`

**Description:**

Opens the file at `path`, reads its content and parses is as json data.

**Arguments**:

- `path`: the path to the file to open

**Return value**:

The function returns the parsed data as a Nest object according to the
[table above](#type-correlations).

**Example**:

The file `example.json`:

```json
[1, 2, 3, 4]
```

The code:

```nest
|#| 'stdjson.nest' = json

>>> ('example.json' @json.load_s '\n' ><) --> <{ 1, 2, 3, 4 }>
```

---

### `@load_s`

**Synopsis:**

`[string: Str] @load_s -> Any`

**Description:**

Parses the contents of `string` as JSON data.

**Arguments**:

- `string`: the string to parse

**Return value**:

The function returns the parsed data as a Nest object according to the
[table above](#type-correlations).

**Example**:

```nest
|#| 'stdjson.nest' = json

>>> ('[1, 2, 3, 4]' @json.load_s '\n' ><) --> <{ 1, 2, 3, 4 }>
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

**Example**:

```nest
|#| 'stdjson.nest' = json

    json.OPTIONS.allow_comments \
    json.OPTIONS.allow_trailing_commas | \
@json.set_options
```

---

## Constants

### `OPTIONS`

A map containing the options that can be enabled with
[`set_options`](#set_options).

- `allow_comments`: does not raise an error when a comment is found. Two types
  of comments are allowed: single-line comments that start with `//` and end
  at the end of the line, and multi-line comments that start with `/*` and end
  with `*/`
- `allow_trailing_commas`: does not raise an error if a comma is found after the
  last element of an object or array

!!!warning
    Both of these options are not standard and very few parsers support them.
