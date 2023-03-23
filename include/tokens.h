/* Tokens for the lexer */

#ifndef TOKENS_H
#define TOKENS_H

#include "error.h"

#define NST_IS_STACK_OP(token_type) \
    ( (token_type) >= NST_TT_ADD && (token_type) <= NST_TT_LTE )
#define NST_IS_NUM_OP(token_type) \
    ( (token_type) >= NST_TT_ADD && (token_type) <= NST_TT_RSHIFT )
#define NST_IS_COND_OP(token_type) \
    ( (token_type) >= NST_TT_L_AND && (token_type) <= NST_TT_L_XOR )
#define NST_IS_COMP_OP(token_type) \
    ( (token_type) >= NST_TT_GT && (token_type) <= NST_TT_LTE )
#define NST_IS_LOCAL_STACK_OP(token_type) \
    ( (token_type) >= NST_TT_CAST && (token_type) <= NST_TT_RANGE )
#define NST_IS_ASSIGNMENT(token_type) \
    ( (token_type) >= NST_TT_ASSIGN && (token_type) <= NST_TT_CONCAT_A )
#define NST_IS_LOCAL_OP(token_type) \
    ( (token_type) >= NST_TT_LEN && (token_type) <= NST_TT_TYPEOF )
#define NST_IS_ATOM(token_type) \
    ( (token_type) >= NST_TT_LEN && (token_type) <= NST_TT_L_VBRACE )
#define NST_IS_VALUE(token_type) \
    ( (token_type) == NST_TT_IDENT || (token_type) == NST_TT_VALUE )
#define NST_IS_EXPR_END(token_type) \
    ( (token_type) >= NST_TT_L_BRACKET && (token_type) <= NST_TT_EOFILE )
#define NST_IS_EXPR_END_W_BREAK(token_type) \
    ( ((token_type) >= NST_TT_L_BRACKET && (token_type) <= NST_TT_EOFILE) || \
      (token_type) == NST_TT_BREAK )

// the assignment tokens are in the same order as the stack op tokens
#define NST_ASSIGMENT_TO_STACK_OP(token_type) \
    ((Nst_TokType)((token_type) - NST_TT_ADD_A))

#define NST_TOK(expr) ((Nst_Tok *)(expr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

EXPORT typedef enum _Nst_TokenType
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
    NST_TT_L_BRACKET, // + expr-end start
    NST_TT_R_PAREN,   // |
    NST_TT_R_BRACE,   // |
    NST_TT_R_VBRACE,  // |
    NST_TT_R_BRACKET, // |
    NST_TT_IF,        // |
    NST_TT_AS,        // |
    NST_TT_ENDL,      // |
    NST_TT_COMMA,     // |
    NST_TT_COLON,     // |
    NST_TT_CATCH,     // |
    NST_TT_EOFILE,    // - expr-end end

    // other tokens
    NST_TT_EXTRACT,
    NST_TT_WHILE,
    NST_TT_DOWHILE,
    NST_TT_FOR,
    NST_TT_FUNC,
    NST_TT_RETURN,
    NST_TT_SWITCH,
    NST_TT_BREAK,
    NST_TT_CONTINUE,
    NST_TT_TRY
}
Nst_TokType;

EXPORT typedef struct _Nst_Tok
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_TokType type;
    Nst_Obj *value;
}
Nst_Tok;

// Creates a new token with a value
EXPORT Nst_Tok *nst_tok_new_value(Nst_Pos     start,
                                  Nst_Pos     end,
                                  Nst_TokType type,
                                  Nst_Obj    *value,
                                  Nst_OpErr  *err);
// Creates a new tokens with only a type
EXPORT Nst_Tok *nst_tok_new_noval(Nst_Pos start, Nst_Pos end, Nst_TokType type, Nst_OpErr *err);
// Creates a new token where start and end are the same
EXPORT Nst_Tok *nst_tok_new_noend(Nst_Pos start, Nst_TokType type, Nst_OpErr *err);
EXPORT void nst_token_destroy(Nst_Tok *token);

// Returns the corresponding token id from a string literal
// Ex nst_str_to_tok("+") -> NST_TT_ADD == 0
EXPORT Nst_TokType nst_tok_from_str(i8 *str);

// Prints a token like the tokens when using the -t flag
EXPORT void nst_print_tok(Nst_Tok *token);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TOKENS_H