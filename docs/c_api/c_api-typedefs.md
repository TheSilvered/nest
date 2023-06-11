# `typedefs.h`

This header defines some useful macros for platform-depended code and declares
the type aliases mentioned in the introduction.

## Macros

### `WINDOWS`

**Description**:

Defined if the platform is MS Windows at compile-time.

---

### `ARCH_x64`

**Description**:

Defined if the architecture is 64 bits.

---

### `ARCH_x86`

**Description**:

Defined if the architecture is 32 bits.

---

### `EXPORT`

**Description**:

On Windows, makes DLLs export the function, on Linux it does nothing.

---

### `NORETURN`

**Description**:

Marks a function with `_Noreturn`, `__declspec(noreturn)` or
`__attribute__((noreturn))` depending on version and compiler.

---

### `WIN_FMT`

**Description**:

To be placed before a format string parameter in a function declaration to
enable compile-time checks on Windows.

---

### `GNU_FMT`

**Synopsis**:

```better-c
GNU_FMT(m, n)
```

**Description**:

To place after a printf-like function to enable compile-time checking for the
arguments. `n` is the argument of the format string, `m` the total number of
arguments before variable ones.
