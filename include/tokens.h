/* Tokens for the lexer */

#ifndef TOKENS_H
#define TOKENS_H

#include "error.h"

#define Nst_IS_STACK_OP(token_type) \
    ( (token_type) >= Nst_TT_ADD && (token_type) <= Nst_TT_LTE )
#define Nst_IS_NUM_OP(token_type) \
    ( (token_type) >= Nst_TT_ADD && (token_type) <= Nst_TT_RSHIFT )
#define Nst_IS_COND_OP(token_type) \
    ( (token_type) >= Nst_TT_L_AND && (token_type) <= Nst_TT_L_XOR )
#define Nst_IS_COMP_OP(token_type) \
    ( (token_type) >= Nst_TT_GT && (token_type) <= Nst_TT_LTE )
#define Nst_IS_LOCAL_STACK_OP(token_type) \
    ( (token_type) >= Nst_TT_CAST && (token_type) <= Nst_TT_RANGE )
#define Nst_IS_ASSIGNMENT(token_type) \
    ( (token_type) >= Nst_TT_ASSIGN && (token_type) <= Nst_TT_CONCAT_A )
#define Nst_IS_LOCAL_OP(token_type) \
    ( (token_type) >= Nst_TT_LEN && (token_type) <= Nst_TT_TYPEOF )
#define Nst_IS_ATOM(token_type) \
    ( (token_type) >= Nst_TT_LEN && (token_type) <= Nst_TT_L_VBRACE )
#define Nst_IS_VALUE(token_type) \
    ( (token_type) == Nst_TT_IDENT || (token_type) == Nst_TT_VALUE )
#define Nst_IS_EXPR_END(token_type) \
    ( (token_type) >= Nst_TT_L_BRACKET && (token_type) <= Nst_TT_EOFILE )
#define Nst_IS_EXPR_END_W_BREAK(token_type) \
    ( ((token_type) >= Nst_TT_L_BRACKET && (token_type) <= Nst_TT_BREAK) )

// the assignment tokens are in the same order as the stack op tokens
#define Nst_ASSIGMENT_TO_STACK_OP(token_type) \
    ((Nst_TokType)((token_type) - Nst_TT_ADD_A))

#define Nst_TOK(expr) ((Nst_Tok *)(expr))

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

NstEXP typedef enum _Nst_TokenType
{
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
    Nst_TT_LTE,       // - - stack-op end, comp-op end
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
    Nst_TT_L_BRACKET, // + + expr-end-w-break start, expr-end start
    Nst_TT_R_PAREN,   // | |
    Nst_TT_R_BRACE,   // | |
    Nst_TT_R_VBRACE,  // | |
    Nst_TT_R_BRACKET, // | |
    Nst_TT_IF,        // | |
    Nst_TT_AS,        // | |
    Nst_TT_ENDL,      // | |
    Nst_TT_COMMA,     // | |
    Nst_TT_COLON,     // | |
    Nst_TT_CATCH,     // | |
    Nst_TT_EOFILE,    // | - expr-end end
    Nst_TT_BREAK,     // - expr-end-w-break end

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

    Nst_TT_INVALID
}
Nst_TokType;

NstEXP typedef struct _Nst_Tok
{
    Nst_Pos start;
    Nst_Pos end;
    Nst_TokType type;
    Nst_Obj *value;
}
Nst_Tok;

// Creates a new token with a value
NstEXP Nst_Tok *NstC Nst_tok_new_value(Nst_Pos start, Nst_Pos end,
                                       Nst_TokType type, Nst_Obj *value);
// Creates a new tokens with only a type
NstEXP Nst_Tok *NstC Nst_tok_new_noval(Nst_Pos start, Nst_Pos end,
                                       Nst_TokType type);
// Creates a new token where start and end are the same
NstEXP Nst_Tok *NstC Nst_tok_new_noend(Nst_Pos start, Nst_TokType type);
NstEXP void NstC Nst_token_destroy(Nst_Tok *token);

// Returns the corresponding token id from a string literal
// Ex Nst_str_to_tok("+") -> Nst_TT_ADD == 0
NstEXP Nst_TokType NstC Nst_tok_from_str(i8 *str);

// Prints a token like the tokens when using the -t flag
NstEXP void NstC Nst_print_tok(Nst_Tok *token);

#ifdef __cplusplus
}
#endif // !__cplusplus

#endif // !TOKENS_H
