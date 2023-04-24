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
