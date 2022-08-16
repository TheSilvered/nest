#ifndef Nst_TOKENS_H
#define Nst_TOKENS_H

#include "error.h" // Pos
#include "obj.h"

#ifdef __cplusplus
extern "C" {
#endif // !__cplusplus

typedef struct
{
    Nst_Pos start;
    Nst_Pos end;
    int type;
    Nst_Obj *value;
}
Nst_LexerToken;

enum Nst_TokenTypes
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
    NST_TT_INT,       // | |
    NST_TT_REAL,      // | |
    NST_TT_ARR,       // | |
    NST_TT_VECT,      // | |
    NST_TT_MAP,       // | |
    NST_TT_STRING,    // | - value end
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
    NST_TT_CONTINUE
};

Nst_LexerToken *nst_new_token_value(Nst_Pos start,
                                    Nst_Pos end,
                                    int type,
                                    void *value);
Nst_LexerToken *nst_new_token_noval(Nst_Pos start, Nst_Pos end, int type);
Nst_LexerToken *nst_new_token_noend(Nst_Pos start, int type);
void nst_destroy_token(Nst_LexerToken *token);

int nst_str_to_tok(char *str);

void nst_print_token(Nst_LexerToken *token);

#ifdef __cplusplus
}
#endif // !__cplusplus

#define _NST_SYMBOL_CHARS "+-*/^%&|<>=!@~:;?.#()[]{},$"
#define _NST_DIGIT_CHARS "0123456789"
#define _NST_LETTER_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"

#define T_IN_STACK_OP(token_type) \
    ( token_type >= NST_TT_ADD && token_type <= NST_TT_LTE )
#define T_IN_NUM_OP(token_type) \
    ( token_type >= NST_TT_ADD && token_type <= NST_TT_RSHIFT )
#define T_IN_COND_OP(token_type) \
    ( token_type >= NST_TT_L_AND && token_type <= NST_TT_L_XOR )
#define T_IN_COMP_OP(token_type) \
    ( token_type >= NST_TT_GT && token_type <= NST_TT_LTE )
#define T_IN_LOCAL_STACK_OP(token_type) \
    ( token_type >= NST_TT_CAST && token_type <= NST_TT_RANGE )
#define T_IN_ASSIGNMENT(token_type) \
    ( token_type >= NST_TT_ASSIGN && token_type <= NST_TT_CONCAT_A )
#define T_IN_LOCAL_OP(token_type) \
    ( token_type >= NST_TT_LEN && token_type <= NST_TT_TYPEOF )
#define T_IN_ATOM(token_type) \
    ( token_type >= NST_TT_LEN && token_type <= NST_TT_L_VBRACE )
#define T_IN_VALUE(token_type) \
    ( token_type >= NST_TT_IDENT && token_type <= NST_TT_STRING )
#define T_IN_EXPR_END(token_type) \
    ( token_type >= NST_TT_L_BRACKET && token_type <= NST_TT_EOFILE )
#define T_IN_EXPR_END_W_BREAK(token_type) \
    ( token_type >= NST_TT_L_BRACKET && \
      token_type <= NST_TT_EOFILE || \
      token_type == NST_TT_BREAK )

// the assignment tokens are in the same order as the stack op tokens
#define ASSIGMENT_TO_STACK_OP(token_type) \
    ( token_type - NST_TT_ADD_A )

#define TOK(expr) ((Nst_LexerToken *)(expr))

#endif // !Nst_TOKENS_H