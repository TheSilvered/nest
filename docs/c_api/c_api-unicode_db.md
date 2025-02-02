# `unicode_db.h`

Unicode Character Database.

## Authors

TheSilvered

---

## Macros

### `Nst_UCD_MASK_UPPERCASE`

**Description:**

Mask for the Uppercase property flag.

---

### `Nst_UCD_MASK_LOWERCASE`

**Description:**

Mask for the Lowercase property flag.

---

### `Nst_UCD_MASK_CASED`

**Description:**

Mask for the Cased property flag.

---

### `Nst_UCD_MASK_ALPHABETIC`

**Description:**

Mask for the Alphabetic property flag.

---

### `Nst_UCD_MASK_DECIMAL`

**Description:**

Mask for the Numeric_Type=Decimal flag.

---

### `Nst_UCD_MASK_DIGIT`

**Description:**

Mask for the Numeric_Type=Digit flag.

---

### `Nst_UCD_MASK_NUMERIC`

**Description:**

Mask for the Numeric_Type=Numeric flag.

---

### `Nst_UCD_MASK_XID_START`

**Description:**

Mask for the XID_Start property flag.

---

### `Nst_UCD_MASK_XID_CONTINUE`

**Description:**

Mask for the XID_Continue property flag.

---

### `Nst_UCD_MASK_PRINTABLE`

**Description:**

Mask for characters in categories L, N, P, S, Zs.

---

### `Nst_UCD_MAX_CASE_EXPANSION`

**Description:**

Maximum number of characters that
[`Nst_unicode_expand_case`](c_api-unicode_db.md#nst_unicode_expand_case) can
produce.

---

## Structs

### `Nst_UnicodeChInfo`

**Synopsis:**

```better-c
typedef struct _Nst_UnicodeChInfo {
    i32 lower, upper, title;
    u32 flags;
} Nst_UnicodeChInfo
```

**Description:**

Unicode character information.

**Fields:**

- `lower`: lowercase mapping, use in
  [`Nst_unicode_expand_case`](c_api-unicode_db.md#nst_unicode_expand_case)
- `upper`: uppercase mapping, use in
  [`Nst_unicode_expand_case`](c_api-unicode_db.md#nst_unicode_expand_case)
- `title`: titlecase mapping, use in
  [`Nst_unicode_expand_case`](c_api-unicode_db.md#nst_unicode_expand_case)
- `flags`: character flags use `Nst_UCD_MASK_*` to query

---

## Functions

### `Nst_unicode_get_ch_info`

**Synopsis:**

```better-c
Nst_UnicodeChInfo Nst_unicode_get_ch_info(u32 codepoint)
```

**Description:**

Get the [`Nst_UnicodeChInfo`](c_api-unicode_db.md#nst_unicodechinfo) about a
character.

---

### `Nst_unicode_expand_case`

**Synopsis:**

```better-c
usize Nst_unicode_expand_case(u32 codepoint, i32 casing, u32 *out_codepoints)
```

**Description:**

Changes a codepoint's case.

**Parameters:**

- `codepoint`: the codepoint to change the case of
- `casing`: the casing to change to, pass to this parameter `lower`, `upper` or
  `title` of [`Nst_UnicodeChInfo`](c_api-unicode_db.md#nst_unicodechinfo)
- `out_codepoints`: an array where the resulting codepoint(s) are placed, it can
  be `NULL`

**Returns:**

The number of codepoints to which the codepoint expands, which is the number of
codepoints written to `out_codepoints` if it is not `NULL`. This number is less
than or equal to
[`Nst_UCD_MAX_CASE_EXPANSION`](c_api-unicode_db.md#nst_ucd_max_case_expansion)

---

### `Nst_unicode_is_whitespace`

**Synopsis:**

```better-c
bool Nst_unicode_is_whitespace(u32 codepoint)
```

**Description:**

Check if a character is whitespace.

---

### `Nst_unicode_is_titlecase`

**Synopsis:**

```better-c
bool Nst_unicode_is_titlecase(Nst_UnicodeChInfo ch_info)
```

**Description:**

Check if a character is titlecase.
