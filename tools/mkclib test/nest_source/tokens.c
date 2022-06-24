#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include "tokens.h"
#include "nst_types.h"

Token *new_token_value(Pos start, Pos end, int type, void *value)
{
    Token *token = malloc(sizeof(Token));
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

Token *new_token_noval(Pos start, Pos end, int type)
{
    Token *token = malloc(sizeof(Token));
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

Token *new_token_noend(Pos start, int type)
{
    Token *token = malloc(sizeof(Token));
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

void destroy_token(Token *token)
{
    if ( token == NULL )
        return;

    if ( token->value != NULL )
    {
        if ( token->type == STRING || token->type == IDENT )
            destroy_string(token->value);
        else
            free(token->value);
    }
    free(token);
}

int str_to_tok(char *str)
{
    if ( strcmp("+", str) == 0 )
        return ADD;
    if ( strcmp("-", str) == 0 )
        return SUB;
    if ( strcmp("*", str) == 0 )
        return MUL;
    if ( strcmp("/", str) == 0 )
        return DIV;
    if ( strcmp("^", str) == 0 )
        return POW;
    if ( strcmp("%", str) == 0 )
        return MOD;
    if ( strcmp("&&", str) == 0 )
        return L_AND;
    if ( strcmp("||", str) == 0 )
        return L_OR;
    if ( strcmp("&|", str) == 0 )
        return L_XOR;
    if ( strcmp("&", str) == 0 )
        return B_AND;
    if ( strcmp("|", str) == 0 )
        return B_OR;
    if ( strcmp("^^", str) == 0 )
        return B_XOR;
    if ( strcmp("<<", str) == 0 )
        return LSHIFT;
    if ( strcmp(">>", str) == 0 )
        return RSHIFT;
    if ( strcmp(">", str) == 0 )
        return GT;
    if ( strcmp("<", str) == 0 )
        return LT;
    if ( strcmp("==", str) == 0 )
        return EQ;
    if ( strcmp("!=", str) == 0 )
        return NEQ;
    if ( strcmp(">=", str) == 0 )
        return GTE;
    if ( strcmp("<=", str) == 0 )
        return LTE;
    if ( strcmp("><", str) == 0 )
        return CONCAT;
    if ( strcmp(">>>", str) == 0 )
        return STDOUT;
    if ( strcmp("<<<", str) == 0 )
        return STDIN;
    if ( strcmp("->", str) == 0 )
        return RANGE;
    if ( strcmp("$", str) == 0 )
        return LEN;
    if ( strcmp("::", str) == 0 )
        return CAST;
    if ( strcmp("?::", str) == 0 )
        return TYPEOF;
    if ( strcmp("=", str) == 0 )
        return ASSIGN;
    if ( strcmp("+=", str) == 0 )
        return ADD_A;
    if ( strcmp("-=", str) == 0 )
        return SUB_A;
    if ( strcmp("*=", str) == 0 )
        return MUL_A;
    if ( strcmp("/=", str) == 0 )
        return DIV_A;
    if ( strcmp("^=", str) == 0 )
        return POW_A;
    if ( strcmp("%=", str) == 0 )
        return MOD_A;
    if ( strcmp("|=", str) == 0 )
        return B_OR_A;
    if ( strcmp("&=", str) == 0 )
        return B_AND_A;
    if ( strcmp("^^=", str) == 0 )
        return B_XOR_A;
    if ( strcmp("<<=", str) == 0 )
        return LSHIFT_A;
    if ( strcmp(">>=", str) == 0 )
        return RSHIFT_A;
    if ( strcmp("><=", str) == 0 )
        return CONCAT_A;
    if ( strcmp("@", str) == 0 )
        return CALL;
    if ( strcmp("!", str) == 0 )
        return L_NOT;
    if ( strcmp("~", str) == 0 )
        return B_NOT;
    if ( strcmp(".", str) == 0 )
        return EXTRACT;
    if ( strcmp("?", str) == 0 )
        return IF;
    if ( strcmp("?..", str) == 0 )
        return WHILE;
    if ( strcmp("..?", str) == 0 )
        return DOWHILE;
    if ( strcmp("...", str) == 0 )
        return FOR;
    if ( strcmp("~=", str) == 0 )
        return AS;
    if ( strcmp("#", str) == 0 )
        return FUNC;
    if ( strcmp("=>", str) == 0 )
        return RETURN;
    if ( strcmp("..", str) == 0 )
        return CONTINUE;
    if ( strcmp(";", str) == 0 )
        return BREAK;
    if ( strcmp(":", str) == 0 )
        return COLON;
    if ( strcmp("|#|", str) == 0 )
        return IMPORT;
    if ( strcmp("$", str) == 0 )
        return LEN;
    if ( strcmp(",", str) == 0 )
        return COMMA;
    if ( strcmp("(", str) == 0 )
        return L_PAREN;
    if ( strcmp(")", str) == 0 )
        return R_PAREN;
    if ( strcmp("[", str) == 0 )
        return L_BRACKET;
    if ( strcmp("]", str) == 0 )
        return R_BRACKET;
    if ( strcmp("{", str) == 0 )
        return L_BRACE;
    if ( strcmp("}", str) == 0 )
        return R_BRACE;
    if ( strcmp("<{", str) == 0 )
        return L_VBRACE;
    if ( strcmp("}>", str) == 0 )
        return R_VBRACE;
    return -1;
}

void print_token(Token *token)
{
    printf("(");

    if      ( token->type == ADD       ) printf("ADD");
    else if ( token->type == SUB       ) printf("SUB");
    else if ( token->type == MUL       ) printf("MUL");
    else if ( token->type == DIV       ) printf("DIV");
    else if ( token->type == POW       ) printf("POW");
    else if ( token->type == MOD       ) printf("MOD");
    else if ( token->type == B_AND     ) printf("B_AND");
    else if ( token->type == B_OR      ) printf("B_OR");
    else if ( token->type == B_XOR     ) printf("B_XOR");
    else if ( token->type == LSHIFT    ) printf("LSHIFT");
    else if ( token->type == RSHIFT    ) printf("RSHIFT");
    else if ( token->type == CONCAT    ) printf("CONCAT");
    else if ( token->type == L_AND     ) printf("L_AND");
    else if ( token->type == L_OR      ) printf("L_OR");
    else if ( token->type == L_XOR     ) printf("L_XOR");
    else if ( token->type == GT        ) printf("GT");
    else if ( token->type == LT        ) printf("LT");
    else if ( token->type == EQ        ) printf("EQ");
    else if ( token->type == NEQ       ) printf("NEQ");
    else if ( token->type == GTE       ) printf("GTE");
    else if ( token->type == LTE       ) printf("LTE");
    else if ( token->type == CAST      ) printf("CAST");
    else if ( token->type == CALL      ) printf("CALL");
    else if ( token->type == EXTRACT   ) printf("EXTRACT");
    else if ( token->type == RANGE     ) printf("RANGE");
    else if ( token->type == ASSIGN    ) printf("ASSIGN");
    else if ( token->type == ADD_A     ) printf("ADD_A");
    else if ( token->type == SUB_A     ) printf("SUB_A");
    else if ( token->type == MUL_A     ) printf("MUL_A");
    else if ( token->type == DIV_A     ) printf("DIV_A");
    else if ( token->type == POW_A     ) printf("POW_A");
    else if ( token->type == MOD_A     ) printf("MOD_A");
    else if ( token->type == B_AND_A   ) printf("B_AND_A");
    else if ( token->type == B_OR_A    ) printf("B_OR_A");
    else if ( token->type == B_XOR_A   ) printf("B_XOR_A");
    else if ( token->type == LSHIFT_A  ) printf("LSHIFT_A");
    else if ( token->type == RSHIFT_A  ) printf("RSHIFT_A");
    else if ( token->type == CONCAT_A  ) printf("CONCAT_A");
    else if ( token->type == LEN       ) printf("LEN");
    else if ( token->type == L_NOT     ) printf("L_NOT");
    else if ( token->type == B_NOT     ) printf("B_NOT");
    else if ( token->type == STDOUT    ) printf("STDOUT");
    else if ( token->type == STDIN     ) printf("STDIN");
    else if ( token->type == TYPEOF    ) printf("TYPEOF");
    else if ( token->type == IDENT     ) printf("IDENT");
    else if ( token->type == N_INT     ) printf("N_INT");
    else if ( token->type == N_REAL    ) printf("N_REAL");
    else if ( token->type == STRING    ) printf("STRING");
    else if ( token->type == L_PAREN   ) printf("L_PAREN");
    else if ( token->type == L_BRACE   ) printf("L_BRACE");
    else if ( token->type == L_VBRACE  ) printf("L_VBRACE");
    else if ( token->type == L_BRACKET ) printf("L_BRACKET");
    else if ( token->type == R_PAREN   ) printf("R_PAREN");
    else if ( token->type == R_BRACE   ) printf("R_BRACE");
    else if ( token->type == R_VBRACE  ) printf("R_VBRACE");
    else if ( token->type == R_BRACKET ) printf("R_BRACKET");
    else if ( token->type == IF        ) printf("IF");
    else if ( token->type == AS        ) printf("AS");
    else if ( token->type == ENDL      ) printf("ENDL");
    else if ( token->type == COMMA     ) printf("COMMA");
    else if ( token->type == COLON     ) printf("COLON");
    else if ( token->type == EOFILE    ) printf("EOFILE");
    else if ( token->type == WHILE     ) printf("WHILE");
    else if ( token->type == DOWHILE   ) printf("DOWHILE");
    else if ( token->type == FOR       ) printf("FOR");
    else if ( token->type == FUNC      ) printf("FUNC");
    else if ( token->type == RETURN    ) printf("RETURN");
    else if ( token->type == CONTINUE  ) printf("CONTINUE");
    else if ( token->type == BREAK     ) printf("BREAK");
    else if ( token->type == IMPORT    ) printf("IMPORT");
    else printf("__UNKNOWN__");

    printf(" - ");

    if ( token->type == N_INT )
        printf("%lli, ", AS_INT(((Nst_Obj *)token->value)));
    else if ( token->type == N_REAL )
        printf("%g, ", AS_REAL(((Nst_Obj *)token->value)));
    else if ( token->type == STRING || token->type == IDENT )
        printf("%s, ", AS_STR(((Nst_Obj *)token->value))->value);

    printf("%zi:%zi, %zi:%zi)",
        token->start.line,
        token->start.col,
        token->end.line,
        token->end.col
    );

    fflush(stdout);
}
