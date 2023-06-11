# `tokens.h`

This header contains the definitions for the tokens used by the lexer.

## Macros

### `NST_IS_STACK_OP`

**Synopsis**:

```better-c
NST_IS_STACK_OP(token_type)
```

**Description**:

Whether a token type represents a stack operator.

---

### `NST_IS_NUM_OP`

**Synopsis**:

```better-c
NST_IS_NUM_OP(token_type)
```

**Description**:

Whether a token type represents a numeric stack operator.

---

### `NST_IS_COND_OP`

**Synopsis**:

```better-c
NST_IS_COND_OP(token_type)
```

**Description**:

Whether a token type represents a logical stack operator.

---

### `NST_IS_COMP_OP`

**Synopsis**:

```better-c
NST_IS_COMP_OP(token_type)
```

**Description**:

Whether a token type represents a comparison stack operator.

---

### `NST_IS_LOCAL_STACK_OP`

**Synopsis**:

```better-c
NST_IS_LOCAL_STACK_OP(token_type)
```

**Description**:

Whether a token type represents a local-stack operator.

---

### `NST_IS_ASSIGNMENT`

**Synopsis**:

```better-c
NST_IS_ASSIGNMENT(token_type)
```

**Description**:

Whether a token type represents an assignment operator.

---

### `NST_IS_LOCAL_OP`

**Synopsis**:

```better-c
NST_IS_LOCAL_OP(token_type)
```

**Description**:

Whether a token type represents a local operator.

---

### `NST_IS_ATOM`

**Synopsis**:

```better-c
NST_IS_ATOM(token_type)
```

**Description**:

Whether a token type represents an atom-starting token.

---

### `NST_IS_VALUE`

**Synopsis**:

```better-c
NST_IS_VALUE(token_type)
```

**Description**:

Whether a token type represents a value.

---

### `NST_IS_EXPR_END`

**Synopsis**:

```better-c
NST_IS_EXPR_END(token_type)
```

**Description**:

Whether a token type represents an expression-end token.

---

### `NST_IS_EXPR_END_W_BREAK`

**Synopsis**:

```better-c
NST_IS_EXPR_END_W_BREAK(token_type)
```

**Description**:

Whether a token type represents an expression-end token, including `NST_TT_BREAK`.

---

### `NST_ASSIGMENT_TO_STACK_OP`

**Synopsis**:

```better-c
NST_ASSIGMENT_TO_STACK_OP(token_type)
```

**Description**:

Transforms a compound assignment token type into a stack operator token type.

---

### `NST_TOK`

**Synopsis**:

```better-c
NST_TOK(expr)
```

**Description**:

Casts `expr` to `Nst_Tok *`

---

## Structs

### `Nst_Tok`

**Synopsis**:

```better-c
typedef struct _Nst_Tok
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_TokType type;
    Nst_Obj *value;
}
Nst_Tok
```

**Description**:

The structure defining a Nest token.

**Fields**:

- `start`: the start position
- `end`: the end position
- `type`: the type of the token
- `value`: the value of the token

---

## Functions

### `nst_tok_new_value`

**Synopsis**:

```better-c
Nst_Tok *nst_tok_new_value(Nst_Pos     start,
                           Nst_Pos     end,
                           Nst_TokType type,
                           Nst_Obj    *value,
                           Nst_OpErr  *err)
```

**Description**:

Creates a new token that contains a value.

**Arguments**:

- `[in] start`: the start position
- `[in] end`: the end position
- `[in] type`: the type of the token
- `[in] value`: the value of the token
- `[out] err`: the error

**Return value**:

The function returns the new token or `NULL` on failure.

---

### `nst_tok_new_noval`

**Synopsis**:

```better-c
Nst_Tok *nst_tok_new_noval(Nst_Pos start,
                           Nst_Pos end,
                           Nst_TokType type,
                           Nst_OpErr *err)
```

**Description**:

Creates a new token that does not contain a value.

**Arguments**:

- `[in] start`: the start position
- `[in] end`: the end position
- `[in] type`: the type of the token
- `[out] err`: the error

**Return value**:

The function returns the new token or `NULL` on failure.

---

### `nst_tok_new_noend`

**Synopsis**:

```better-c
Nst_Tok *nst_tok_new_noend(Nst_Pos start, Nst_TokType type, Nst_OpErr *err)
```

**Description**:

Creates a new token that does not contain a value and with the same start and
end positions.

**Arguments**:

- `[in] start`: the start and end position
- `[in] type`: the type of the token
- `[out] err`: the error

**Return value**:

The function returns the new token or `NULL` on failure.

---

### `nst_token_destroy`

**Synopsis**:

```better-c
void nst_token_destroy(Nst_Tok *token)
```

**Description**:

Destroys a token.

**Arguments**:

- `[in] token`: the token to destroy

---

### `nst_tok_from_str`

**Synopsis**:

```better-c
Nst_TokType nst_tok_from_str(i8 *str)
```

**Description**:

Given a string the relative token type is returned.

**Arguments**:

- `[in] str`: the string that represents the token

**Return value**:

The function returns the token type represented by the string or `-1` on failure.

---

### `nst_print_tok`

**Synopsis**:

```better-c
void nst_print_tok(Nst_Tok *token)
```

**Description**:



**Arguments**:

- `arg`:

**Return value**:

---

## Enums

### `Nst_TokType`

**Synopsis**:

```better-c
typedef enum _Nst_TokenType
{
    NST_TT_ADD,       // + + stack-op start, num-op start
    NST_TT_SUB,       // | |
    NST_TT_MUL,       // | |
    NST_TT_DIV,       // | |
    NST_TT_POW,       // | |
    NST_TT_MOD,       // | |
    NST_TT_B_AND,     // | |
    NST_TT_B_OR,      // | |
    NST_TT_B_XOR,     // | |
    NST_TT_LSHIFT,    // | |
    NST_TT_RSHIFT,    // | - num-op end
    NST_TT_CONCAT,    // |
    NST_TT_L_AND,     // | + cond-op start
    NST_TT_L_OR,      // | |
    NST_TT_L_XOR,     // | - cond-op end
    NST_TT_GT,        // | + comp-op start
    NST_TT_LT,        // | |
    NST_TT_EQ,        // | |
    NST_TT_NEQ,       // | |
    NST_TT_GTE,       // | |
    NST_TT_LTE,       // - - stack-op end, comp-op end
    NST_TT_CAST,      // + local-stack-op start
    NST_TT_CALL,      // |
    NST_TT_SEQ_CALL,  // |
    NST_TT_THROW,     // |
    NST_TT_RANGE,     // - local-stack-op end
    NST_TT_ASSIGN,    // + assignment start
    NST_TT_ADD_A,     // |
    NST_TT_SUB_A,     // |
    NST_TT_MUL_A,     // |
    NST_TT_DIV_A,     // |
    NST_TT_POW_A,     // |
    NST_TT_MOD_A,     // |
    NST_TT_B_AND_A,   // |
    NST_TT_B_OR_A,    // |
    NST_TT_B_XOR_A,   // |
    NST_TT_LSHIFT_A,  // |
    NST_TT_RSHIFT_A,  // |
    NST_TT_CONCAT_A,  // - assignment end
    NST_TT_LEN,       // + + atom start, local-op start
    NST_TT_L_NOT,     // | |
    NST_TT_B_NOT,     // | |
    NST_TT_STDOUT,    // | |
    NST_TT_STDIN,     // | |
    NST_TT_IMPORT,    // | |
    NST_TT_LOC_CALL,  // | |
    NST_TT_NEG,       // | |
    NST_TT_TYPEOF,    // | - local-op end
    NST_TT_IDENT,     // | + value start
    NST_TT_VALUE,     // | - value end
    NST_TT_LAMBDA,    // |
    NST_TT_L_PAREN,   // |
    NST_TT_L_BRACE,   // |
    NST_TT_L_VBRACE,  // - atom end
    NST_TT_L_BRACKET, // + + expr-end-w-break start, expr-end start
    NST_TT_R_PAREN,   // | |
    NST_TT_R_BRACE,   // | |
    NST_TT_R_VBRACE,  // | |
    NST_TT_R_BRACKET, // | |
    NST_TT_IF,        // | |
    NST_TT_AS,        // | |
    NST_TT_ENDL,      // | |
    NST_TT_COMMA,     // | |
    NST_TT_COLON,     // | |
    NST_TT_CATCH,     // | |
    NST_TT_EOFILE,    // | - expr-end end
    NST_TT_BREAK,     // - expr-end-w-break end

    // other tokens
    NST_TT_EXTRACT,
    NST_TT_WHILE,
    NST_TT_DOWHILE,
    NST_TT_FOR,
    NST_TT_FUNC,
    NST_TT_RETURN,
    NST_TT_SWITCH,
    NST_TT_CONTINUE,
    NST_TT_TRY
}
Nst_TokType
```

**Description**:

The token types used by the lexer.

**Fields**:

- `NST_TT_ADD`: `+`
- `NST_TT_SUB`: `-`
- `NST_TT_MUL`: `*`
- `NST_TT_DIV`: `/`
- `NST_TT_POW`: `^`
- `NST_TT_MOD`: `%`
- `NST_TT_B_AND`: `&`
- `NST_TT_B_OR`: `|`
- `NST_TT_B_XOR`: `^^`
- `NST_TT_LSHIFT`: `<<`
- `NST_TT_RSHIFT`: `>>`
- `NST_TT_CONCAT`: `><`
- `NST_TT_L_AND`: `&&`
- `NST_TT_L_OR`: `||`
- `NST_TT_L_XOR`: `&|`
- `NST_TT_GT`: `>`
- `NST_TT_LT`: `<`
- `NST_TT_EQ`: `==`
- `NST_TT_NEQ`: `!=`
- `NST_TT_GTE`: `>=`
- `NST_TT_LTE`: `<=`
- `NST_TT_CAST`: `::`
- `NST_TT_CALL`: `@`
- `NST_TT_SEQ_CALL`: `*@`
- `NST_TT_THROW`: `!!`
- `NST_TT_RANGE`: `->`
- `NST_TT_ASSIGN`: `=`
- `NST_TT_ADD_A`: `+=`
- `NST_TT_SUB_A`: `-=`
- `NST_TT_MUL_A`: `*=`
- `NST_TT_DIV_A`: `/=`
- `NST_TT_POW_A`: `^=`
- `NST_TT_MOD_A`: `%=`
- `NST_TT_B_AND_A`: `&=`
- `NST_TT_B_OR_A`: `|=`
- `NST_TT_B_XOR_A`: `^^=`
- `NST_TT_LSHIFT_A`: `<<=`
- `NST_TT_RSHIFT_A`: `>>=`
- `NST_TT_CONCAT_A`: `><=`
- `NST_TT_LEN`: `$`
- `NST_TT_L_NOT`: `!`
- `NST_TT_B_NOT`: `~`
- `NST_TT_STDOUT`: `>>>`
- `NST_TT_STDIN`: `<<<`
- `NST_TT_IMPORT`: `|#|`
- `NST_TT_LOC_CALL`: `@@`
- `NST_TT_NEG`: `-:`
- `NST_TT_TYPEOF`: `?::`
- `NST_TT_IDENT`: identifier
- `NST_TT_VALUE`: integer, byte, real or string literal
- `NST_TT_LAMBDA`: `##`
- `NST_TT_L_PAREN`: `(`
- `NST_TT_L_BRACE`: `{`
- `NST_TT_L_VBRACE`: `<{`
- `NST_TT_L_BRACKET`: `[`
- `NST_TT_R_PAREN`: `)`
- `NST_TT_R_BRACE`: `}`
- `NST_TT_R_VBRACE`: `}>`
- `NST_TT_R_BRACKET`: `]`
- `NST_TT_IF`: `?`
- `NST_TT_AS`: `:=`
- `NST_TT_ENDL`: line feed
- `NST_TT_COMMA`: `,`
- `NST_TT_COLON`: `:`
- `NST_TT_CATCH`: `?!`
- `NST_TT_EOFILE`: the end of the file
- `NST_TT_BREAK`: `;`
- `NST_TT_EXTRACT`: `.`
- `NST_TT_WHILE`: `?..`
- `NST_TT_DOWHILE`: `..?`
- `NST_TT_FOR`: `...`
- `NST_TT_FUNC`: `#`
- `NST_TT_RETURN`: `=>`
- `NST_TT_SWITCH`: `|>`
- `NST_TT_CONTINUE`: `..`
- `NST_TT_TRY`: `??`
