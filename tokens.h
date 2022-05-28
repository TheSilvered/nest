#ifndef Nst_TOKENS_H
#define Nst_TOKENS_H

#include "error.h" // Pos

typedef struct Token
{
    Pos start;
    Pos end;
    int type;
    void *value;
}
Token;

enum token_types
{
    ADD,       // + + stack-op start, num-op start
    SUB,       // | |
    MUL,       // | |
    DIV,       // | |
    POW,       // | |
    MOD,       // | |
    B_AND,     // | |
    B_OR,      // | |
    B_XOR,     // | |
    LSHIFT,    // | |
    RSHIFT,    // | - num-op end
    CONCAT,    // |
    L_AND,     // | + cond-op start
    L_OR,      // | |
    L_XOR,     // | - cond-op end
    GT,        // | + comp-op start
    LT,        // | |
    EQ,        // | |
    NEQ,       // | |
    GTE,       // | |
    LTE,       // - - stack-op end, comp-op end
    CAST,      // + local-stack-op start
    CALL,      // |
    EXTRACT,   // |
    RANGE,     // - local-stack-op end
    ASSIGN,    // + assignment start
    ADD_A,     // |
    SUB_A,     // |
    MUL_A,     // |
    DIV_A,     // |
    POW_A,     // |
    MOD_A,     // |
    B_AND_A,   // |
    B_OR_A,    // |
    B_XOR_A,   // |
    LSHIFT_A,  // |
    RSHIFT_A,  // |
    CONCAT_A,  // - assignment end
    LEN,       // + + atom start, local-op start
    L_NOT,     // | |
    B_NOT,     // | |
    OUT,       // | |
    IN,        // | |
    TYPEOF,    // | - local-op end
    IDENT,     // | + value start
    INT,       // | |
    REAL,      // | |
    STRING,    // | - value end
    L_PAREN,   // |
    L_BRACE,   // |
    L_VBRACE,  // - atom end
    L_BRACKET, // + expr-end start
    R_PAREN,   // |
    R_BRACE,   // |
    R_VBRACE,  // |
    R_BRACKET, // |
    IF,        // |
    AS,        // |
    ENDL,      // |
    COMMA,     // |
    COLON,     // |
    EOFILE,    // - expr-end end

    // other tokens
    WHILE,
    DOWHILE,
    FOR,
    FUNC,
    RETURN,
    CONTINUE,
    BREAK,

    // only used by the optimizer
    ARRAY,
    VECTOR,
    MAP
};

#define SYMBOL_CHARS "+-*/^%&|<>=!@~:;?.#\\"
#define PAREN_CHARS "<()[]{}"
#define DIGIT_CHARS "0123456789"
#define LETTER_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_"

#define T_IN_STACK_OP(token_type) \
    ( token_type >= ADD && token_type <= LTE )
#define T_IN_NUM_OP(token_type) \
    ( token_type >= ADD && token_type <= RSHIFT )
#define T_IN_COND_OP(token_type) \
    ( token_type >= L_AND && token_type <= L_XOR )
#define T_IN_COMP_OP(token_type) \
    ( token_type >= GT && token_type <= LTE )
#define T_IN_LOCAL_STACK_OP(token_type) \
    ( token_type >= CAST && token_type <= RANGE )
#define T_IN_ASSIGNMENT(token_type) \
    ( token_type >= ASSIGN && token_type <= CONCAT_A )
#define T_IN_LOCAL_OP(token_type) \
    ( token_type >= LEN && token_type <= TYPEOF )
#define T_IN_ATOM(token_type) \
    ( token_type >= LEN && token_type <= L_VBRACE )
#define T_IN_VALUE(token_type) \
    ( token_type >= IDENT && token_type <= STRING )
#define T_IN_EXPR_END(token_type) \
    ( token_type >= L_BRACKET && token_type <= EOFILE )

// the assignment tokens are in the same order as the stack op tokens
#define ASSIGMENT_TO_STACK_OP(token_type) \
    ( token_type - ADD_A )

#define TOK(expr) ((Token *)(expr))

Token *new_token_value(Pos start, Pos end, int type, void *value);
Token *new_token_noval(Pos start, Pos end, int type);
Token *new_token_noend(Pos start, int type);
void destroy_token(Token *token);

int str_to_tok(char *str);

void print_token(Token *token);

#endif // !Nst_TOKENS_H