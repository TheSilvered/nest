# `str_builder.h`

String builder.

## Authors

TheSilvered

---

## Structs

### `Nst_StrBuilder`

**Synopsis:**

```better-c
typedef struct _Nst_StrBuilder {
    u8 *value;
    usize len;
    usize cap;
} Nst_StrBuilder
```

**Description:**

Structure representing a string builder. This structure owns `value`.

**Fields:**

- `value`: the string
- `len`: the length of the string
- `cap`: the size in bytes of the allocated block

---

## Functions

### `Nst_sb_init`

**Synopsis:**

```better-c
bool Nst_sb_init(Nst_StrBuilder *sb, usize reserve)
```

**Description:**

Initialize `sb`. Pre-allocate `reserve` slots.

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_destroy`

**Synopsis:**

```better-c
void Nst_sb_destroy(Nst_StrBuilder *sb)
```

**Description:**

Destroy an [`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder). Calling this
function on a builder that failed to initialize, that was already destroyed or
that was passed to [`Nst_str_from_sb`](c_api-str_builder.md#nst_str_from_sb)
will do nothing.

---

### `Nst_sb_reserve`

**Synopsis:**

```better-c
bool Nst_sb_reserve(Nst_StrBuilder *sb, usize amount)
```

**Description:**

Ensure that the string can be expanded by `amount` bytes withot reallocating.

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_push`

**Synopsis:**

```better-c
bool Nst_sb_push(Nst_StrBuilder *sb, u8 *chars, usize count)
```

**Description:**

Add `chars` to the end of a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder).

**Parameters:**

- `sb`: the string builder to append the value to
- `chars`: the characters to append
- `count`: the length of `chars`

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_push_sv`

**Synopsis:**

```better-c
bool Nst_sb_push_sv(Nst_StrBuilder *sb, Nst_StrView sv)
```

**Description:**

Add the value of a [`Nst_StrView`](c_api-str_view.md#nst_strview) to the end of
a [`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder).

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_push_str`

**Synopsis:**

```better-c
bool Nst_sb_push_str(Nst_StrBuilder *sb, Nst_Obj *str)
```

**Description:**

Add the value of a Nest `Str` object to the end of a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder).

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_push_c`

**Synopsis:**

```better-c
bool Nst_sb_push_c(Nst_StrBuilder *sb, const char *s)
```

**Description:**

Add a NUL-terminated string (a C-string) to the end of a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder).

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_sb_push_cps`

**Synopsis:**

```better-c
bool Nst_sb_push_cps(Nst_StrBuilder *sb, u32 *cps, usize count)
```

**Description:**

Add an array of codepoints to the end of a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder). The codepoints are
encoded in extUTF-8.

**Parameters:**

- `sb`: the string builder to append the value to
- `cps`: the array of codepoints
- `count`: the number of codepoints in the array

**Returns:**

`true` on success and `false` if a memory allocation fails. The error is set.

---

### `Nst_str_from_sb`

**Synopsis:**

```better-c
Nst_ObjRef *Nst_str_from_sb(Nst_StrBuilder *sb)
```

**Description:**

Create a new Nest `Str` object from a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder). The string builder is
emptied and its value is moved to the new string.

**Returns:**

The new object on success and `NULL` on failure. The error is set.

---

### `Nst_sv_from_sb`

**Synopsis:**

```better-c
Nst_StrView Nst_sv_from_sb(Nst_StrBuilder *sb)
```

**Description:**

Create a new [`Nst_StrView`](c_api-str_view.md#nst_strview) from a
[`Nst_StrBuilder`](c_api-str_builder.md#nst_strbuilder).
