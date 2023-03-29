# JSON library (`stdjson.nest` - `json`)

JSON to Nest type correlations:

| JSON          | Nest  |
| ------------- | ----- |
| object        | Map   |
| array         | Array |
| string        | Str   |
| number (int)  | Int   |
| number (real) | Real  |
| true          | true  |
| false         | false |
| null          | null  |

Nest to JSON type correlations:

| Nest   | JSON          |
| ------ | ------------- |
| Map    | object        |
| Array  | array         |
| Vector | array         |
| Str    | string        |
| Byte   | number (int)  |
| Int    | number (int)  |
| Real   | number (real) |
| true   | true          |
| false  | false         |
| null   | null          |

## Functions

### `[string: Str] @load_s`

Parses the contents of `string` as JSON data.

### `[path: Str] @load_f`

Parses the contents of the file at `path` as JSON data.

### `[object: Any, indent: Int?] @dump_s`

Generates a JSON string that serializes `object`.  
`indent` specifies the indentation level when the object contains maps or
sequences. If it is set to 0, everything will be written in one continuous line.
If set to -1, the smallest representation will be used (removing the spaces
after commas and colons).  
By default `indent` is set to 0.

### `[path: Str, object: Any, indent: Int?] @dump_f`

Writes to the file at `path` the serialization to JSON of `object`. This
function overwrites any existing content on the file.  
`indent` works exactly like it does in `dump_s`.

### `[options: Int] @set_options`

Sets any option contained in [`OPTIONS`](#options).  
To set more than one option you can use the `|` operator. The options given are
set to true and the ones omitted are set to false. Setting `options` to zero
disables everything. Every option is disabled by default.

```text
|#| 'stdjson.nest' = json
json.OPTIONS.allow_comments \
json.OPTIONS.allow_trailing_commas | @json.set_options
```

### `[] @get_options`

Returns the options set with `set_options`.

```text
|#| 'stdjson.nest' = json
-- allows comments without disabling trailing commas if it is enabled
@@json.get_options json.OPTIONS.allow_comments | @json.set_options

-- checks if trailing commas are allowed
@@json.get_options json.OPTIONS.allow_trailing_commas & ?
    >>> 'Trailing commas are allowed\n'
```

## Constants

### `OPTIONS`

A map containing the following options that can be enabled with
[`set_options`](#options-int-set_options).

- `allow_comments`: does not raise an error when a comment is found. Two types
  of comments are allowed: single-line comments that start with `//` and end
  at the end of the line, and multi-line comments that start with `/*` and end
  with `*/`
- `allow_trailing_commas`: does not raise an error if a comma is found after the
  last element of an object or array
