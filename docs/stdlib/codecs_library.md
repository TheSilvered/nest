# Codecs library

## Importing

```nest
|#| 'stdcodecs.nest' = cc
```

## Functions

### `@cp_is_valid`

**Synopsis:**

```nest
[cp: Int|Byte] @cp_is_valid -> Bool
```

**Returns:**

`true` if `cp` is a valid Unicode code point and `false` otherwise.

---

### `@from_cp`

**Synopsis:**

```nest
[cp: Int|Byte] @from_cp -> Str
```

**Returns:**

A new string containing the character associated with the given code point. If
`cp` is not valid (can be checked with
[`cp_is_valid`](codecs_library.md#cp_is_valid)) the function throws an error.

---

### `@to_cp`

**Synopsis:**

```nest
[char: Str] @to_cp -> Int
```

**Returns:**

The code point associated with the caracter in `char`. If `char` does not
contain only one character an error is thrown.

---

## Constants

### `ASCII`

[ASCII](https://en.wikipedia.org/wiki/ASCII) (a.k.a. US-ASCII) encoding name.

---

### `UTF_8`

[UTF-8](https://en.wikipedia.org/wiki/UTF-8) encoding name.

---

### `EXT_UTF_8`

extUTF-8 encoding name. This encoding is Nest-specific and is UTF-8 that
accepts unpaired surrogates.

---

### `UTF_16`

[UTF-16](https://en.wikipedia.org/wiki/UTF-16) encoding name.

---

### `UTF_16LE`

[UTF-16LE](https://en.wikipedia.org/wiki/UTF-16#Byte-order_encoding_schemes)
encoding name.

---

### `UTF_16BE`

[UTF-16BE](https://en.wikipedia.org/wiki/UTF-16#Byte-order_encoding_schemes)
encoding name.

---

### `EXT_UTF_16`

extUTF-16 encoding name. This encoding is Nest-specific and is UTF-16 that
accepts unpaired surrogates. The only exception is the last character that
must not be a high surrogate.

---

### `EXT_UTF_16LE`

extUTF-16LE encoding name. Little endian version of
[extUTF-16.](codecs_library.md#ext_utf_16).

---

### `EXT_UTF_16BE`

extUTF-16BE encoding name. Big endian version of
[extUTF-16.](codecs_library.md#ext_utf_16).

---

### `UTF_32`

[UTF-32](https://en.wikipedia.org/wiki/UTF-32) encoding name.

---

### `UTF_32LE`

[UTF-32LE](https://en.wikipedia.org/wiki/UTF-32) encoding name.

---

### `UTF_32BE`

[UTF-32BE](https://en.wikipedia.org/wiki/UTF-32) encoding name.

---

### `CP1250`

[CP1250](https://en.wikipedia.org/wiki/Windows-1250) (a.k.a. Windows-1250)
encoding name.

---

### `CP1251`

[CP1251](https://en.wikipedia.org/wiki/Windows-1251) (a.k.a. Windows-1251)
encoding name.

---

### `CP1252`

[CP1252](https://en.wikipedia.org/wiki/Windows-1252) (a.k.a. Windows-1252)
encoding name.

---

### `CP1253`

[CP1253](https://en.wikipedia.org/wiki/Windows-1253) (a.k.a. Windows-1253)
encoding name.

---

### `CP1254`

[CP1254](https://en.wikipedia.org/wiki/Windows-1254) (a.k.a. Windows-1254)
encoding name.

---

### `CP1255`

[CP1255](https://en.wikipedia.org/wiki/Windows-12505) (a.k.a. Windows-1255)
encoding name.

---

### `CP1256`

[CP1256](https://en.wikipedia.org/wiki/Windows-1256) (a.k.a. Windows-1256)
encoding name.

---

### `CP1257`

[CP1257](https://en.wikipedia.org/wiki/Windows-1257) (a.k.a. Windows-1257)
encoding name.

---

### `CP1258`

[CP1258](https://en.wikipedia.org/wiki/Windows-1258) (a.k.a. Windows-1258)
encoding name.

---

### `LATIN_1`

Latin-1 (a.k.a. ISO/IEC 8859-1) encoding name.

---

### `ISO_8859_1`

ISO/IEC 8859-1 (a.k.a. latin-1) encoding name. This is the same as
[`LATIN_1`](codecs_library.md#latin_1).
