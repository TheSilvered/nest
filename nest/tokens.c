#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "nst_types.h"
#include "tokens.h"

Nst_LexerToken *nst_new_token_value(Nst_Pos start, Nst_Pos end, int type, void *value)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = value;
    token->type = type;

    return token;
}

Nst_LexerToken *nst_new_token_noval(Nst_Pos start, Nst_Pos end, int type)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = end;
    token->value = NULL;
    token->type = type;

    return token;
}

Nst_LexerToken *nst_new_token_noend(Nst_Pos start, int type)
{
    Nst_LexerToken *token = malloc(sizeof(Nst_LexerToken));
    if ( token == NULL )
    {
        errno = ENOMEM;
        return NULL;
    }

    token->start = start;
    token->end = start;
    token->value = NULL;
    token->type = type;

    return token;
}

void nst_destroy_token(Nst_LexerToken *token)
{
    if ( token == NULL )
        return;

    if ( token->value != NULL )
        dec_ref(token->value);
    free(token);
}

int nst_str_to_tok(char *str)
{
    if ( strcmp("+", str) == 0 )
        return NST_TT_ADD;
    if ( strcmp("-", str) == 0 )
        return NST_TT_SUB;
    if ( strcmp("*", str) == 0 )
        return NST_TT_MUL;
    if ( strcmp("/", str) == 0 )
        return NST_TT_DIV;
    if ( strcmp("^", str) == 0 )
        return NST_TT_POW;
    if ( strcmp("%", str) == 0 )
        return NST_TT_MOD;
    if ( strcmp("&&", str) == 0 )
        return NST_TT_L_AND;
    if ( strcmp("||", str) == 0 )
        return NST_TT_L_OR;
    if ( strcmp("&|", str) == 0 )
        return NST_TT_L_XOR;
    if ( strcmp("&", str) == 0 )
        return NST_TT_B_AND;
    if ( strcmp("|", str) == 0 )
        return NST_TT_B_OR;
    if ( strcmp("^^", str) == 0 )
        return NST_TT_B_XOR;
    if ( strcmp("<<", str) == 0 )
        return NST_TT_LSHIFT;
    if ( strcmp(">>", str) == 0 )
        return NST_TT_RSHIFT;
    if ( strcmp(">", str) == 0 )
        return NST_TT_GT;
    if ( strcmp("<", str) == 0 )
        return NST_TT_LT;
    if ( strcmp("==", str) == 0 )
        return NST_TT_EQ;
    if ( strcmp("!=", str) == 0 )
        return NST_TT_NEQ;
    if ( strcmp(">=", str) == 0 )
        return NST_TT_GTE;
    if ( strcmp("<=", str) == 0 )
        return NST_TT_LTE;
    if ( strcmp("><", str) == 0 )
        return NST_TT_CONCAT;
    if ( strcmp(">>>", str) == 0 )
        return NST_TT_STDOUT;
    if ( strcmp("<<<", str) == 0 )
        return NST_TT_STDIN;
    if ( strcmp("->", str) == 0 )
        return NST_TT_RANGE;
    if ( strcmp("$", str) == 0 )
        return NST_TT_LEN;
    if ( strcmp("::", str) == 0 )
        return NST_TT_CAST;
    if ( strcmp("?::", str) == 0 )
        return NST_TT_TYPEOF;
    if ( strcmp("=", str) == 0 )
        return NST_TT_ASSIGN;
    if ( strcmp("+=", str) == 0 )
        return NST_TT_ADD_A;
    if ( strcmp("-=", str) == 0 )
        return NST_TT_SUB_A;
    if ( strcmp("*=", str) == 0 )
        return NST_TT_MUL_A;
    if ( strcmp("/=", str) == 0 )
        return NST_TT_DIV_A;
    if ( strcmp("^=", str) == 0 )
        return NST_TT_POW_A;
    if ( strcmp("%=", str) == 0 )
        return NST_TT_MOD_A;
    if ( strcmp("|=", str) == 0 )
        return NST_TT_B_OR_A;
    if ( strcmp("&=", str) == 0 )
        return NST_TT_B_AND_A;
    if ( strcmp("^^=", str) == 0 )
        return NST_TT_B_XOR_A;
    if ( strcmp("<<=", str) == 0 )
        return NST_TT_LSHIFT_A;
    if ( strcmp(">>=", str) == 0 )
        return NST_TT_RSHIFT_A;
    if ( strcmp("><=", str) == 0 )
        return NST_TT_CONCAT_A;
    if ( strcmp("@", str) == 0 )
        return NST_TT_CALL;
    if ( strcmp("@@", str) == 0 )
        return NST_TT_LOC_CALL;
    if ( strcmp("!", str) == 0 )
        return NST_TT_L_NOT;
    if ( strcmp("~", str) == 0 )
        return NST_TT_B_NOT;
    if ( strcmp(".", str) == 0 )
        return NST_TT_EXTRACT;
    if ( strcmp("-:", str) == 0 )
        return NST_TT_NEG;
    if ( strcmp("?", str) == 0 )
        return NST_TT_IF;
    if ( strcmp("?..", str) == 0 )
        return NST_TT_WHILE;
    if ( strcmp("..?", str) == 0 )
        return NST_TT_DOWHILE;
    if ( strcmp("...", str) == 0 )
        return NST_TT_FOR;
    if ( strcmp("~=", str) == 0 )
        return NST_TT_AS;
    if ( strcmp("#", str) == 0 )
        return NST_TT_FUNC;
    if ( strcmp("=>", str) == 0 )
        return NST_TT_RETURN;
    if ( strcmp("|>", str) == 0 )
        return NST_TT_SWITCH;
    if ( strcmp("..", str) == 0 )
        return NST_TT_CONTINUE;
    if ( strcmp(";", str) == 0 )
        return NST_TT_BREAK;
    if ( strcmp(":", str) == 0 )
        return NST_TT_COLON;
    if ( strcmp("|#|", str) == 0 )
        return NST_TT_IMPORT;
    if ( strcmp("$", str) == 0 )
        return NST_TT_LEN;
    if ( strcmp(",", str) == 0 )
        return NST_TT_COMMA;
    if ( strcmp("(", str) == 0 )
        return NST_TT_L_PAREN;
    if ( strcmp(")", str) == 0 )
        return NST_TT_R_PAREN;
    if ( strcmp("[", str) == 0 )
        return NST_TT_L_BRACKET;
    if ( strcmp("]", str) == 0 )
        return NST_TT_R_BRACKET;
    if ( strcmp("{", str) == 0 )
        return NST_TT_L_BRACE;
    if ( strcmp("}", str) == 0 )
        return NST_TT_R_BRACE;
    if ( strcmp("<{", str) == 0 )
        return NST_TT_L_VBRACE;
    if ( strcmp("}>", str) == 0 )
        return NST_TT_R_VBRACE;
    return -1;
}

void nst_print_token(Nst_LexerToken *token)
{
    printf("(");

    if      ( token->type == NST_TT_ADD       ) printf("ADD");
    else if ( token->type == NST_TT_SUB       ) printf("SUB");
    else if ( token->type == NST_TT_MUL       ) printf("MUL");
    else if ( token->type == NST_TT_DIV       ) printf("DIV");
    else if ( token->type == NST_TT_POW       ) printf("POW");
    else if ( token->type == NST_TT_MOD       ) printf("MOD");
    else if ( token->type == NST_TT_B_AND     ) printf("B_AND");
    else if ( token->type == NST_TT_B_OR      ) printf("B_OR");
    else if ( token->type == NST_TT_B_XOR     ) printf("B_XOR");
    else if ( token->type == NST_TT_LSHIFT    ) printf("LSHIFT");
    else if ( token->type == NST_TT_RSHIFT    ) printf("RSHIFT");
    else if ( token->type == NST_TT_CONCAT    ) printf("CONCAT");
    else if ( token->type == NST_TT_L_AND     ) printf("L_AND");
    else if ( token->type == NST_TT_L_OR      ) printf("L_OR");
    else if ( token->type == NST_TT_L_XOR     ) printf("L_XOR");
    else if ( token->type == NST_TT_GT        ) printf("GT");
    else if ( token->type == NST_TT_LT        ) printf("LT");
    else if ( token->type == NST_TT_EQ        ) printf("EQ");
    else if ( token->type == NST_TT_NEQ       ) printf("NEQ");
    else if ( token->type == NST_TT_GTE       ) printf("GTE");
    else if ( token->type == NST_TT_LTE       ) printf("LTE");
    else if ( token->type == NST_TT_CAST      ) printf("CAST");
    else if ( token->type == NST_TT_CALL      ) printf("CALL");
    else if ( token->type == NST_TT_EXTRACT   ) printf("EXTRACT");
    else if ( token->type == NST_TT_RANGE     ) printf("RANGE");
    else if ( token->type == NST_TT_ASSIGN    ) printf("ASSIGN");
    else if ( token->type == NST_TT_ADD_A     ) printf("ADD_A");
    else if ( token->type == NST_TT_SUB_A     ) printf("SUB_A");
    else if ( token->type == NST_TT_MUL_A     ) printf("MUL_A");
    else if ( token->type == NST_TT_DIV_A     ) printf("DIV_A");
    else if ( token->type == NST_TT_POW_A     ) printf("POW_A");
    else if ( token->type == NST_TT_MOD_A     ) printf("MOD_A");
    else if ( token->type == NST_TT_B_AND_A   ) printf("B_AND_A");
    else if ( token->type == NST_TT_B_OR_A    ) printf("B_OR_A");
    else if ( token->type == NST_TT_B_XOR_A   ) printf("B_XOR_A");
    else if ( token->type == NST_TT_LSHIFT_A  ) printf("LSHIFT_A");
    else if ( token->type == NST_TT_RSHIFT_A  ) printf("RSHIFT_A");
    else if ( token->type == NST_TT_CONCAT_A  ) printf("CONCAT_A");
    else if ( token->type == NST_TT_LEN       ) printf("LEN");
    else if ( token->type == NST_TT_L_NOT     ) printf("L_NOT");
    else if ( token->type == NST_TT_B_NOT     ) printf("B_NOT");
    else if ( token->type == NST_TT_STDOUT    ) printf("STDOUT");
    else if ( token->type == NST_TT_STDIN     ) printf("STDIN");
    else if ( token->type == NST_TT_TYPEOF    ) printf("TYPEOF");
    else if ( token->type == NST_TT_IDENT     ) printf("IDENT");
    else if ( token->type == NST_TT_INT     ) printf("N_INT");
    else if ( token->type == NST_TT_REAL    ) printf("N_REAL");
    else if ( token->type == NST_TT_STRING    ) printf("STRING");
    else if ( token->type == NST_TT_L_PAREN   ) printf("L_PAREN");
    else if ( token->type == NST_TT_L_BRACE   ) printf("L_BRACE");
    else if ( token->type == NST_TT_L_VBRACE  ) printf("L_VBRACE");
    else if ( token->type == NST_TT_L_BRACKET ) printf("L_BRACKET");
    else if ( token->type == NST_TT_R_PAREN   ) printf("R_PAREN");
    else if ( token->type == NST_TT_R_BRACE   ) printf("R_BRACE");
    else if ( token->type == NST_TT_R_VBRACE  ) printf("R_VBRACE");
    else if ( token->type == NST_TT_R_BRACKET ) printf("R_BRACKET");
    else if ( token->type == NST_TT_IF        ) printf("IF");
    else if ( token->type == NST_TT_AS        ) printf("AS");
    else if ( token->type == NST_TT_ENDL      ) printf("ENDL");
    else if ( token->type == NST_TT_COMMA     ) printf("COMMA");
    else if ( token->type == NST_TT_COLON     ) printf("COLON");
    else if ( token->type == NST_TT_EOFILE    ) printf("EOFILE");
    else if ( token->type == NST_TT_WHILE     ) printf("WHILE");
    else if ( token->type == NST_TT_DOWHILE   ) printf("DOWHILE");
    else if ( token->type == NST_TT_FOR       ) printf("FOR");
    else if ( token->type == NST_TT_FUNC      ) printf("FUNC");
    else if ( token->type == NST_TT_RETURN    ) printf("RETURN");
    else if ( token->type == NST_TT_CONTINUE  ) printf("CONTINUE");
    else if ( token->type == NST_TT_BREAK     ) printf("BREAK");
    else if ( token->type == NST_TT_IMPORT    ) printf("IMPORT");
    else printf("__UNKNOWN__");

    printf(" - ");

    if ( token->type == NST_TT_INT )
        printf("%lli, ", AS_INT(token->value));
    else if ( token->type == NST_TT_REAL )
        printf("%g, ", AS_REAL(token->value));
    else if ( token->type == NST_TT_STRING || token->type == NST_TT_IDENT )
        printf("%s, ", AS_STR(nst_repr_string(AS_STR(token->value)))->value);

    printf("%zi:%zi, %zi:%zi)",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col
    );

    fflush(stdout);
}
