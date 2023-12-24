# `typedefs.h`

Clearer C types and useful platform-agnostic macros.

## Authors

TheSilvered

---

## Macros

### `Nst_TRACK_OBJ_INIT_POS`

**Description:**

If defined enables tracking of the position in the program where objects are
allocated. This macro should be defined in `typedefs.h` when compiling.

!!!note
    This macro works only when the program is compiled in debug mode.

---

### `Nst_DISABLE_POOLS`

**Description:**

If defined disables object pools and instead frees the memory of each object.
This macro should be defined in `typedefs.h` when compiling.

!!!note
    This macro works only when the program is compiled in debug mode.

---

### `Nst_COUNT_ALLOC`

**Description:**

If defined enables allocation counting and declares the
[`Nst_log_alloc_count`](c_api-mem.md#nst_log_alloc_count) function. This macro
should be defined in `typedefs.h` when compiling.

!!!note
    This macro works only when the program is compiled in debug mode.

---

### `Nst_WIN`

**Description:**

Defined when compiling on MS Windows.

---

### `_Nst_ARCH_x64`

**Description:**

Defined when compiling on 64-bit architectures.

---

### `_Nst_ARCH_x86`

**Description:**

Defined when compiling on 32-bit architectures.

---

### `NstEXP`

**Description:**

Exports a symbol in a dynamic library.

---

### `Nst_NORETURN`

**Description:**

Marks a function that does not finish.

---

### `Nst_WIN_FMT`

**Description:**

Marks an argument as a printf format string on MSVC.

---

### `Nst_GNU_FMT`

**Synopsis:**

```better-c
#define Nst_GNU_FMT(m, n)
```

**Description:**

Marks an argument as a printf format string on GCC.

---

### `Nst_LITTLE_ENDIAN`

**Description:**

Represents little-endian systems. Always defined.

---

### `Nst_BIG_ENDIAN`

**Description:**

Represents big-endian systems. Always defined.

---

### `Nst_BYTEORDER`

**Description:**

The endianness of the system, either
[`Nst_LITTLE_ENDIAN`](c_api-typedefs.md#nst_little_endian) or
[`Nst_BIG_ENDIAN`](c_api-typedefs.md#nst_big_endian).

---

### `NstC`

**Description:**

Marks a function for for the standard C declaration (`__cdecl`).

---

### `Nst_UNUSED`

**Synopsis:**

```better-c
#define Nst_UNUSED(v)
```

**Description:**

Marks the argument of a function as unused, without rasing any compiler
warnings.

---

## Type aliases

### `i8`

**Synopsis:**

```better-c
typedef char i8
```

**Description:**

`char` alias.

---

### `i16`

**Synopsis:**

```better-c
typedef short i16
```

**Description:**

`short` alias.

---

### `i32`

**Synopsis:**

```better-c
typedef long i32
```

**Description:**

`long int` alias.

---

### `i64`

**Synopsis:**

```better-c
typedef long long i64
```

**Description:**

`long long int` alias.

---

### `u8`

**Synopsis:**

```better-c
typedef unsigned char u8
```

**Description:**

`unsigned char` alias.

---

### `u16`

**Synopsis:**

```better-c
typedef unsigned short u16
```

**Description:**

`unsigned short` alias.

---

### `u32`

**Synopsis:**

```better-c
typedef unsigned long u32
```

**Description:**

`unsigned long` alias.

---

### `u64`

**Synopsis:**

```better-c
typedef unsigned long long u64
```

**Description:**

`unsigned long long` alias.

---

### `f32`

**Synopsis:**

```better-c
typedef float f32
```

**Description:**

`float` alias.

---

### `f64`

**Synopsis:**

```better-c
typedef double f64
```

**Description:**

`double` alias.

---

### `usize`

**Synopsis:**

```better-c
typedef size_t usize
```

**Description:**

`size_t` alias.

---

### `isize`

**Synopsis:**

```better-c
typedef ptrdiff_t isize
```

**Description:**

`ptrdiff_t` alias.

---

### `Nst_NestCallable`

**Synopsis:**

```better-c
typedef Nst_Obj *(*Nst_NestCallable)(usize, Nst_Obj **)
```

**Description:**

The signature of a C function callable by Nest.
