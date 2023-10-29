# `tokens.h`

Tokens for the lexer.

## Authors

TheSilvered

---

## Macros

### `Nst_IS_STACK_OP`

**Synopsis:**

```better-c
#define Nst_IS_STACK_OP(token_type)
```

**Description:**

Checks if a token is in the `STACK_OP` category.

---

### `Nst_IS_NUM_OP`

**Synopsis:**

```better-c
#define Nst_IS_NUM_OP(token_type)
```

**Description:**

Checks if a token is in the `NUM_OP` category.

---

### `Nst_IS_COND_OP`

**Synopsis:**

```better-c
#define Nst_IS_COND_OP(token_type)
```

**Description:**

Checks if a token is in the `COND_OP` category.

---

### `Nst_IS_COMP_OP`

**Synopsis:**

```better-c
#define Nst_IS_COMP_OP(token_type)
```

**Description:**

Checks if a token is in the `COMP_OP` category.

---

### `Nst_IS_LOCAL_STACK_OP`

**Synopsis:**

```better-c
#define Nst_IS_LOCAL_STACK_OP(token_type)
```

**Description:**

Checks if a token is in the `LOCAL_STACK_OP` category.

---

### `Nst_IS_ASSIGNMENT`

**Synopsis:**

```better-c
#define Nst_IS_ASSIGNMENT(token_type)
```

**Description:**

Checks if a token is in the `ASSIGNMENT` category.

---

### `Nst_IS_LOCAL_OP`

**Synopsis:**

```better-c
#define Nst_IS_LOCAL_OP(token_type)
```

**Description:**

Checks if a token is in the `LOCAL_OP` category.

---

### `Nst_IS_ATOM`

**Synopsis:**

```better-c
#define Nst_IS_ATOM(token_type)
```

**Description:**

Checks if a token is in the `ATOM` category.

---

### `Nst_IS_VALUE`

**Synopsis:**

```better-c
#define Nst_IS_VALUE(token_type)
```

**Description:**

Checks if a token is in the `VALUE` category.

---

### `Nst_IS_EXPR_END`

**Synopsis:**

```better-c
#define Nst_IS_EXPR_END(token_type)
```

**Description:**

Checks if a token is in the `EXPR_END` category.

---

### `Nst_ASSIGMENT_TO_STACK_OP`

**Synopsis:**

```better-c
#define Nst_ASSIGMENT_TO_STACK_OP(token_type)
```

**Description:**

Transforms a compound-assignment token type into a `STACK_OP` one.

---

### `Nst_TOK`

**Synopsis:**

```better-c
#define Nst_TOK(expr)
```

**Description:**

Casts expr to [`Nst_Tok *`](c_api-tokens.md#nst_tok).

---

## Structs

### `Nst_Tok`

**Synopsis:**

```better-c
typedef struct _Nst_Tok {
    Nst_Pos start;
    Nst_Pos end;
    struct _Nst_TokType type;
    Nst_Obj *value;
} Nst_Tok
```

**Description:**

A structure representing a Nest lexer token.

**Fields:**

- `start`: the start position of the token
- `end`: the end position of the token
- `type`: the type of the token
- `value`: the value of the token

---

## Functions

### `Nst_tok_new_value`

**Synopsis:**

```better-c
Nst_Tok *Nst_tok_new_value(Nst_Pos start, Nst_Pos end, Nst_TokType type,
                           Nst_Obj *value)
```

**Description:**

Creates a new token on the heap with a value.

**Parameters:**

- `start`: the start position of the token
- `end`: the end position of the token
- `type`: the type of the token
- `value`: the value of the token

**Returns:**

The new token on success and `NULL` on failure. The error is set.

---

### `Nst_tok_new_noval`

**Synopsis:**

```better-c
Nst_Tok *Nst_tok_new_noval(Nst_Pos start, Nst_Pos end, Nst_TokType type)
```

**Description:**

Creates a new token on the heap that has no value.

**Parameters:**

- `start`: the start position of the token
- `end`: the end position of the token
- `type`: the type of the token

**Returns:**

The new token on success and `NULL` on failure. The error is set.

---

### `Nst_tok_new_noend`

**Synopsis:**

```better-c
Nst_Tok *Nst_tok_new_noend(Nst_Pos start, Nst_TokType type)
```

**Description:**

Creates a new token on the heap that has no value and that uses the same start
and end positions.

**Parameters:**

- `start`: the start position of the token
- `type`: the type of the token

**Returns:**

The new token on success and `NULL` on failure. The error is set.

---

### `Nst_tok_from_str`

**Synopsis:**

```better-c
Nst_TokType Nst_tok_from_str(i8 *str)
```

**Description:**

Gets the token type from a string of punctuation characters.

str is expected to be at least one character long.

**Parameters:**

- `str`: the string to parse into the token type

**Returns:**

The parsed token type or [`Nst_TT_INVALID`](c_api-tokens.md#nst_toktype) if the
string does not contain a valid token literal.

---

### `Nst_print_tok`

**Synopsis:**

```better-c
void Nst_print_tok(Nst_Tok *token)
```

**Description:**

Prints a token to the Nest standard output.

---

## Enums

### `Nst_TokType`

**Synopsis:**

```better-c
typedef enum _Nst_TokType {
    Nst_TT_ADD,       // + + stack-op start, num-op start
    Nst_TT_SUB,       // | |
    Nst_TT_MUL,       // | |
    Nst_TT_DIV,       // | |
    Nst_TT_POW,       // | |
    Nst_TT_MOD,       // | |
    Nst_TT_B_AND,     // | |
    Nst_TT_B_OR,      // | |
    Nst_TT_B_XOR,     // | |
    Nst_TT_LSHIFT,    // | |
    Nst_TT_RSHIFT,    // | - num-op end
    Nst_TT_CONCAT,    // |
    Nst_TT_L_AND,     // | + cond-op start
    Nst_TT_L_OR,      // | |
    Nst_TT_L_XOR,     // | - cond-op end
    Nst_TT_GT,        // | + comp-op start
    Nst_TT_LT,        // | |
    Nst_TT_EQ,        // | |
    Nst_TT_NEQ,       // | |
    Nst_TT_GTE,       // | |
    Nst_TT_LTE,       // | - comp-op end
    Nst_TT_CONTAINS,  // - stack-op end
    Nst_TT_CAST,      // + local-stack-op start
    Nst_TT_CALL,      // |
    Nst_TT_SEQ_CALL,  // |
    Nst_TT_THROW,     // |
    Nst_TT_RANGE,     // - local-stack-op end
    Nst_TT_ASSIGN,    // + assignment start
    Nst_TT_ADD_A,     // |
    Nst_TT_SUB_A,     // |
    Nst_TT_MUL_A,     // |
    Nst_TT_DIV_A,     // |
    Nst_TT_POW_A,     // |
    Nst_TT_MOD_A,     // |
    Nst_TT_B_AND_A,   // |
    Nst_TT_B_OR_A,    // |
    Nst_TT_B_XOR_A,   // |
    Nst_TT_LSHIFT_A,  // |
    Nst_TT_RSHIFT_A,  // |
    Nst_TT_CONCAT_A,  // - assignment end
    Nst_TT_LEN,       // + + atom start, local-op start
    Nst_TT_L_NOT,     // | |
    Nst_TT_B_NOT,     // | |
    Nst_TT_STDOUT,    // | |
    Nst_TT_STDIN,     // | |
    Nst_TT_IMPORT,    // | |
    Nst_TT_LOC_CALL,  // | |
    Nst_TT_NEG,       // | |
    Nst_TT_TYPEOF,    // | - local-op end
    Nst_TT_IDENT,     // | + value start
    Nst_TT_VALUE,     // | - value end
    Nst_TT_LAMBDA,    // |
    Nst_TT_L_PAREN,   // |
    Nst_TT_L_BRACE,   // |
    Nst_TT_L_VBRACE,  // - atom end
    Nst_TT_L_BRACKET, // + expr-end start
    Nst_TT_R_PAREN,   // |
    Nst_TT_R_BRACE,   // |
    Nst_TT_R_VBRACE,  // |
    Nst_TT_R_BRACKET, // |
    Nst_TT_IF,        // |
    Nst_TT_AS,        // |
    Nst_TT_ENDL,      // |
    Nst_TT_COMMA,     // |
    Nst_TT_COLON,     // |
    Nst_TT_CATCH,     // |
    Nst_TT_FMT_STR,   // |
    Nst_TT_EOFILE,    // - expr-end end
    Nst_TT_BREAK,     //

    // other tokens

    Nst_TT_EXTRACT,
    Nst_TT_WHILE,
    Nst_TT_DOWHILE,
    Nst_TT_FOR,
    Nst_TT_FUNC,
    Nst_TT_RETURN,
    Nst_TT_SWITCH,
    Nst_TT_CONTINUE,
    Nst_TT_TRY,

    // invalid token

    Nst_TT_INVALID = -1
} Nst_TokType
```

**Description:**

Types of tokens generated by the Nest lexer.
